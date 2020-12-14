/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#ifndef __ERROR_H__
#define __ERROR_H__
#include "token.h"

typedef enum
{
    ERR_END_OF_COMMENT,
    ERR_IDENT_TOO_LONG,
    ERR_NUM_LITERAL_TOO_LONG,
    ERR_INVALID_CHAR_CONSTANT,
    ERR_INVALID_SYMBOL,
    ERR_INTERNAL_ERROR,

    ERR_INVALID_CONSTANT,
    ERR_INVALID_TYPE,
    ERR_INVALID_BASIC_TYPE,
    ERR_INVALID_PARAMETER,
    ERR_INVALID_STATEMENT,
    ERR_INVALID_ARGUMENTS,
    ERR_INVALID_COMPARATOR,
    ERR_INVALID_EXPRESSION,
    ERR_INVALID_TERM,
    ERR_INVALID_FACTOR,
    ERR_INVALID_VARIABLE,
    ERR_INVALID_FUNCTION,
    ERR_INVALID_PROCEDURE,
    
    ERR_INVALID_LVALUE,

    ERR_UNDECLARED_IDENT,
    ERR_UNDECLARED_CONSTANT,
    ERR_UNDECLARED_INT_CONSTANT,
    ERR_UNDECLARED_TYPE,
    ERR_UNDECLARED_VARIABLE,
    ERR_UNDECLARED_FUNCTION,
    ERR_UNDECLARED_PROCEDURE,
    ERR_DUPLICATE_IDENT,
    ERR_TYPE_INCONSISTENCY,
    ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY
} ErrorCode;

#define ERM_END_OF_COMMENT "End of comment expected!"
#define ERM_IDENT_TOO_LONG "Identification too long!"
#define ERM_NUM_LITERAL_TOO_LONG "Numeric literal too long!"
#define ERM_INVALID_CHAR_CONSTANT "Invalid const char!"
#define ERM_INVALID_SYMBOL "Invalid symbol!"
#define ERM_INTERNAL_ERROR "Internal error!"

#define ERM_INVALID_CONSTANT "Invalid constant!"
#define ERM_INVALID_TYPE "Invalid type!"
#define ERM_INVALID_BASIC_TYPE "Invalid basic type!"
#define ERM_INVALID_PARAMETER "Invalid parameter!"
#define ERM_INVALID_STATEMENT "Invalid statement!"
#define ERM_INVALID_ARGUMENTS "Invalid arguments!"
#define ERM_INVALID_COMPARATOR "Invalid comparator!"
#define ERM_INVALID_EXPRESSION "Invalid expression!"
#define ERM_INVALID_TERM "Invalid term!"
#define ERM_INVALID_FACTOR "Invalid factor!"

#define ERM_INVALID_VARIABLE "A variable expected."
#define ERM_INVALID_FUNCTION "A function identifier expected."
#define ERM_INVALID_PROCEDURE "A procedure identifier expected."

#define ERM_INVALID_LVALUE "Invalid lvalue in assignment."

#define ERM_UNDECLARED_IDENT "Undeclared identifier."
#define ERM_UNDECLARED_CONSTANT "Undeclared constant."
#define ERM_UNDECLARED_INT_CONSTANT "Undeclared integer constant."
#define ERM_UNDECLARED_TYPE "Undeclared type."
#define ERM_UNDECLARED_VARIABLE "Undeclared variable."
#define ERM_UNDECLARED_FUNCTION "Undeclared function."
#define ERM_UNDECLARED_PROCEDURE "Undeclared procedure."
#define ERM_DUPLICATE_IDENT "Duplicate identifier."
#define ERM_TYPE_INCONSISTENCY "Type inconsistency"
#define ERM_PARAMETERS_ARGUMENTS_INCONSISTENCY "The number of arguments and the number of parameters are inconsistent."

void error(ErrorCode err, int lineNo, int colNo);
void missingToken(TokenType tokenType, int lineNo, int colNo);
void assert(char* msg);

#endif
