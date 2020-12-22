/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include "semantics.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

extern SymTab* symtab;
extern Token* currentToken;

Object* lookupObject(char* name)
{
    Scope* currentScope = symtab->currentScope;
    Object* object;

    while (currentScope != NULL)
    {
        object = findObject(currentScope->objList, name);

        if (object != NULL)
        {
            return object;
        }

        currentScope = currentScope->outer;
    }

    // Lookup globalObjectList
    return findObject(symtab->globalObjectList, name);
}

void checkFreshIdent(char* name)
{
    if (findObject(symtab->currentScope->objList, name) != NULL)
    {
        error(ERR_DUPLICATE_IDENT, currentToken->lineNo, currentToken->colNo);
    }
}

Object* checkDeclaredIdent(char* name)
{
    Object* object = lookupObject(name);

    if (object == NULL)
    {
        error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
    }

    return object;
}

Object* checkDeclaredConstant(char* name)
{
    Object* object = lookupObject(name);

    if (object == NULL)
    {
        error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo);
    }
    else if (object->kind != OBJ_CONSTANT)
    {
        error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
    }

    return object;
}

Object* checkDeclaredType(char* name)
{
    Object* object = lookupObject(name);

    if (object == NULL)
    {
        error(ERR_UNDECLARED_TYPE, currentToken->lineNo, currentToken->colNo);
    }
    else if (object->kind != OBJ_TYPE)
    {
        error(ERR_INVALID_TYPE, currentToken->lineNo, currentToken->colNo);
    }

    return object;
}

Object* checkDeclaredVariable(char* name)
{
    Object* object = lookupObject(name);

    if (object == NULL)
    {
        error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
    }
    else if (object->kind != OBJ_VARIABLE)
    {
        error(ERR_INVALID_VARIABLE, currentToken->lineNo, currentToken->colNo);
    }

    return object;
}

Object* checkDeclaredVariable1(char* name)
{
    Object* object = lookupObject(name);

    if (object == NULL)
    {
        error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
    }
    else
    {
        switch (object->kind)
        {
        case OBJ_VARIABLE:
        case OBJ_CONSTANT:
        case OBJ_FUNCTION:
            break;
        case OBJ_PARAMETER:
            if (object->paramAttrs->function != symtab->currentScope->owner)
            {
                error(ERR_INVALID_VARIABLE, currentToken->lineNo, currentToken->colNo);
            }
            break;
        default:
            error(ERR_INVALID_VARIABLE, currentToken->lineNo, currentToken->colNo);
            break;
        }
    }

    return object;
}

Object* checkDeclaredFunction(char* name)
{
    Object* object = lookupObject(name);

    if (object == NULL)
    {
        error(ERR_UNDECLARED_FUNCTION, currentToken->lineNo, currentToken->colNo);
    }
    else if (object->kind != OBJ_FUNCTION)
    {
        error(ERR_INVALID_FUNCTION, currentToken->lineNo, currentToken->colNo);
    }

    return object;
}

Object* checkDeclaredProcedure(char* name)
{
    Object* object = lookupObject(name);

    if (object == NULL)
    {
        error(ERR_UNDECLARED_PROCEDURE, currentToken->lineNo, currentToken->colNo);
    }
    else if (object->kind != OBJ_PROCEDURE)
    {
        error(ERR_INVALID_PROCEDURE, currentToken->lineNo, currentToken->colNo);
    }

    return object;
}

Object* checkDeclaredLValueIdent(char* name)
{
    Object* object = lookupObject(name);

    if (object == NULL)
    {
        error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
    }
    else
    {
        switch (object->kind)
        {
        case OBJ_VARIABLE:
            break;
        case OBJ_FUNCTION:
            if (object != symtab->currentScope->owner)
            {
                error(ERR_INVALID_LVALUE, currentToken->lineNo, currentToken->colNo);
            }
            break;
        case OBJ_PARAMETER:
            break;
        default:
            error(ERR_INVALID_LVALUE, currentToken->lineNo, currentToken->colNo);
            break;
        }
    }

    return object;
}

void checkIntType(Type* type)
{
    if (type->typeClass != TP_INT)
    {
        error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    }
    
}
void checkCharType(Type* type)
{
    if (type->typeClass != TP_CHAR)
    {
        error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    }
}
void checkArrayType(Type* type)
{
    if (type->typeClass != TP_ARRAY)
    {
        error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    }
}
void checkBasicType(Type* type)
{
    if (type->typeClass != TP_CHAR && type->typeClass != TP_INT)
    {
        error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    }
}
void checkTypeEquality(Type* type1, Type* type2)
{
    if (type1->typeClass != type2->typeClass)
    {
        error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
    }
    else if (type1->typeClass == TP_ARRAY)
    {
        if (type1->arraySize != type2->arraySize)
        {
            error(ERR_TYPE_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
        }
        else
        {
            checkTypeEquality(type1->elementType, type2->elementType);
        }
    }
}
