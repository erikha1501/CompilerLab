/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdlib.h>

#include "error.h"
#include "parser.h"
#include "reader.h"
#include "scanner.h"
#include "symtab.h"
#include "debug.h"

Token* currentToken;
Token* lookAhead;

void scan(void)
{
    Token* tmp = currentToken;
    currentToken = lookAhead;
    lookAhead = getValidToken();
    free(tmp);
}

void eat(TokenType tokenType)
{
    if (lookAhead->tokenType == tokenType)
    {
        scan();
    }
    else
    {
        missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
    }
}

void compileProgram(void)
{
    Object* programObject = NULL;
    eat(KW_PROGRAM);

    eat(TK_IDENT);
    programObject = createProgramObject(currentToken->string);

    eat(SB_SEMICOLON);
    enterBlock(programObject->progAttrs->scope);

    compileBlock();

    eat(SB_PERIOD);
    exitBlock();
}

void compileBlock(void)
{
    if (lookAhead->tokenType == KW_CONST)
    {
        eat(KW_CONST);
        compileConstDecl();
        compileConstDecls();
        compileBlock2();
    }
    else
    {
        compileBlock2();
    }
}

void compileBlock2(void)
{
    if (lookAhead->tokenType == KW_TYPE)
    {
        eat(KW_TYPE);
        compileTypeDecl();
        compileTypeDecls();
        compileBlock3();
    }
    else
        compileBlock3();
}

void compileBlock3(void)
{
    if (lookAhead->tokenType == KW_VAR)
    {
        eat(KW_VAR);
        compileVarDecl();
        compileVarDecls();
        compileBlock4();
    }
    else
        compileBlock4();
}

void compileBlock4(void)
{
    compileSubDecls();
    compileBlock5();
}

void compileBlock5(void)
{
    eat(KW_BEGIN);
    compileStatements();
    eat(KW_END);
}

void compileConstDecls(void)
{
    while (lookAhead->tokenType == TK_IDENT)
        compileConstDecl();
}

void compileConstDecl(void)
{
    Object* constantObject = NULL;

    eat(TK_IDENT);
    constantObject = createConstantObject(currentToken->string);

    eat(SB_EQ);
    constantObject->constAttrs->value = compileConstant();

    eat(SB_SEMICOLON);
    declareObject(constantObject);
}

void compileTypeDecls(void)
{
    while (lookAhead->tokenType == TK_IDENT)
        compileTypeDecl();
}

void compileTypeDecl(void)
{
    Object* typeObject = NULL;

    eat(TK_IDENT);
    typeObject = createTypeObject(currentToken->string);

    eat(SB_EQ);
    // TODO: Get actual type
    compileType();

    eat(SB_SEMICOLON);
    declareObject(typeObject);
}

void compileVarDecls(void)
{
    while (lookAhead->tokenType == TK_IDENT)
        compileVarDecl();
}

void compileVarDecl(void)
{
    Object* variableObject = NULL;

    eat(TK_IDENT);
    variableObject = createVariableObject(currentToken->string);

    eat(SB_COLON);
    variableObject->varAttrs->type = compileType();

    eat(SB_SEMICOLON);
    declareObject(variableObject);
}

void compileSubDecls(void)
{
    while (1)
    {
        if (lookAhead->tokenType == KW_FUNCTION)
        {
            compileFuncDecl();
        }
        else if (lookAhead->tokenType == KW_PROCEDURE)
        {
            compileProcDecl();
        }
        else
        {
            break;
        }
    }
}

void compileFuncDecl(void)
{
    Object* functionObject = NULL;

    eat(KW_FUNCTION);

    eat(TK_IDENT);
    functionObject = createFunctionObject(currentToken->string);
    declareObject(functionObject);
    enterBlock(functionObject->funcAttrs->scope);

    compileParams();

    eat(SB_COLON);
    functionObject->funcAttrs->returnType = compileBasicType();

    eat(SB_SEMICOLON);

    compileBlock();

    eat(SB_SEMICOLON);
    exitBlock();
}

void compileProcDecl(void)
{
    Object* procedureObject = NULL;

    eat(KW_PROCEDURE);

    eat(TK_IDENT);
    procedureObject = createProcedureObject(currentToken->string);
    declareObject(procedureObject);
    enterBlock(procedureObject->procAttrs->scope);

    compileParams();

    eat(SB_SEMICOLON);

    compileBlock();

    eat(SB_SEMICOLON);
    exitBlock();
}

ConstantValue* compileUnsignedConstant(void)
{
    ConstantValue* constantValue = NULL;

    switch (lookAhead->tokenType)
    {
    case TK_NUMBER:
        eat(TK_NUMBER);
        constantValue = makeIntConstant(currentToken->value);
        break;
    case TK_IDENT:
        // TODO lookup
        break;
    case TK_CHAR:
        eat(TK_CHAR);
        constantValue = makeCharConstant(currentToken->value);
        break;
    default:
        error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
        break;
    }

    return constantValue;
}

ConstantValue* compileConstant(void)
{
    ConstantValue* constantValue = NULL;

    switch (lookAhead->tokenType)
    {
    case TK_CHAR:
        eat(TK_CHAR);
        constantValue = makeCharConstant(currentToken->value);
        break;
    case SB_PLUS:
        eat(SB_PLUS);
        constantValue = compileConstant2();
        break;
    case SB_MINUS:
        eat(SB_MINUS);
        constantValue = compileConstant2();
        constantValue->intValue = -constantValue->intValue;
        break;
    default:
        constantValue = compileConstant2();
        break;
    }

    return constantValue;
}

ConstantValue* compileConstant2(void)
{
    ConstantValue* constantValue = NULL;

    switch (lookAhead->tokenType)
    {
    case TK_IDENT:
        eat(TK_IDENT);
        // TODO lookup;
        break;
    case TK_NUMBER:
        eat(TK_NUMBER);
        constantValue = makeIntConstant(currentToken->value);
        break;

    default:
        error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
        break;
    }

    return constantValue;
}

Type* compileType(void)
{
    Type* type = NULL;
    int arraySize = -1;

    switch (lookAhead->tokenType)
    {
    case KW_INTEGER:
        eat(KW_INTEGER);
        type = makeIntType();
        break;
    case KW_CHAR:
        eat(KW_CHAR);
        type = makeCharType();
        break;
    case TK_IDENT:
        eat(TK_IDENT);
        // TODO lookup
        break;
    case KW_ARRAY:
        eat(KW_ARRAY);
        eat(SB_LSEL);
        eat(TK_NUMBER);
        arraySize = currentToken->value;
        eat(SB_RSEL);
        eat(KW_OF);
        type = makeArrayType(arraySize, compileType());
        break;
    default:
        error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
        break;
    }

    return type;
}

Type* compileBasicType(void)
{
    Type* type = NULL;

    switch (lookAhead->tokenType)
    {
    case KW_INTEGER:
        eat(KW_INTEGER);
        type = makeIntType();
        break;
    case KW_CHAR:
        eat(KW_CHAR);
        type = makeCharType();
        break;

    default:
        error(ERR_INVALIDBASICTYPE, lookAhead->lineNo, lookAhead->colNo);
        break;
    }

    return type;
}

void compileParams(void)
{
    if (lookAhead->tokenType == SB_LPAR)
    {
        eat(SB_LPAR);
        compileParam();
        compileParams2();
        eat(SB_RPAR);
    }
}

void compileParams2(void)
{
    if (lookAhead->tokenType == SB_SEMICOLON)
    {
        eat(SB_SEMICOLON);
        compileParam();
        compileParams2();
    }
}

void compileParam(void)
{
    Object* parameterObject = NULL;
    int paramKind = PARAM_VALUE;

    if (lookAhead->tokenType == KW_VAR)
    {
        eat(KW_VAR);
        paramKind = PARAM_REFERENCE;
    }

    if (lookAhead->tokenType == TK_IDENT)
    {
        eat(TK_IDENT);
        parameterObject = createParameterObject(currentToken->string, paramKind, symtab->currentScope->owner);
        eat(SB_COLON);
        parameterObject->paramAttrs->type = compileBasicType();
    }
    else
    {
        error(ERR_INVALIDPARAM, lookAhead->lineNo, lookAhead->colNo);
    }

    declareObject(parameterObject);
}

void compileStatements(void)
{
    compileStatement();
    compileStatements2();
}

void compileStatements2(void)
{
    if (lookAhead->tokenType == SB_SEMICOLON)
    {
        eat(SB_SEMICOLON);
        compileStatement();
        compileStatements2();
    }
}

void compileStatement(void)
{
    switch (lookAhead->tokenType)
    {
    case TK_IDENT:
        compileAssignSt();
        break;
    case KW_CALL:
        compileCallSt();
        break;
    case KW_BEGIN:
        compileGroupSt();
        break;
    case KW_IF:
        compileIfSt();
        break;
    case KW_WHILE:
        compileWhileSt();
        break;
    case KW_FOR:
        compileForSt();
        break;
        // EmptySt needs to check FOLLOW tokens
    case SB_SEMICOLON:
    case KW_END:
    case KW_ELSE:
        break;
        // Error occurs
    default:
        error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
        break;
    }
}

void compileAssignSt(void)
{
    // TK_IDENT Indexes
    eat(TK_IDENT);
    compileIndexes();
    eat(SB_ASSIGN);
    compileExpression();
}

void compileCallSt(void)
{
    eat(KW_CALL);
    // ProcedureIdent
    eat(TK_IDENT);
    compileArguments();
}

void compileGroupSt(void)
{
    eat(KW_BEGIN);
    compileStatements();
    eat(KW_END);
}

void compileIfSt(void)
{
    eat(KW_IF);
    compileCondition();
    eat(KW_THEN);
    compileStatement();
    if (lookAhead->tokenType == KW_ELSE)
    {
        compileElseSt();
    }
}

void compileElseSt(void)
{
    eat(KW_ELSE);
    compileStatement();
}

void compileWhileSt(void)
{
    eat(KW_WHILE);
    compileCondition();
    eat(KW_DO);
    compileStatement();
}

void compileForSt(void)
{
    eat(KW_FOR);
    eat(TK_IDENT);
    eat(SB_ASSIGN);
    compileExpression();
    eat(KW_TO);
    compileExpression();
    eat(KW_DO);
    compileStatement();
}

void compileArguments(void)
{
    switch (lookAhead->tokenType)
    {
    case SB_LPAR:
        eat(SB_LPAR);
        compileExpression();
        compileArguments2();
        eat(SB_RPAR);
        break;
    // Check FOLLOW set
    case SB_SEMICOLON:
    case KW_END:
    case KW_ELSE:
    case SB_TIMES:
    case SB_SLASH:
    case SB_PLUS:
    case SB_MINUS:
    case KW_TO:
    case KW_DO:
    case SB_COMMA:
    case SB_EQ:
    case SB_NEQ:
    case SB_LE:
    case SB_LT:
    case SB_GE:
    case SB_GT:
    case SB_RPAR:
    case SB_RSEL:
    case KW_THEN:
        break;
    default:
        error(ERR_INVALIDARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
    }
}

void compileArguments2(void)
{
    switch (lookAhead->tokenType)
    {
    case SB_COMMA:
        eat(SB_COMMA);
        compileExpression();
        compileArguments2();
        break;
    case SB_RPAR:
        break;
    default:
        error(ERR_INVALIDARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
        break;
    }
}

void compileCondition(void)
{
    compileExpression();
    compileCondition2();
}

void compileCondition2(void)
{
    switch (lookAhead->tokenType)
    {
    case SB_EQ:
    case SB_NEQ:
    case SB_LE:
    case SB_LT:
    case SB_GE:
    case SB_GT:
        eat(lookAhead->tokenType);
        break;
    default:
        error(ERR_INVALIDCOMPARATOR, lookAhead->lineNo, lookAhead->colNo);
        break;
    }
    compileExpression();
}

void compileExpression(void)
{
    if (lookAhead->tokenType == SB_PLUS)
    {
        eat(SB_PLUS);
    }
    else if (lookAhead->tokenType == SB_MINUS)
    {
        eat(SB_MINUS);
    }

    compileExpression2();
}

void compileExpression2(void)
{
    compileTerm();
    compileExpression3();
}

void compileExpression3(void)
{
    switch (lookAhead->tokenType)
    {
    case SB_PLUS:
        eat(SB_PLUS);
        compileTerm();
        compileExpression3();
        break;
    case SB_MINUS:
        eat(SB_MINUS);
        compileTerm();
        compileExpression3();
        break;
        // Check the FOLLOW set
    case KW_TO:
    case KW_DO:
    case SB_RPAR:
    case SB_COMMA:
    case SB_EQ:
    case SB_NEQ:
    case SB_LE:
    case SB_LT:
    case SB_GE:
    case SB_GT:
    case SB_RSEL:
    case SB_SEMICOLON:
    case KW_END:
    case KW_ELSE:
    case KW_THEN:
        break;
    default:
        error(ERR_INVALIDEXPRESSION, lookAhead->lineNo, lookAhead->colNo);
    }
}

void compileTerm(void)
{
    compileFactor();
    compileTerm2();
}

void compileTerm2(void)
{
    switch (lookAhead->tokenType)
    {
    case SB_TIMES:
        eat(SB_TIMES);
        compileFactor();
        compileTerm2();
        break;
    case SB_SLASH:
        eat(SB_SLASH);
        compileFactor();
        compileTerm2();
        break;
        // Check the FOLLOW set
    case SB_PLUS:
    case SB_MINUS:
    case KW_TO:
    case KW_DO:
    case SB_RPAR:
    case SB_COMMA:
    case SB_EQ:
    case SB_NEQ:
    case SB_LE:
    case SB_LT:
    case SB_GE:
    case SB_GT:
    case SB_RSEL:
    case SB_SEMICOLON:
    case KW_END:
    case KW_ELSE:
    case KW_THEN:
        break;
    default:
        error(ERR_INVALIDTERM, lookAhead->lineNo, lookAhead->colNo);
    }
}

void compileFactor(void)
{
    switch (lookAhead->tokenType)
    {
    case TK_NUMBER:
        eat(TK_NUMBER);
        break;
    case TK_CHAR:
        eat(TK_CHAR);
        break;
    case TK_IDENT:
        eat(TK_IDENT);
        if (lookAhead->tokenType == SB_LSEL)
        {
            compileIndexes();
        }
        else if (lookAhead->tokenType == SB_LPAR)
        {
            compileArguments();
        }
        break;
    case SB_LPAR:
        eat(SB_LPAR);
        compileExpression();
        eat(SB_RPAR);
        break;
    default:
        error(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
        break;
    }
}

void compileIndexes(void)
{
    if (lookAhead->tokenType == SB_LSEL)
    {
        eat(SB_LSEL);
        compileExpression();
        eat(SB_RSEL);
        compileIndexes();
    }
}

int compile(char* fileName)
{
    if (openInputStream(fileName) == IO_ERROR)
        return IO_ERROR;

    initialize();
    initSymTab();

    currentToken = NULL;
    lookAhead = getValidToken();

    compileProgram();
    printObject(symtab->program, 0);

    cleanSymTab();

    free(currentToken);
    free(lookAhead);
    closeInputStream();
    return IO_SUCCESS;
}
