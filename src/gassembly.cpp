#include "gassembly.h"
#include "gutils.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


char *findFirstExternalOp(const char *buffer, const char operation)
{
    char *opPos = (char*)buffer;  
    size_t bufferLen = strlen(buffer);
    size_t memBracketBalance  = 0;
    size_t calcBracketBalance = 0;
    while (opPos < buffer + bufferLen
            && (*opPos != operation || calcBracketBalance != 0 || memBracketBalance != 0)) {
        if (*opPos == '[')
            ++memBracketBalance;
        else if (*opPos == ']')
            --memBracketBalance;
        else if (*opPos == '(')
            ++calcBracketBalance;
        else if (*opPos == ')')
            --calcBracketBalance;
        ++opPos;
    }
    assert(opPos >= buffer);
    if (opPos != buffer && opPos != buffer + bufferLen)
        return opPos;
    else
        return NULL;
}

static bool isMemCall(const char *buffer) 
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

static bool isRegister(const char *buffer) 
{
    char *iter = (char*)buffer;
    while (isspace(*iter)) 
        ++iter;
    if (iter - buffer < strlen(buffer) + 1
            && (*iter >= 'a' && *iter - 'a' < MAX_REGISTERS)
            && (*(iter + 1) == 'x'))
            return true;
    return false;
}

static bool isLable(const char *buffer) 
{
    if (isRegister(buffer))
        return false;

    char *iter = (char*)buffer;
    while (isspace(*iter)) 
        ++iter;
    
    if (!isalpha(*iter) || *iter == '\0')           //TODO maybe add local lable support (like `.loop`)
        return false;
    ++iter;

    while (*iter != '\0') {
        if (!isalpha(*iter) && !isdigit(*iter) && *iter != '_')
            return false;
        ++iter;
    }
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
 *              5 = Parsing error in Lable        case
 *              6 = Bad operand ptr
 *
 */
gassembly_status gassembly_putOperand(const char *operand, FILE *out) 
{
    /*
     * WARNING: `operand` should be a null-terminated string
     */

    if (!gPtrValid(operand))
        return gassembly_status_BadOpPtr;

    if (!gPtrValid(operand))
        return gassembly_status_ErrFile;


    /* cropping opening spaces */
    while (isspace(*operand))
        ++operand;

    size_t operandLen = strlen(operand);

    /* cropping closing spaces */
    char *iter = (char*)(operand + operandLen - 1);
    while (isspace(*iter)) {
        --iter;
        --operandLen;
    }
    *(iter + 1) = '\0';

    if (operandLen == 0)
        return gassembly_status_Empty;

    int isOk = 0;
    operandFormat format = {};
    char calcOp = 0;
    char *brackPos = NULL;

    if (isInteger(operand)) {
        /* Literal integer case */                     
        format.isMemCall  = 0;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        if (fwrite(&format, sizeof(format), 1, out) != 1)
            return gassembly_status_ErrFile;

        SPU_INTEG_TYPE val = 0;          
        if (sscanf(operand, "%lli%n", &val, &isOk) != 1)
            return gassembly_status_ErrLit;

        if (fwrite(&val, sizeof(SPU_INTEG_TYPE), 1, out) != 1)
            return gassembly_status_ErrFile;

    } else if (isDouble(operand)) {
        /* Literal double case */
        format.isMemCall  = 0;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        if (fwrite(&format, sizeof(format), 1, out) != 1)
            return gassembly_status_ErrFile;

        SPU_FLOAT_TYPE val = 0;          
        if (sscanf(operand, "%lg%n", &val, &isOk) != 1)
            return gassembly_status_ErrLit;

        if (fwrite(&val, sizeof(SPU_FLOAT_TYPE), 1, out) != 1)
            return gassembly_status_ErrFile;
    } else if (isRegister(operand)) {
        /* Register case */
        format.isMemCall  = 0;
        format.isRegister = 1;
        format.calculation = gCalc_none;
        if (fwrite(&format, sizeof(format), 1, out) != 1)
            return gassembly_status_ErrFile;

        char reg = 0;
        if (sscanf(operand, "%cx%n", &reg, &isOk) != 1)
            return gassembly_status_ErrReg;

        char regCode = reg - 'a' + 1;
        assert(regCode > 0);

        if (fputc(regCode, out) == EOF)
            return gassembly_status_ErrFile;

    } else if (isLable(operand)) {
        /* Lable case */
        format.isMemCall  = 0;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        if (fwrite(&format, sizeof(format), 1, out) != 1)  
            return gassembly_status_ErrFile;

        char lable[GASSEMBLY_MAX_LINE_SIZE] = {};

        if (sscanf(operand, "%s", lable) != 1)
            return gassembly_status_ErrLable;
        size_t i = 0;
        while (i < GASSEMBLY_MAX_LABLES && strcmp(gassembly_Lables[i], lable) != 0)
            ++i;
            if (i == GASSEMBLY_MAX_LABLES)      // Crutch for UB cases with jmp to unknown lable
            --i;

        if (fwrite(&gassembly_Fixups[i], sizeof(SPU_INTEG_TYPE), 1, out) != 1)
            return gassembly_status_ErrFile;

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
        if (fwrite(&format, sizeof(format), 1, out) != 1)
            return gassembly_status_ErrFile;
        
        gassembly_putOperand(subOperand, out);
    } 
    else if (*operand == '(' && *(operand + operandLen - 1) == ')' ) {
        /* Brackets calculation case */      
        
        char subOperand[GASSEMBLY_MAX_LINE_SIZE] = {};
        strncpy(subOperand, operand + 1, operandLen - 2);

        gassembly_putOperand(subOperand, out);
    } else {
        /* Resolving calculations case */
        char *addPos = findFirstExternalOp(operand, '+');
        char *subPos = findFirstExternalOp(operand, '-');
        char *opPos  = NULL;

        if (addPos == NULL && subPos == NULL) {
            /* Mult case */
            format.isMemCall  = 0;
            format.isRegister = 0;
            format.calculation = gCalc_mul;
            if (fwrite(&format, sizeof(format), 1, out) != 1)
                return gassembly_status_ErrFile;

            calcOp = '*';
            opPos = findFirstExternalOp(operand, '*');
        } else if (addPos != NULL && (subPos == NULL || addPos < subPos)) {
            /* Plus case */
            format.isMemCall  = 0;
            format.isRegister = 0;
            format.calculation = gCalc_add;
            if (fwrite(&format, sizeof(format), 1, out) != 1)
                return gassembly_status_ErrFile;
            
            opPos = addPos;
            calcOp = '+';
        } else {
            /* Minus case */
            format.isMemCall  = 0;
            format.isRegister = 0;
            format.calculation = gCalc_sub;
            if (fwrite(&format, sizeof(format), 1, out) != 1)
                return gassembly_status_ErrFile;
            
            opPos = subPos;
            calcOp = '-';
        }

        if (opPos == NULL) {
            fprintf(stderr, "ERROR: parsing error, no `+` or `-` or `*` in a calculation\n");
            return gassembly_status_ErrCalc;
        }
        assert(opPos > operand);
        
        char subOperand_1[GASSEMBLY_MAX_LINE_SIZE] = {};
        char subOperand_2[GASSEMBLY_MAX_LINE_SIZE] = {};

        strncpy(subOperand_1, operand, opPos - operand);
        ++opPos;
        strncpy(subOperand_2, opPos,  operandLen - (opPos - operand));

        int status_1 = gassembly_putOperand(subOperand_1, out);
        int status_2 = gassembly_putOperand(subOperand_2, out);
        if (status_1 != 0 || status_2 != 0)
            return gassembly_status_ErrCalc;
    }

    return gassembly_status_OK;
}

gassembly_status gassembly_putOpening(FILE *out, const bool fixupRun, const size_t offset)
{
    gassembly_status status = gassembly_assembleFromLine("call main  \0", out, fixupRun, offset);
    if (status > 1) {
        fprintf(stderr, "Error occured during pre-assembling, error_code = %d (%s)\n>>%s\n", status, gassembly_statusMsg[status], "call main   ; This is an automaticly added opening line");
        return status;
    }
    
    status = gassembly_assembleFromLine("exit   \0", out, fixupRun, offset);
    if (status > 1) {
        fprintf(stderr, "Error occured during pre-assembling, error_code = %d (%s)\n>>%s\n", status, gassembly_statusMsg[status], "exit   ; This is an automaticly added opening line"); 
        return status;
    }

    return gassembly_status_OK;
}

gassembly_status gassembly_assembleFromFile(FILE *in, FILE *out) 
{
    char buffer[GASSEMBLY_MAX_LINE_SIZE] = {};
    
    gassembly_status status; 
    const long startPos = ftell(in);
    const long offset   = ftell(out);
    FILE *devNull = tmpfile();
    bool fixupRun = true;


    size_t line = 1;
    if (!getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in))
        return gassembly_status_ErrFile;

    status = gassembly_putOpening(devNull, fixupRun, offset);
    if (status != gassembly_status_OK)
        return status;

    while (!feof(in)) {
        status = gassembly_assembleFromLine(buffer, devNull, fixupRun, offset);
        if (status > 1) {
            fprintf(stderr, "Error occured during pre-assembling, error_code = %d (%s)\n>>%s\n", status, gassembly_statusMsg[status], buffer);      //TODO
            return status;
        }
        if (!getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in))
            return gassembly_status_ErrFile;
        ++line;
    }
    fclose(devNull);

    line = 1;
    fixupRun = false;
    fseek(in, startPos, SEEK_SET);
    if (!getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in))
        return gassembly_status_ErrFile;
        
    status = gassembly_putOpening(devNull, fixupRun, offset);
    if (status != gassembly_status_OK)
        return status;

    while (!feof(in)) {
        status = gassembly_assembleFromLine(buffer, out, fixupRun, offset);
        if (status > 1) {
            fprintf(stderr, "Error occured during assembling, error_code = %d (%s)\n>>%s\n", status, gassembly_statusMsg[status], buffer);      //TODO
            return status;
        }
        if (!getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in))
            return gassembly_status_ErrFile;
        ++line;
    }

    size_t i = 0;       //TODO DEBUG
    while (*gassembly_Lables[i] != '\0') {
        fprintf(stderr, "Lable = #%s#, Fixup = %d\n", gassembly_Lables[i], (SPU_INTEG_TYPE)gassembly_Fixups[i]); //DEBUG
        ++i;
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

bool gassembly_getLable(const char *buffer, char **beg, char **end)
{
    char *iter = (char*)buffer;
    while (isspace(*iter))
        ++iter;
    *beg = iter;
    if (!isalpha(*iter))
        return false;
    
    while (isalpha(*iter) || isdigit(*iter) || *iter == '_')
        ++iter;
    *end = iter;
    
    while (isspace(*iter))
        ++iter;
    
    if (*iter != ':')
        return false;

    ++iter;
    while (isspace(*iter))
        ++iter;
    if (*iter != '\0')
        return false;

    return true;
}


gassembly_status gassembly_assembleFromLine(const char *buffer, FILE *out, const bool fixupRun, const long offset) 
{
    /* 
     * WARNING: `buffer` must be a null terminated
     * string with len eq GASSEMBLY_MAX_LINE_SIZE 
     */

    if (!gPtrValid(buffer))
        return gassembly_status_BadOpPtr;
    if (!gPtrValid(out))
        return gassembly_status_ErrFile;

    gassembly_status status = gassembly_status_OK;
    
    /* cropping comments that start with `;` */
    char *commentPos = (char*)strchr(buffer, ';');
    if (commentPos != NULL)
        *commentPos = '\0';
    
    /* checking if line is only spaces */
    while (isspace(*buffer)) 
        ++buffer;

    if (*buffer == '\0')
        return gassembly_status_Empty;

    char *lableBeg = NULL;  
    char *lableEnd = NULL;
    if (gassembly_getLable(buffer, &lableBeg, &lableEnd)) {
        if (fixupRun) {
            size_t i = 0;
            while (i < GASSEMBLY_MAX_LABLES && *gassembly_Lables[i] != '\0')
                ++i;
            if (i == GASSEMBLY_MAX_LABLES) {
                fprintf(stderr, "ERROR: Fixup table is full!\n");
                return gassembly_status_ErrFixups;
            }
            strncpy(gassembly_Lables[i], lableBeg, lableEnd - lableBeg);
            assert(gassembly_Fixups[i] == 0);
            assert(ftell(out) >= offset);
            gassembly_Fixups[i] = ftell(out) - offset;
        }
        return gassembly_status_Empty;
    }


    char   keyword[GASSEMBLY_MAX_LINE_SIZE] = {};
    char operand_1[GASSEMBLY_MAX_LINE_SIZE] = {};
    char operand_2[GASSEMBLY_MAX_LINE_SIZE] = {};

    if (sscanf(buffer, "%s", keyword) != 1)
        return gassembly_status_ErrFile;


    char opcode = (gOpcodeByKeyword(keyword));
    if (opcode == -1) {
        fprintf(stderr, "Bad opcode provided\n");
        return gassembly_status_ErrOpcode;
    }
    if (fputc(opcode, out) == EOF)        //TODO implement dictionary for opcodes
        return gassembly_status_ErrFile;
    
    /* 
     * By now buffer shell not have opening spaces
     */
    char *operands = (char*)buffer;
    while(*operands != '\0' && !isspace(*operands) && *operands != '[')
        ++operands;

    char *delim = findFirstExternalOp(operands, ',');

    while (delim != NULL) {
        char operand[GASSEMBLY_MAX_LINE_SIZE] = {};
        assert(delim > operands);
        strncpy(operand, operands, delim - operands);
        status = gassembly_putOperand(operand, out);             //TODO handle returned error codes
        if (status > 1)
            return status;

        operands = delim + 1;
        delim = findFirstExternalOp(operands, ',');
    }
    status = gassembly_putOperand(operands, out);
    if (status > 1)
        return status;

    operandFormat emptyFormat = {};
    if (fwrite(&emptyFormat, sizeof(emptyFormat), 1, out) != 1)
        return gassembly_status_ErrFile;
    return gassembly_status_OK;
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
gassembly_status gassembly_getOperand(FILE *in, FILE *out) 
{
    gassembly_status status = gassembly_status_OK;
    operandFormat format;
    if (fread(&format, sizeof(format), 1, in) != 1) {
        fprintf(stderr, "failed to read format!\n");
        if (feof(in) || ferror(in))
            return gassembly_status_ErrFile;
        return gassembly_status_BadFormat;
    }


    if (!operandFormat_formatVerify(format)) {
        return gassembly_status_BadFormat;
    }

    if (operandFormat_isEmpty(format)) {
        return gassembly_status_Empty;
    }

    if (format.isMemCall) {
        fprintf(out, "[");
        status = gassembly_getOperand(in, out);
        if (status > 1)
            return status;
        fprintf(out, "]");
    } else if (format.calculation != gCalc_none) {
        fprintf(out, "(");
        status = gassembly_getOperand(in, out);
        if (status > 1)
            return status;

        fprintf(out, " %c ", DELIMS_LIST[format.calculation]);   
        
        status = gassembly_getOperand(in, out);
        if (status > 1)
            return status;
        fprintf(out, ")");
    } else if (format.isRegister) {
        char regCode = 0;
        regCode = fgetc(in);

        if (regCode == EOF)
            return gassembly_status_ErrFile;
        
        if (regCode < 1 || regCode >= MAX_REGISTERS)
            return gassembly_status_ErrReg;

        fprintf(out, "%cx", regCode + 'a' - 1);
    } else {
        SPU_FLOAT_TYPE val = 0;
        if (!fread(&val, sizeof(SPU_FLOAT_TYPE), 1, in)) {
            if (feof(in) || ferror(in))
                return gassembly_status_ErrFile;
            return gassembly_status_ErrLit;
        }

        fprintf(out, ELEM_PRINTF_FORM, val);
    }
    return gassembly_status_OK;
}

gassembly_status gassembly_disassembleFromFile(FILE *in, FILE *out) 
{
    char opcode;
    gassembly_status status = gassembly_status_OK;
    fread(&opcode, sizeof(char), 1, in);
    if (ferror(in))
        return gassembly_status_ErrFile;

    while (!feof(in)) {
        assert(opcode < gCnt);
        fprintf(out, "%s ", gDisassambleTable[opcode]);
        while ((status = gassembly_getOperand(in, out)) == 0) {
            if (fputc(' ', out) == EOF)
                return gassembly_status_ErrFile;
        }

        if (status != 1) {
            fprintf(stderr, "Error occured while reading operand, error_code = %d\n", status);
            return status;
        }
        
        fprintf(out, "\n");
        fread(&opcode, sizeof(char), 1, in);
        if (ferror(in))
            return gassembly_status_ErrFile;
    }
    return gassembly_status_OK;
}
