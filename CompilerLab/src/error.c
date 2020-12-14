/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include "error.h"
#include <stdio.h>
#include <stdlib.h>

void error(ErrorCode err, int lineNo, int colNo)
{
    switch (err)
    {
    case ERR_END_OF_COMMENT:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_END_OF_COMMENT);
        break;
    case ERR_IDENT_TOO_LONG:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_IDENT_TOO_LONG);
        break;
    case ERR_NUM_LITERAL_TOO_LONG:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_NUM_LITERAL_TOO_LONG);
    case ERR_INVALID_CHAR_CONSTANT:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_CHAR_CONSTANT);
        break;
    case ERR_INVALID_SYMBOL:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_SYMBOL);
        break;
    case ERR_INTERNAL_ERROR:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INTERNAL_ERROR);
        break;
    case ERR_INVALID_CONSTANT:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_CONSTANT);
        break;
    case ERR_INVALID_TYPE:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_TYPE);
        break;
    case ERR_INVALID_BASIC_TYPE:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_BASIC_TYPE);
        break;
    case ERR_INVALID_PARAMETER:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_PARAMETER);
        break;
    case ERR_INVALID_STATEMENT:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_STATEMENT);
        break;
    case ERR_INVALID_ARGUMENTS:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_ARGUMENTS);
        break;
    case ERR_INVALID_COMPARATOR:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_COMPARATOR);
        break;
    case ERR_INVALID_EXPRESSION:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_EXPRESSION);
        break;
    case ERR_INVALID_TERM:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_TERM);
        break;
    case ERR_INVALID_FACTOR:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_FACTOR);
        break;
    case ERR_INVALID_VARIABLE:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_VARIABLE);
        break;
    case ERR_INVALID_FUNCTION:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_FUNCTION);
        break;
    case ERR_INVALID_PROCEDURE:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_PROCEDURE);
        break;
    case ERR_INVALID_LVALUE:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_INVALID_LVALUE);
        break;
    case ERR_UNDECLARED_IDENT:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_UNDECLARED_IDENT);
        break;
    case ERR_UNDECLARED_CONSTANT:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_UNDECLARED_CONSTANT);
        break;
    case ERR_UNDECLARED_INT_CONSTANT:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_UNDECLARED_INT_CONSTANT);
        break;
    case ERR_UNDECLARED_TYPE:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_UNDECLARED_TYPE);
        break;
    case ERR_UNDECLARED_VARIABLE:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_UNDECLARED_VARIABLE);
        break;
    case ERR_UNDECLARED_FUNCTION:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_UNDECLARED_FUNCTION);
        break;
    case ERR_UNDECLARED_PROCEDURE:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_UNDECLARED_PROCEDURE);
        break;
    case ERR_DUPLICATE_IDENT:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_DUPLICATE_IDENT);
        break;
    case ERR_TYPE_INCONSISTENCY:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_TYPE_INCONSISTENCY);
        break;
    case ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY:
        printf("%d-%d:%s\n", lineNo, colNo, ERM_PARAMETERS_ARGUMENTS_INCONSISTENCY);
        break;
    }
    exit(-1);
}

void missingToken(TokenType tokenType, int lineNo, int colNo)
{
    printf("%d-%d:Missing %s\n", lineNo, colNo, tokenToString(tokenType));
    exit(0);
}

void assert(char* msg)
{
    printf("%s\n", msg);
}
