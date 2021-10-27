#include "gassembly.h"
#include "gutils.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

//TODO switch to `sscanf_s`
//TODO check `fwrite` and `fputc` outp

char *findFirstExternalOp(const char *buffer, const char operation)
{
    char *opPos = (char*)buffer;  
    size_t bufferLen = strlen(buffer);
    size_t bracketBalance = 0;
    while (opPos < buffer + bufferLen
            && (*opPos != operation || bracketBalance != 0)) {
        if (*opPos == '[')
            ++bracketBalance;
        else if (*opPos == ']')
            --bracketBalance;
        ++opPos;
    }
    assert(opPos >= buffer);
    if (opPos != buffer && opPos != buffer + bufferLen)
        return opPos;
    else
        return NULL;
}

bool isMemCall(const char *buffer) 
{
    /*
     * WARNING buffer must be a null-terminated string
     */
    char  *openBrackPos = 0;
    char *closeBrackPos = 0;
    if ((openBrackPos = (char*)strchr(buffer, '[')) == NULL)
        return false;
    if ((closeBrackPos = findFirstExternalOp(openBrackPos + 1, ']')) == NULL)
        return false;

    /* checking that everything befor `[` is a space */
    char *iter = (char*)buffer;
    while (iter != openBrackPos) {
        if (!isspace(*iter))
            return false;
        ++iter;
    }

    /* checking that everything after `]` is a space */
    while (isspace(*(++closeBrackPos))) {}

    if (*closeBrackPos != '\0')
        return false;
    else 
        return true;
}

/**
 *
 * returns status:
 *              0 = OK
 *              1 = Empty
 *              2 = Parsing error in Literal      case
 *              3 = Parsing error in Register     case
 *              4 = Parsing error in Calculations case
 */
int gassembly_putOperand(const char *operand, FILE *out) 
{
    /*
     * WARNING: `operand` should be a null-terminated string
     */

    while (isspace(*operand))
        ++operand;

    // if (!gPtrValid(operand))
    //     return 0;
    // assert(!gPtrValid(out));

    size_t operandLen = strlen(operand);
    int isOk = 0;
    operandFormat format = {};
    char calcOp = 0;
    char *brackPos = NULL;
    
    if (operandLen == 0)
        return 1;

    if (strIsInteger(operand)) {
        /* Literal case */                     
        format.isMemCall  = 0;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        fwrite(&format, sizeof(format), 1, out);
        //fprintf(out, "|LIT_FORM|");

        SPU_VAL_TYPE val = 0;          
        sscanf(operand, "%i%n", &val, &isOk);
        assert(isOk != 0 && "ERROR: bad parsing of literal operand");
        fwrite(&val, sizeof(SPU_VAL_TYPE), 1, out);
        // fprintf(out, "|val = %d|\n", val);
    } else if (strnConsistsChrs(operand, DELIMS_LIST, GASSEMBLY_MAX_LINE_SIZE, GASSEMBLY_MAX_LINE_SIZE) == 0) {
        /* Register case */
        format.isMemCall  = 0;
        format.isRegister = 1;
        format.calculation = gCalc_none;
        fwrite(&format, sizeof(format), 1, out);            //TODO check if complete
        // fprintf(out, "|REG_FORM|");

        char reg = 0;
        sscanf(operand, "%cx%n", &reg, &isOk);
        // if (isOk == 0) {
        //    fprintf(stderr, "ERROR: bad parsing of register operand (more than one word provided?)\n");
        //    return 3;
        // }
        char regCode = reg - 'a' + 1;
        assert(regCode > 0);
        fputc(regCode, out);
        // fprintf(out, "|reg = %d|\n", regCode);
    } else if (isMemCall(operand)) {
        /* Mem call case */
        char subOperand[GASSEMBLY_MAX_LINE_SIZE] = {};
        char *openBrackPos = (char*)strchr(operand, '[');
        ++openBrackPos;
        char *closeBrackPos = findFirstExternalOp(openBrackPos, ']');
        assert(closeBrackPos > openBrackPos);
        strncpy(subOperand, openBrackPos, closeBrackPos - openBrackPos);

        format.isMemCall  = 1;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        fwrite(&format, sizeof(format), 1, out);            //TODO check if complete
        // fprintf(out, "|MEM_FORM|\n");
        
        // printf("In mem-call case subOperand = #%s#\n", subOperand);
        gassembly_putOperand(subOperand, out);
    } 
    else {
        /* Calculations case */
        char *opPos = findFirstExternalOp(operand, '+');


        if (opPos == NULL) {
            /* Mult case */
            format.isMemCall  = 0;
            format.isRegister = 0;
            format.calculation = gCalc_mul;
            fwrite(&format, sizeof(format), 1, out);
            //fprintf(out, "|MUL_FORM|\n");

            calcOp = '*';
            opPos = findFirstExternalOp(operand, '*');
        } else {
            /* Plus case */
            format.isMemCall  = 0;
            format.isRegister = 0;
            format.calculation = gCalc_add;
            fwrite(&format, sizeof(format), 1, out);
            //fprintf(out, "|SUM_FORM|\n");

            calcOp = '+';
        }

        if (opPos == NULL) {
            fprintf(stderr, "ERROR: parsing error, no `+` or `*` in a calculation\n");
            return 4;
        }
        assert(opPos > operand);
        
        char subOperand_1[GASSEMBLY_MAX_LINE_SIZE] = {};
        char subOperand_2[GASSEMBLY_MAX_LINE_SIZE] = {};

        strncpy(subOperand_1, operand, opPos - operand);
        ++opPos;
        strncpy(subOperand_2, opPos,  operandLen - (opPos - operand));

        // fprintf(stderr, "subOperand_1 = #%s#\n", subOperand_1);
        // fprintf(stderr, "subOperand_2 = #%s#\n", subOperand_2);
        int status_1 = gassembly_putOperand(subOperand_1, out);
        int status_2 = gassembly_putOperand(subOperand_2, out);
        if (status_1 != 0 || status_2 != 0)
            return 4;
    }
    return 0;
}

/** 
 * returns status:
 *              0 = OK
 *              1 = parsing error: extra `+`
 *              2 = parsing error: extra `*`
 *              3 = parsing error: bad brackets
 *              4 = parsing error: `+` and `*` in succession
 */
int gassembly_parseOperand(const char *buffer, char *operand_1, char *operand_2) {
    const size_t bufferLen = strlen(buffer);
    char *delim = (char*)buffer;
    char *firstOperand  = 0;
    char *secondOperand = 0;
    size_t bracketBalance = 0;
    size_t paramCnt   = 0;
    bool   wordOpen   = false;
    bool   sumOpen    = false;
    bool   mulOpen    = false;

    while (delim - buffer < bufferLen && paramCnt < 2) {
        assert(delim - buffer >= 0);
        bool wordWasOpened = wordOpen;
        size_t oldParamCnt = paramCnt;
        
        if (isspace(*delim) || strchr(DELIMS_LIST, *delim) != NULL) {
            wordOpen = false;
        } else if (!wordOpen)
            wordOpen = true;

        if (*delim == '[')
            ++bracketBalance;
        else if (*delim == ']')
            --bracketBalance;
        else if (*delim == '+') {
            if (sumOpen)
                return 1;
            sumOpen = true;
        } else if (*delim == '*') {
            if (mulOpen)
                return 2;
            mulOpen = true;
        }

        if (!wordWasOpened && !wordOpen
                && !mulOpen
                && !sumOpen
                && *delim == '[') {
            ++paramCnt;
            wordOpen = true;
        }

        if (wordOpen && !wordWasOpened 
                && bracketBalance == 0 
                && !mulOpen
                && !sumOpen)
            ++paramCnt;
 
        if (wordOpen && !wordWasOpened) {
            if (mulOpen && sumOpen)
                return 4;
            sumOpen = mulOpen = false;
        }
       
        // printf("curDelim = %c \t paramCnt = %d\n", *delim, paramCnt); //DEBUG
        
        if (paramCnt != oldParamCnt) {
            if (firstOperand == NULL)
                firstOperand = delim;
            else
                secondOperand = delim;
        }
        ++delim;
    }
    if (firstOperand  != NULL && secondOperand != NULL) {
        strncpy(operand_1, firstOperand, secondOperand - firstOperand);
        strncpy(operand_2, secondOperand, bufferLen - (secondOperand - buffer));
    }
    else if (firstOperand != NULL) 
        strncpy(operand_1, firstOperand, bufferLen - (firstOperand - buffer));
    return 0;
}

void getline(char *buffer, size_t bufferLen, FILE *in) 
{
    //TODO check pointers?
    char c;
    size_t curLen = 0;
    c = fgetc(in);
    while (!feof(in) && c != '\n') {
        if (curLen < bufferLen - 1) 
            buffer[curLen++] = c;
        else 
            goto finish;
        c = fgetc(in);
    }
finish:
    buffer[curLen++] = '\0';
}

void gassembly_assembleFromFile(FILE *in, FILE *out) 
{
    char buffer[GASSEMBLY_MAX_LINE_SIZE] = {};

    getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in);
    while (!feof(in)) {
        // fprintf(stderr, "Got line = #%s#\n", buffer);
        gassembly_assembleFromLine(buffer, out);
        getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in);
    }
}

int gOpcodeByKeyword(char *keyword)
{              
    size_t i = 0;
    while (i < gCnt && strcmp(keyword, gDisassambleTable[i])) {
        ++i;
    }
    if (i == gCnt)
        return -1;
    return i;
}

void gassembly_assembleFromLine(const char *buffer, FILE *out) 
{
    /* 
     * WARNING: `buffer` must be a null terminated
     * string with len eq GASSEMBLY_MAX_LINE_SIZE 
     */
    // assert(!gPtrValid(buffer));
    // assert(!gPtrValid(out));

    /* cropping comments that start with `;` */
    char *commentPos = (char*)strchr(buffer, ';');
    if (commentPos != NULL)
        *commentPos = '\0';
    
    /* checking if line is only spaces */
    while (isspace(*buffer)) 
        ++buffer;

    if (*buffer == '\0')
        return;

    char   keyword[GASSEMBLY_MAX_LINE_SIZE] = {};
    char operand_1[GASSEMBLY_MAX_LINE_SIZE] = {};
    char operand_2[GASSEMBLY_MAX_LINE_SIZE] = {};
    sscanf(buffer, "%s", keyword);
    fputc(gOpcodeByKeyword(keyword), out);        //TODO implement dictionary for opcodes
    
    /* 
     * WARNING: by now buffer must not have opening spaces
     */
    char *operands = (char*)buffer;
    while(*operands != '\0' && !isspace(*operands) && *operands != '[')
        ++operands;

    // fprintf(stderr, "Before! operand_1 = #%s#\noperand_2 = #%s#\n", operand_1, operand_2);
    // fprintf(stderr, "Operands = #%s#\n", operands);
    gassembly_parseOperand(operands, operand_1, operand_2);
    // fprintf(stderr, "operand_1 = #%s#\noperand_2 = #%s#\n", operand_1, operand_2);
    if (*operand_1 != '\0')
        gassembly_putOperand(operand_1, out);                   //TODO handle returned error codes
    if (*operand_2 != '\0')
        gassembly_putOperand(operand_2, out);
    
    /* writing empty format as the end of operands */
    operandFormat emptyFormat = {};
    fwrite(&emptyFormat, sizeof(emptyFormat), 1, out);
}

bool gassembly_formatIsEmpty(const operandFormat format)
{
    static operandFormat operandFormatZeroReference = {};
    return !memcmp(&format, &operandFormatZeroReference, sizeof(operandFormat));
}

void gassembly_formatDump(const operandFormat format, FILE *out)
{
    fprintf(out, "%s\nFormat dump:\n", LOG_DELIM);
    fprintf(out, "isRegister  = %d\n", format.isRegister);
    fprintf(out, "isMemCall   = %d\n", format.isMemCall);
    fprintf(out, "calculation = %d\n", format.calculation);
    fprintf(out, "%s\n", LOG_DELIM);
}

bool gassembly_formatVerify(const operandFormat format)
{
   if (gassembly_formatIsEmpty(format))
        return true;

    if (format.isMemCall) 
        return (format.calculation == gCalc_none);

    if (format.isRegister) 
        return (!format.isMemCall && (format.calculation == gCalc_none));

    if (format.calculation != gCalc_none)
        return (!format.isMemCall && (format.calculation != gCalc_empty));

    return true;
}

/** 
 * returns:
 *          0 - OK, got operand
 *          1 - OK, zero operand format, operand list end
 *          2 - ERROR: error in memory call (no suboperand provided?)
 *          3 - ERROR: error in calculation (no suboperand provided?)
 *          4 - ERROR: error in register 
 *          5 - ERROR: error in literal
 *          6 - ERROR: bad format provided
 *          7 - ERROR: file reading failed
 */
int gassembly_getOperand(FILE *in, FILE *out) 
{
    operandFormat format;
    if (!fread(&format, sizeof(format), 1, in)) {
        fprintf(stderr, "failed to read format!\n");
        if (feof(in) || ferror(in))
            return 7;
        return 6;
    }
    // gassembly_formatDump(format, stderr);


    if (!gassembly_formatVerify(format)) {
        // gassembly_formatDump(format, logOut); //TODO optional log errors
        return 6;
    }

    if (gassembly_formatIsEmpty(format)) {
        return 1;
    }
    if (format.isMemCall) {
        fprintf(out, "[");
        if (gassembly_getOperand(in, out) != 0)
            return 2;
        fprintf(out, "]");
    } else if (format.calculation != gCalc_none) {
        if (gassembly_getOperand(in, out) != 0)
            return 3;

        fprintf(out, " %c ", DELIMS_LIST[format.calculation]);   
        
        if (gassembly_getOperand(in, out) != 0)
            return 3;
    } else if (format.isRegister) {
        char regCode = 0;
        regCode = fgetc(in);

        if (regCode == EOF)
            return 7;
        
        if (regCode < 1 || regCode > 100)
            return 4;

        fprintf(out, "%cx", regCode + 'a' - 1);
    } else {
        SPU_VAL_TYPE val = 0;
        if (!fread(&val, sizeof(SPU_VAL_TYPE), 1, in)) {
            if (feof(in) || ferror(in))
                return 7;
            return 5;
        }

        fprintf(out, "0x%X", val);
    }
    return 0;
}

void gassembly_disassembleFromFile(FILE *in, FILE *out) 
{
    char opcode;
    fread(&opcode, sizeof(char), 1, in);
    while (!feof(in)) {
        assert(opcode < gCnt);
        fprintf(out, "%s ", gDisassambleTable[opcode]);
        int status = 0;
        while ((status = gassembly_getOperand(in, out)) == 0) 
            fputc(' ', out);

        if (status != 1)
            fprintf(stderr, "Error occured while reading operand, error_code = %d\n", status);
        
        fprintf(out, "\n");
        fread(&opcode, sizeof(char), 1, in);
    }
}
