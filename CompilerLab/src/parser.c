/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdlib.h>

#include "debug.h"
#include "error.h"
#include "parser.h"
#include "reader.h"
#include "scanner.h"
#include "semantics.h"
#include "symtab.h"

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
    checkFreshIdent(currentToken->string);
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
    checkFreshIdent(currentToken->string);
    typeObject = createTypeObject(currentToken->string);

    eat(SB_EQ);
    typeObject->typeAttrs->actualType = compileType();

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
    checkFreshIdent(currentToken->string);
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
    checkFreshIdent(currentToken->string);
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
    checkFreshIdent(currentToken->string);
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
    Object* object = NULL;

    switch (lookAhead->tokenType)
    {
    case TK_NUMBER:
        eat(TK_NUMBER);
        constantValue = makeIntConstant(currentToken->value);
        break;
    case TK_IDENT:
        object = checkDeclaredConstant(currentToken->string);
        if (object != NULL)
        {
            constantValue = duplicateConstantValue(object->constAttrs->value);
        }
        break;
    case TK_CHAR:
        eat(TK_CHAR);
        constantValue = makeCharConstant(currentToken->value);
        break;
    default:
        error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
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
    Object* object = NULL;

    switch (lookAhead->tokenType)
    {
    case TK_IDENT:
        eat(TK_IDENT);
        object = checkDeclaredConstant(currentToken->string);
        if (object != NULL && object->constAttrs->value->type == TP_INT)
        {
            constantValue = duplicateConstantValue(object->constAttrs->value);
        }
        else
        {
            error(ERR_UNDECLARED_INT_CONSTANT, currentToken->lineNo, currentToken->colNo);
        }
        break;
    case TK_NUMBER:
        eat(TK_NUMBER);
        constantValue = makeIntConstant(currentToken->value);
        break;

    default:
        error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
        break;
    }

    return constantValue;
}

Type* compileType(void)
{
    Object* object = NULL;

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
        object = checkDeclaredType(currentToken->string);
        if (object != NULL)
        {
            type = duplicateType(object->typeAttrs->actualType);
        }
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
        error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
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
        error(ERR_INVALID_BASIC_TYPE, lookAhead->lineNo, lookAhead->colNo);
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
    enum ParamKind paramKind = PARAM_VALUE;

    if (lookAhead->tokenType == KW_VAR)
    {
        eat(KW_VAR);
        paramKind = PARAM_REFERENCE;
    }

    if (lookAhead->tokenType == TK_IDENT)
    {
        eat(TK_IDENT);
        checkFreshIdent(currentToken->string);
        parameterObject = createParameterObject(currentToken->string, paramKind, symtab->currentScope->owner);
        eat(SB_COLON);
        parameterObject->paramAttrs->type = compileBasicType();
    }
    else
    {
        error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
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
        error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
        break;
    }
}

Type* compileLValue(void)
{
    Object* lValueObject = NULL;
    Type* lValueType = NULL;

    eat(TK_IDENT);
    lValueObject = checkDeclaredLValueIdent(currentToken->string);

    if (lValueObject != NULL)
    {
        switch (lValueObject->kind)
        {
        case OBJ_VARIABLE:
            lValueType = compileIndexes(lValueObject->varAttrs->type);
            break;
        case OBJ_FUNCTION:
            lValueType = lValueObject->funcAttrs->returnType;
            break;
        case OBJ_PARAMETER:
            lValueType = lValueObject->paramAttrs->type;
            break;
        }
    }

    return lValueType;
}

void compileAssignSt(void)
{
    Type* lValueType = NULL;
    Type* expressionType = NULL;
    lValueType = compileLValue();

    eat(SB_ASSIGN);
    expressionType = compileExpression();
    checkTypeEquality(lValueType, expressionType);
}

void compileCallSt(void)
{
    Object* procedureObject = NULL;
    eat(KW_CALL);

    eat(TK_IDENT);
    procedureObject = checkDeclaredProcedure(currentToken->string);

    compileArguments(procedureObject->procAttrs->paramList);
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
    Object* variableObject = NULL;
    Type* variableType = NULL;
    Type* fromType = NULL;
    Type* toType = NULL;

    eat(KW_FOR);

    eat(TK_IDENT);
    variableObject = checkDeclaredVariable(currentToken->string);
    if (variableObject != NULL)
    {
        variableType = variableObject->varAttrs->type;
    }

    eat(SB_ASSIGN);
    fromType = compileExpression();
    checkTypeEquality(variableType, fromType);

    eat(KW_TO);
    toType = compileExpression();
    checkTypeEquality(variableType, toType);

    eat(KW_DO);
    compileStatement();
}

void compileArgument(Object* param)
{
    Type* expressionType = NULL;

    if (param->paramAttrs->kind == PARAM_VALUE)
    {
        expressionType = compileExpression();
    }
    else
    {
        expressionType = compileLValue();
    }

    checkTypeEquality(param->paramAttrs->type, expressionType);
}

void compileArguments(ObjectNode* paramList)
{
    ObjectNode* paramNode = paramList;

    if (paramNode == NULL)
    {
        return;
    }

    eat(SB_LPAR);

    while (1)
    {
        compileArgument(paramNode->object);

        paramNode = paramNode->next;
        if (paramNode == NULL)
        {
            // End of parameter list
            break;
        }

        // End of argument list
        if (lookAhead->tokenType == SB_RPAR)
        {
            error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
        }
        else
        {
            eat(SB_COMMA);
        }
    }

    if (lookAhead->tokenType == SB_COMMA)
    {
        error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    }

    eat(SB_RPAR);

#ifdef OLD
    switch (lookAhead->tokenType)
    {
    case SB_LPAR:
        do
        {
            if (paramNode != NULL)
            {
                compileArgument(paramNode->object);
                paramNode = paramNode->next;
            }
            else
            {
                error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
            }

        } while (lookAhead->tokenType == SB_COMMA);

        if (paramNode != NULL)
        {
            error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
        }
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
        error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
    }
#endif // OLD

}

void compileCondition(void)
{
    Type* leftType = NULL;
    Type* rightType = NULL;

    leftType = compileExpression();
    checkBasicType(leftType);

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
        error(ERR_INVALID_COMPARATOR, lookAhead->lineNo, lookAhead->colNo);
        break;
    }

    rightType = compileExpression();
    checkBasicType(rightType);

    checkTypeEquality(leftType, rightType);
}

Type* compileExpression(void)
{
    Type* expressionType = NULL;

    switch (lookAhead->tokenType)
    {
    case SB_PLUS:
        eat(SB_PLUS);
        expressionType = compileExpression2();
        checkIntType(expressionType);
        break;
    case SB_MINUS:
        eat(SB_MINUS);
        expressionType = compileExpression2();
        checkIntType(expressionType);
        break;
    default:
        expressionType = compileExpression2();
    }

    return expressionType;
}

Type* compileExpression2(void)
{
    Type* termType = NULL;

    termType = compileTerm();
    compileExpression3();

    return termType;
}

void compileExpression3(void)
{
    Type* expressionType = NULL;

    switch (lookAhead->tokenType)
    {
    case SB_PLUS:
        eat(SB_PLUS);
        expressionType = compileTerm();
        checkIntType(expressionType);
        compileExpression3();
        break;
    case SB_MINUS:
        eat(SB_MINUS);
        expressionType = compileTerm();
        checkIntType(expressionType);
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
        error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
    }
}

Type* compileTerm(void)
{
    Type* termType = NULL;
    
    termType = compileFactor();
    compileTerm2();

    return termType;
}

void compileTerm2(void)
{
    Type* expressionType = NULL;

    switch (lookAhead->tokenType)
    {
    case SB_TIMES:
        eat(SB_TIMES);
        expressionType = compileFactor();
        checkIntType(expressionType);
        compileTerm2();
        break;
    case SB_SLASH:
        eat(SB_SLASH);
        expressionType = compileFactor();
        checkIntType(expressionType);
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
        error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
    }
}

Type* compileFactor(void)
{
    Object* object = NULL;
    Type* type = NULL;

    switch (lookAhead->tokenType)
    {
    case TK_NUMBER:
        eat(TK_NUMBER);
        type = intType;
        break;
    case TK_CHAR:
        eat(TK_CHAR);
        type = charType;
        break;
    case TK_IDENT:
        eat(TK_IDENT);
        object = checkDeclaredIdent(currentToken->string);

        switch (object->kind)
        {
        case OBJ_CONSTANT:
            type = getConstantType(object->constAttrs->value);
            break;
        case OBJ_PARAMETER:
            type = object->paramAttrs->type;
            break;
        case OBJ_VARIABLE:
            type = compileIndexes(object->varAttrs->type);
            break;
        case OBJ_FUNCTION:
            compileArguments(object->funcAttrs->paramList);
            type = object->funcAttrs->returnType;
            break;
        default:
            error(ERR_INVALID_FACTOR, currentToken->lineNo, currentToken->colNo);
            break;
        }

        break;
    default:
        error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
        break;
    }

    return type;
}

Type* compileIndexes(Type* arrayType)
{
    Type* indexerType = NULL;
    // TODO: ?

    while (arrayType->typeClass == TP_ARRAY)
    {
        eat(SB_LSEL);
        indexerType = compileExpression();
        // TODO: ?
        checkIntType(indexerType);
        eat(SB_RSEL);

        arrayType = arrayType->elementType;
    }

    return arrayType;
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
