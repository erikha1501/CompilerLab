/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"

extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];

CharCode currentCharCode;
int state = -1;

/***************************************************************/

/// <summary>
/// Helper function for reading next CharCode
/// </summary>
void readCharCode()
{
    readChar();
    if (currentChar >= 0)
    {
        currentCharCode = charCodes[currentChar];
    }
    else
    {
        currentCharCode = CHAR_UNKNOWN;
    }
}

void skipBlank()
{
    while (state == 1 && currentCharCode == CHAR_SPACE)
    {
        readCharCode();
    }
    state = 0;
}

void skipBlockComment()
{
    while (1)
    {
        switch (state)
        {
        case 3:
            if (currentChar == EOF)
            {
                state = 40;
            }
            else if (currentCharCode == CHAR_TIMES)
            {
                readCharCode();
                state = 4;
            }
            else
            {
                readCharCode();
                state = 3;
            }
            break;

        case 4:
            if (currentChar == EOF)
            {
                state = 40;
            }
            else if (currentCharCode == CHAR_TIMES)
            {
                readCharCode();
                state = 4;
            }
            else if (currentCharCode == CHAR_RPAR)
            {
                readCharCode();
                state = 5;
            }
            else
            {
                state = 3;
            }
            break;

        case 5:
            state = 0;
            return;

        case 40:
            error(ERR_ENDOFCOMMENT, lineNo, colNo);
            return;

        default:
            break;
        }
    }
}

void skipLineComment()
{
    while (1)
    {
        if (currentChar == '\n')
        {
            readCharCode();
            return;
        }

        readCharCode();
    }
}

Token* readIdentKeyword(void)
{
    int startLineNo = lineNo;
    int startColNo = colNo;

    int identifierLength = 0;
    char buf[MAX_IDENT_LEN + 1];

    while (1)
    {
        switch (state)
        {
        case 8:
            if (currentCharCode == CHAR_LETTER || currentCharCode == CHAR_DIGIT)
            {
                if (identifierLength > MAX_IDENT_LEN)
                {
                    error(ERR_IDENTTOOLONG, lineNo, colNo);
                    state = -1;
                    return makeToken(TK_NONE, startLineNo, startColNo);
                }

                buf[identifierLength++] = currentChar;
                readCharCode();
                state = 8;
            }
            else
            {
                state = 9;
            }
            break;
        case 9:
            buf[identifierLength] = '\0';

            TokenType keywordType = checkKeyword(buf);
            TokenType tokenType = keywordType == TK_NONE ? TK_IDENT : keywordType;

            Token* token = makeToken(tokenType, startLineNo, startColNo);
            // Copy lexeme to token->string.
#ifdef _MSC_VER
            strncpy_s(token->string, sizeof(token->string), buf, identifierLength);
#else
            strncpy(token->string, buf, identifierLength);
#endif
            state = 0;
            return token;
        default:
            break;
        }
    }
}

Token* readNumber(void)
{
    int startLineNo = lineNo;
    int startColNo = colNo;

    int numberLength = 0;
    char buf[MAX_NUM_LEN + 1];

    while (1)
    {
        switch (state)
        {
        case 10:
            if (currentCharCode == CHAR_DIGIT)
            {
                if (numberLength > MAX_NUM_LEN)
                {
                    error(ERR_NUMLITERALTOOLONG, lineNo, colNo);
                    state = -1;
                    return makeToken(TK_NONE, startLineNo, startColNo);
                }

                buf[numberLength++] = currentChar;
                readCharCode();
                state = 10;
            }
            else
            {
                state = 11;
            }
            break;
        case 11:
            buf[numberLength] = '\0';

            Token* numberToken = makeToken(TK_NUMBER, startLineNo, startColNo);

            // Copy lexeme to token->string.
#ifdef _MSC_VER
            strncpy_s(numberToken->string, sizeof(numberToken->string), buf, numberLength);
#else
            strncpy(numberToken->string, buf, numberLength);
#endif
            numberToken->value = atoi(buf);

            state = 0;
            return numberToken;
        default:
            break;
        }
    }
}

Token* readConstChar(void)
{
    int startLineNo = lineNo;
    int startColNo = colNo;
    int charValue;

    readCharCode();

    // Check if currentChar is a printable character.
    if (currentChar >= 0x20 && currentChar <= 0x7E)
    {
        charValue = currentChar;

        readCharCode();
        if (currentCharCode == CHAR_SINGLEQUOTE)
        {
            readCharCode();
            Token* constCharToken = makeToken(TK_CHAR, startLineNo, startColNo);
            constCharToken->value = charValue;
            constCharToken->string[0] = charValue;
            constCharToken->string[1] = '\0';

            state = 0;
            return constCharToken;
        }
    }

    error(ERR_INVALIDCHARCONSTANT, lineNo, colNo);
    state = -1;
    return makeToken(TK_NONE, startLineNo, startColNo);
}

Token* getToken(void)
{
    Token* token;
    int startLineNo, startColNo;

    if (currentChar == EOF)
        return makeToken(TK_EOF, lineNo, colNo);

    switch (charCodes[currentChar])
    {
    case CHAR_SPACE:
        state = 1;
        skipBlank();
        return getToken();

    case CHAR_LETTER:
        state = 8;
        return readIdentKeyword();

    case CHAR_DIGIT:
        state = 10;
        return readNumber();

    case CHAR_PLUS:
        token = makeToken(SB_PLUS, lineNo, colNo);
        readCharCode();
        state = 0;
        return token;

    case CHAR_MINUS:
        token = makeToken(SB_MINUS, lineNo, colNo);
        readCharCode();
        state = 0;
        return token;

    case CHAR_TIMES:
        token = makeToken(SB_TIMES, lineNo, colNo);
        readCharCode();
        state = 0;
        return token;

    case CHAR_SLASH:
        token = makeToken(SB_SLASH, lineNo, colNo);
        readCharCode();
        state = 0;
        return token;

    case CHAR_EQ:
        token = makeToken(SB_EQ, lineNo, colNo);
        readCharCode();
        state = 0;
        return token;

    case CHAR_LPAR:
        startLineNo = lineNo;
        startColNo = colNo;

        readCharCode();
        if (currentCharCode == CHAR_PERIOD)
        {
            readCharCode();
            return makeToken(SB_LSEL, startLineNo, startColNo);
        }
        else if (currentCharCode == CHAR_TIMES)
        {
            readCharCode();
            state = 3;
            skipBlockComment();
            return getToken();
        }
        else
        {
            state = 0;
            return makeToken(SB_LPAR, startLineNo, startColNo);
        }

    case CHAR_SINGLEQUOTE:
        return readConstChar();

    case CHAR_LT:
        startLineNo = lineNo;
        startColNo = colNo;

        readCharCode();
        if (currentCharCode == CHAR_EQ)
        {
            readCharCode();
            state = 0;
            return makeToken(SB_LE, startLineNo, startColNo);
        }
        else
        {
            state = 0;
            return makeToken(SB_LT, startLineNo, startColNo);
        }

    case CHAR_GT:
        startLineNo = lineNo;
        startColNo = colNo;

        readCharCode();
        if (currentCharCode == CHAR_EQ)
        {
            readCharCode();
            state = 0;
            return makeToken(SB_GE, startLineNo, startColNo);
        }
        else
        {
            state = 0;
            return makeToken(SB_GT, startLineNo, startColNo);
        }

    case CHAR_EXCLAIMATION:
        startLineNo = lineNo;
        startColNo = colNo;

        readCharCode();
        if (currentCharCode == CHAR_EQ)
        {
            readCharCode();
            state = 0;
            return makeToken(SB_NEQ, startLineNo, startColNo);
        }
        else
        {
            error(ERR_INVALIDSYMBOL, lineNo, colNo);
            state = -1;
            return makeToken(TK_NONE, startLineNo, startColNo);
        }

    case CHAR_PERIOD:
        startLineNo = lineNo;
        startColNo = colNo;

        readCharCode();
        if (currentCharCode == CHAR_RPAR)
        {
            readCharCode();
            state = 0;
            return makeToken(SB_RSEL, startLineNo, startColNo);
        }
        else
        {
            state = 0;
            return makeToken(SB_PERIOD, startLineNo, startColNo);
        }

    case CHAR_COLON:
        startLineNo = lineNo;
        startColNo = colNo;

        readCharCode();
        if (currentCharCode == CHAR_EQ)
        {
            readCharCode();
            state = 0;
            return makeToken(SB_ASSIGN, startLineNo, startColNo);
        }
        else
        {
            state = 0;
            return makeToken(SB_COLON, startLineNo, startColNo);
        }

    case CHAR_COMMA:
        token = makeToken(SB_COMMA, lineNo, colNo);
        readCharCode();
        state = 0;
        return token;

    case CHAR_SEMICOLON:
        token = makeToken(SB_SEMICOLON, lineNo, colNo);
        readCharCode();
        state = 0;
        return token;

    case CHAR_RPAR:
        token = makeToken(SB_RPAR, lineNo, colNo);
        readCharCode();
        state = 0;
        return token;

    case CHAR_DOUBLEQUOTE:
        readCharCode();
        skipLineComment();
        state = 0;
        return getToken();

    default:
        token = makeToken(TK_NONE, lineNo, colNo);
        error(ERR_INVALIDSYMBOL, lineNo, colNo);
        readCharCode();
        return token;
    }
}

/******************************************************************/

void printToken(Token* token)
{

    printf("%d-%d:", token->lineNo, token->colNo);

    switch (token->tokenType)
    {
    case TK_NONE:
        printf("TK_NONE\n");
        break;
    case TK_IDENT:
        printf("TK_IDENT(%s)\n", token->string);
        break;
    case TK_NUMBER:
        printf("TK_NUMBER(%s)\n", token->string);
        break;
    case TK_CHAR:
        printf("TK_CHAR(\'%s\')\n", token->string);
        break;
    case TK_EOF:
        printf("TK_EOF\n");
        break;

    case KW_PROGRAM:
        printf("KW_PROGRAM\n");
        break;
    case KW_CONST:
        printf("KW_CONST\n");
        break;
    case KW_TYPE:
        printf("KW_TYPE\n");
        break;
    case KW_VAR:
        printf("KW_VAR\n");
        break;
    case KW_INTEGER:
        printf("KW_INTEGER\n");
        break;
    case KW_CHAR:
        printf("KW_CHAR\n");
        break;
    case KW_ARRAY:
        printf("KW_ARRAY\n");
        break;
    case KW_OF:
        printf("KW_OF\n");
        break;
    case KW_FUNCTION:
        printf("KW_FUNCTION\n");
        break;
    case KW_PROCEDURE:
        printf("KW_PROCEDURE\n");
        break;
    case KW_BEGIN:
        printf("KW_BEGIN\n");
        break;
    case KW_END:
        printf("KW_END\n");
        break;
    case KW_CALL:
        printf("KW_CALL\n");
        break;
    case KW_IF:
        printf("KW_IF\n");
        break;
    case KW_THEN:
        printf("KW_THEN\n");
        break;
    case KW_ELSE:
        printf("KW_ELSE\n");
        break;
    case KW_WHILE:
        printf("KW_WHILE\n");
        break;
    case KW_DO:
        printf("KW_DO\n");
        break;
    case KW_FOR:
        printf("KW_FOR\n");
        break;
    case KW_TO:
        printf("KW_TO\n");
        break;

    case SB_SEMICOLON:
        printf("SB_SEMICOLON\n");
        break;
    case SB_COLON:
        printf("SB_COLON\n");
        break;
    case SB_PERIOD:
        printf("SB_PERIOD\n");
        break;
    case SB_COMMA:
        printf("SB_COMMA\n");
        break;
    case SB_ASSIGN:
        printf("SB_ASSIGN\n");
        break;
    case SB_EQ:
        printf("SB_EQ\n");
        break;
    case SB_NEQ:
        printf("SB_NEQ\n");
        break;
    case SB_LT:
        printf("SB_LT\n");
        break;
    case SB_LE:
        printf("SB_LE\n");
        break;
    case SB_GT:
        printf("SB_GT\n");
        break;
    case SB_GE:
        printf("SB_GE\n");
        break;
    case SB_PLUS:
        printf("SB_PLUS\n");
        break;
    case SB_MINUS:
        printf("SB_MINUS\n");
        break;
    case SB_TIMES:
        printf("SB_TIMES\n");
        break;
    case SB_SLASH:
        printf("SB_SLASH\n");
        break;
    case SB_LPAR:
        printf("SB_LPAR\n");
        break;
    case SB_RPAR:
        printf("SB_RPAR\n");
        break;
    case SB_LSEL:
        printf("SB_LSEL\n");
        break;
    case SB_RSEL:
        printf("SB_RSEL\n");
        break;
    }
}

int scan(char* fileName)
{
    Token* token;

    if (openInputStream(fileName) == IO_ERROR)
        return IO_ERROR;
    currentCharCode = charCodes[currentChar];

    state = 0;
    while (1)
    {
        token = getToken();
        if (token->tokenType == TK_EOF)
        {
            break;
        }

        printToken(token);
        free(token);
    }

    closeInputStream();
    return IO_SUCCESS;
}

/******************************************************************/

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        printf("scanner: no input file.\n");
        return -1;
    }

    if (scan(argv[1]) == IO_ERROR)
    {
        printf("Can\'t read input file!\n");
        return -1;
    }

    return 0;
}
