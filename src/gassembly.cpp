#include "gassembly.h"
#include "gutils.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

//TODO check `fwrite` and `fputc` outp

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
 *              4 = Parsing error in Lable        case
 */
int gassembly_putOperand(const char *operand, FILE *out) 
{
    /*
     * WARNING: `operand` should be a null-terminated string
     */

    /* cropping opening spaces */
    while (isspace(*operand))
        ++operand;

    // if (!gPtrValid(operand))
    //     return 0;
    // assert(!gPtrValid(out));

    size_t operandLen = strlen(operand);

    /* cropping closing spaces */
    char *iter = (char*)(operand + operandLen - 1);
    while (isspace(*iter)) {
        --iter;
        --operandLen;
    }
    *(iter + 1) = '\0';

    int isOk = 0;
    operandFormat format = {};
    char calcOp = 0;
    char *brackPos = NULL;
    
    if (operandLen == 0)
        return 1;

    if (isInteger(operand)) {
        /* Literal integer case */                     
        format.isMemCall  = 0;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        fwrite(&format, sizeof(format), 1, out);
        //fprintf(out, "|LIT_FORM|");

        SPU_INTEG_TYPE val = 0;          
        sscanf(operand, "%lli%n", &val, &isOk);
        assert(sizeof(SPU_INTEG_TYPE) == sizeof(SPU_FLOAT_TYPE) && "SPU_INTEG_TYPE type must be corrected to be compatible with SPU_FLOAT_TYPE");
        assert(isOk != 0 && "ERROR: bad parsing of literal operand");
        fwrite(&val, sizeof(SPU_INTEG_TYPE), 1, out);
        // fprintf(out, "|val = %d|\n", val);
    } else if (isDouble(operand)) {
        /* Literal double case */
        format.isMemCall  = 0;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        fwrite(&format, sizeof(format), 1, out);
        //fprintf(out, "|LIT_FORM|");

        SPU_FLOAT_TYPE val = 0;          
        sscanf(operand, "%lg%n", &val, &isOk);
        assert(isOk != 0 && "ERROR: bad parsing of literal operand");
        fwrite(&val, sizeof(SPU_FLOAT_TYPE), 1, out);
        // fprintf(out, "|val = %d|\n", val);
    } else if (isRegister(operand)) {
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
        //assert(regCode > 0);
        fputc(regCode, out);
        fprintf(stderr, "|reg = %d|\n", regCode);
    } else if (isLable(operand)) {
        /* Lable case */
        format.isMemCall  = 0;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        fwrite(&format, sizeof(format), 1, out);

        char lable[GASSEMBLY_MAX_LINE_SIZE] = {};
        sscanf(operand, "%s", lable);
        size_t i = 0;
        while (i < GASSEMBLY_MAX_LABLES && strcmp(gassembly_Lables[i], lable) != 0)
            ++i;
        if (i == GASSEMBLY_MAX_LABLES)
            --i;
        fwrite(&gassembly_Fixups[i], sizeof(SPU_INTEG_TYPE), 1, out);

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
            fwrite(&format, sizeof(format), 1, out);
            //fprintf(out, "|MUL_FORM|\n");

            calcOp = '*';
            opPos = findFirstExternalOp(operand, '*');
        } else if (addPos != NULL && (subPos == NULL || addPos < subPos)) {
            /* Plus case */
            format.isMemCall  = 0;
            format.isRegister = 0;
            format.calculation = gCalc_add;
            fwrite(&format, sizeof(format), 1, out);
            //fprintf(out, "|SUM_FORM|\n");
            
            opPos = addPos;
            calcOp = '+';
        } else {
            /* Minus case */
            format.isMemCall  = 0;
            format.isRegister = 0;
            format.calculation = gCalc_sub;
            fwrite(&format, sizeof(format), 1, out);
            
            opPos = subPos;
            calcOp = '-';
        }

        if (opPos == NULL) {
            fprintf(stderr, "ERROR: parsing error, no `+` or `-` or `*` in a calculation\n");
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
    
    long startPos = ftell(in);
    FILE *devNull = fopen("tmp.tmp", "wb");
    bool fixupRun = true;

    getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in);
    while (!feof(in)) {
        gassembly_assembleFromLine(buffer, devNull, fixupRun);
        getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in);
    }

    fixupRun = false;
    fseek(in, startPos, SEEK_SET);
    getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in);
    while (!feof(in)) {
        gassembly_assembleFromLine(buffer, out, fixupRun);
        getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in);
    }

    size_t i = 0;       //DEBUG
    while (*gassembly_Lables[i] != '\0') {
        fprintf(stderr, "Lable = #%s#, Fixup = %d\n", gassembly_Lables[i], (SPU_INTEG_TYPE)gassembly_Fixups[i]);
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


void gassembly_assembleFromLine(const char *buffer, FILE *out, const bool fixupRun) 
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


    char *lableBeg = NULL;  
    char *lableEnd = NULL;
    if (gassembly_getLable(buffer, &lableBeg, &lableEnd)) {
        if (fixupRun) {
            size_t i = 0;
            while (i < GASSEMBLY_MAX_LABLES && *gassembly_Lables[i] != '\0')
                ++i;
            if (i == GASSEMBLY_MAX_LABLES) {
                fprintf(stderr, "ERROR: Fixup table is full!\n");
                return;
            }
            strncpy(gassembly_Lables[i], lableBeg, lableEnd - lableBeg);
            assert(gassembly_Fixups[i] == 0);
            gassembly_Fixups[i] = ftell(out);
        }
            return;
    }


    char   keyword[GASSEMBLY_MAX_LINE_SIZE] = {};
    char operand_1[GASSEMBLY_MAX_LINE_SIZE] = {};
    char operand_2[GASSEMBLY_MAX_LINE_SIZE] = {};
    sscanf(buffer, "%s", keyword);
    fputc(gOpcodeByKeyword(keyword), out);        //TODO implement dictionary for opcodes
    
    /* 
     * By now buffer shell not have opening spaces
     */
    char *operands = (char*)buffer;
    while(*operands != '\0' && !isspace(*operands) && *operands != '[')
        ++operands;

    // fprintf(stderr, "Before! operand_1 = #%s#\noperand_2 = #%s#\n", operand_1, operand_2);
    // fprintf(stderr, "Operands = #%s#\n", operands);
    char *delim = findFirstExternalOp(operands, ',');

    while (delim != NULL) {
        char operand[GASSEMBLY_MAX_LINE_SIZE] = {};
        assert(delim > operands);
        strncpy(operand, operands, delim - operands);
        gassembly_putOperand(operand, out);             //TODO handle returned error codes
        operands = delim + 1;
        delim = findFirstExternalOp(operands, ',');
    }
    gassembly_putOperand(operands, out);
    

    operandFormat emptyFormat = {};
    fwrite(&emptyFormat, sizeof(emptyFormat), 1, out);

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


    if (!operandFormat_formatVerify(format)) {
        // gassembly_formatDump(format, logOut); //TODO optional log errors
        return 6;
    }

    if (operandFormat_isEmpty(format)) {
        return 1;
    }
    if (format.isMemCall) {
        fprintf(out, "[");
        if (gassembly_getOperand(in, out) != 0)
            return 2;
        fprintf(out, "]");
    } else if (format.calculation != gCalc_none) {
        fprintf(out, "(");
        if (gassembly_getOperand(in, out) != 0)
            return 3;

        fprintf(out, " %c ", DELIMS_LIST[format.calculation]);   
        
        if (gassembly_getOperand(in, out) != 0)
            return 3;
        fprintf(out, ")");
    } else if (format.isRegister) {
        char regCode = 0;
        regCode = fgetc(in);

        if (regCode == EOF)
            return 7;
        
        if (regCode < 1 || regCode >= MAX_REGISTERS)
            return 4;

        fprintf(out, "%cx", regCode + 'a' - 1);
    } else {
        SPU_FLOAT_TYPE val = 0;
        if (!fread(&val, sizeof(SPU_FLOAT_TYPE), 1, in)) {
            if (feof(in) || ferror(in))
                return 7;
            return 5;
        }

        fprintf(out, "%lf", val);
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
