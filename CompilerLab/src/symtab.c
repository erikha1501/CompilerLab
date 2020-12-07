/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#define _CRT_SECURE_NO_WARNINGS

#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void freeObject(Object* obj);
void freeScope(Scope* scope);
void freeObjectList(ObjectNode* objList);
void freeReferenceList(ObjectNode* objList);

SymTab* symtab;
Type* intType;
Type* charType;

/******************* Type utilities ******************************/

Type* makeIntType(void)
{
    Type* type = (Type*)malloc(sizeof(Type));
    type->typeClass = TP_INT;

    // Default initialize unused fields.
    type->arraySize = -1;
    type->elementType = NULL;

    return type;
}

Type* makeCharType(void)
{
    Type* type = (Type*)malloc(sizeof(Type));
    type->typeClass = TP_CHAR;

    // Default initialize unused fields.
    type->arraySize = -1;
    type->elementType = NULL;

    return type;
}

Type* makeArrayType(int arraySize, Type* elementType)
{
    Type* type = (Type*)malloc(sizeof(Type));
    type->typeClass = TP_ARRAY;
    type->arraySize = arraySize;
    type->elementType = elementType;
    return type;
}

Type* duplicateType(Type* type)
{
    Type* newType = (Type*)malloc(sizeof(Type));
    newType->typeClass = type->typeClass;

    if (type->typeClass == TP_ARRAY)
    {
        newType->arraySize = type->arraySize;
        newType->elementType = duplicateType(type->elementType);
    }
    else
    {
        // Default initialize unused fields.
        newType->arraySize = -1;
        newType->elementType = NULL;
    }

    return newType;
}

int compareType(Type* type1, Type* type2)
{
    if (type1->typeClass != type2->typeClass)
    {
        return 0;
    }

    if (type1->typeClass == TP_ARRAY)
    {
        if (type1->arraySize != type2->arraySize)
        {
            return 0;
        }

        return compareType(type1->elementType, type2->elementType);
    }

    return 1;
}

void freeType(Type* type)
{
    if (type->typeClass == TP_ARRAY)
    {
        freeType(type->elementType);
    }

    free(type);
}

/******************* Constant utility ******************************/

ConstantValue* makeIntConstant(int i)
{
    ConstantValue* newConstantValue = (ConstantValue*)malloc(sizeof(ConstantValue));
    newConstantValue->type = TP_INT;
    newConstantValue->intValue = i;

    return newConstantValue;
}

ConstantValue* makeCharConstant(char ch)
{
    ConstantValue* newConstantValue = (ConstantValue*)malloc(sizeof(ConstantValue));
    newConstantValue->type = TP_CHAR;
    newConstantValue->charValue = ch;

    return newConstantValue;
}

ConstantValue* duplicateConstantValue(ConstantValue* v)
{
    ConstantValue* newConstantValue = (ConstantValue*)malloc(sizeof(ConstantValue));
    newConstantValue->type = v->type;

    if (v->type == TP_CHAR)
    {
        newConstantValue->charValue = v->charValue;
    }
    else
    {
        newConstantValue->intValue = v->intValue;
    }

    return newConstantValue;
}

/******************* Object utilities ******************************/

Scope* createScope(Object* owner, Scope* outer)
{
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    scope->objList = NULL;
    scope->owner = owner;
    scope->outer = outer;
    return scope;
}

Object* createProgramObject(char* programName)
{
    Object* program = (Object*)malloc(sizeof(Object));
    strcpy(program->name, programName);
    program->kind = OBJ_PROGRAM;
    program->progAttrs = (ProgramAttributes*)malloc(sizeof(ProgramAttributes));
    program->progAttrs->scope = createScope(program, NULL);

    symtab->program = program;

    return program;
}

Object* createConstantObject(char* name)
{
    Object* constantObject = (Object*)malloc(sizeof(Object));
    strcpy(constantObject->name, name);
    constantObject->kind = OBJ_CONSTANT;
    constantObject->constAttrs = (ConstantAttributes*)malloc(sizeof(ConstantAttributes));

    return constantObject;
}

Object* createTypeObject(char* name)
{
    Object* typeObject = (Object*)malloc(sizeof(Object));
    strcpy(typeObject->name, name);
    typeObject->kind = OBJ_TYPE;
    typeObject->typeAttrs = (TypeAttributes*)malloc(sizeof(TypeAttributes));
    typeObject->typeAttrs->actualType = NULL;

    return typeObject;
}

Object* createVariableObject(char* name)
{
    Object* variableObject = (Object*)malloc(sizeof(Object));
    strcpy(variableObject->name, name);
    variableObject->kind = OBJ_VARIABLE;
    variableObject->varAttrs = (VariableAttributes*)malloc(sizeof(VariableAttributes));
    variableObject->varAttrs->type = NULL;
    variableObject->varAttrs->scope = symtab->currentScope;

    return variableObject;
}

Object* createFunctionObject(char* name)
{
    Object* functionObject = (Object*)malloc(sizeof(Object));
    strcpy(functionObject->name, name);
    functionObject->kind = OBJ_FUNCTION;
    functionObject->funcAttrs = (FunctionAttributes*)malloc(sizeof(FunctionAttributes));
    functionObject->funcAttrs->returnType = NULL;
    functionObject->funcAttrs->paramList = NULL;
    functionObject->funcAttrs->scope = createScope(functionObject, symtab->currentScope);

    return functionObject;
}

Object* createProcedureObject(char* name)
{
    Object* procedureObject = (Object*)malloc(sizeof(Object));
    strcpy(procedureObject->name, name);
    procedureObject->kind = OBJ_PROCEDURE;
    procedureObject->procAttrs = (ProcedureAttributes*)malloc(sizeof(ProcedureAttributes));
    procedureObject->procAttrs->paramList = NULL;
    procedureObject->procAttrs->scope = createScope(procedureObject, symtab->currentScope);

    return procedureObject;
}

Object* createParameterObject(char* name, enum ParamKind kind, Object* owner)
{
    Object* parameterObject = (Object*)malloc(sizeof(Object));
    strcpy(parameterObject->name, name);
    parameterObject->kind = OBJ_PARAMETER;
    parameterObject->paramAttrs = (ParameterAttributes*)malloc(sizeof(ParameterAttributes));
    parameterObject->paramAttrs->kind = kind;
    parameterObject->paramAttrs->function = owner;
    parameterObject->paramAttrs->type = NULL;

    return parameterObject;
}

void freeObject(Object* obj)
{
    switch (obj->kind)
    {
    case OBJ_PROGRAM:
        freeScope(obj->progAttrs->scope);
        free(obj->progAttrs);
        break;
    case OBJ_TYPE:
        freeType(obj->typeAttrs->actualType);
        free(obj->typeAttrs);
        break;
    case OBJ_CONSTANT:
        free(obj->constAttrs);
        break;
    case OBJ_PROCEDURE:
        freeScope(obj->procAttrs->scope);
        freeReferenceList(obj->procAttrs->paramList);
        free(obj->procAttrs);
        break;
    case OBJ_FUNCTION:
        freeScope(obj->funcAttrs->scope);
        freeReferenceList(obj->funcAttrs->paramList);
        freeType(obj->funcAttrs->returnType);
        free(obj->funcAttrs);
        break;
    case OBJ_PARAMETER:
        freeType(obj->paramAttrs->type);
        free(obj->paramAttrs);
        break;
    case OBJ_VARIABLE:
        freeType(obj->varAttrs->type);
        free(obj->varAttrs);
        break;
    default:
        break;
    }

    free(obj);
}

void freeScope(Scope* scope)
{
    freeObjectList(scope->objList);
}

void freeObjectList(ObjectNode* objList)
{
    ObjectNode* current = objList;
    ObjectNode* toBeFree = NULL;

    while (current != NULL)
    {
        toBeFree = current;
        current = current->next;
        freeObject(toBeFree->object);
        free(toBeFree);
    }
}

void freeReferenceList(ObjectNode* objList)
{
    ObjectNode* current = objList;
    ObjectNode* toBeFree = NULL;

    while (current != NULL)
    {
        toBeFree = current;
        current = current->next;
        free(toBeFree);
    }
}

void addObject(ObjectNode** objList, Object* obj)
{
    ObjectNode* node = (ObjectNode*)malloc(sizeof(ObjectNode));
    node->object = obj;
    node->next = NULL;
    if ((*objList) == NULL)
    {
        *objList = node;
    }
    else
    {
        ObjectNode* n = *objList;
        while (n->next != NULL)
        {
            n = n->next;
        }
        n->next = node;
    }
}

Object* findObject(ObjectNode* objList, char* name)
{
    ObjectNode* current = objList;

    while (current != NULL)
    {
        if (strcmp(objList->object->name, name) == 0)
        {
            return objList->object;
        }

        current = current->next;
    }

    return NULL;
}

/******************* others ******************************/

void initSymTab(void)
{
    Object* obj;
    Object* param;

    symtab = (SymTab*)malloc(sizeof(SymTab));
    symtab->globalObjectList = NULL;

    obj = createFunctionObject("READC");
    obj->funcAttrs->returnType = makeCharType();
    addObject(&(symtab->globalObjectList), obj);

    obj = createFunctionObject("READI");
    obj->funcAttrs->returnType = makeIntType();
    addObject(&(symtab->globalObjectList), obj);

    obj = createProcedureObject("WRITEI");
    param = createParameterObject("i", PARAM_VALUE, obj);
    param->paramAttrs->type = makeIntType();
    addObject(&(obj->procAttrs->paramList), param);
    addObject(&(symtab->globalObjectList), obj);

    obj = createProcedureObject("WRITEC");
    param = createParameterObject("ch", PARAM_VALUE, obj);
    param->paramAttrs->type = makeCharType();
    addObject(&(obj->procAttrs->paramList), param);
    addObject(&(symtab->globalObjectList), obj);

    obj = createProcedureObject("WRITELN");
    addObject(&(symtab->globalObjectList), obj);

    intType = makeIntType();
    charType = makeCharType();
}

void cleanSymTab(void)
{
    freeObject(symtab->program);
    freeObjectList(symtab->globalObjectList);
    free(symtab);
    freeType(intType);
    freeType(charType);
}

void enterBlock(Scope* scope)
{
    symtab->currentScope = scope;
}

void exitBlock(void)
{
    symtab->currentScope = symtab->currentScope->outer;
}

void declareObject(Object* obj)
{
    if (obj->kind == OBJ_PARAMETER)
    {
        Object* owner = symtab->currentScope->owner;
        switch (owner->kind)
        {
        case OBJ_FUNCTION:
            addObject(&(owner->funcAttrs->paramList), obj);
            break;
        case OBJ_PROCEDURE:
            addObject(&(owner->procAttrs->paramList), obj);
            break;
        default:
            break;
        }
    }

    addObject(&(symtab->currentScope->objList), obj);
}
