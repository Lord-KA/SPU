#include "gassembly.h"
#include "gutils.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


static char *findFirstExternalOp(const char *buffer, const char operation)
{
    assert(gPtrValid(buffer));
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
    assert(gPtrValid(buffer));
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
    assert(gPtrValid(buffer));
    char *iter = (char*)buffer;
    while (isspace(*iter)) 
        ++iter;
    if (iter - buffer < strlen(buffer) + 1
            && (*iter >= 'a' && *iter - 'a' < GASSEMBLY_MAX_REGISTERS)
            && (*(iter + 1) == 'x')) {
        ++iter;
        while (isspace(*(++iter))) {}
        if (*iter == '\0')
            return true;
    }
    return false;
}

static bool isLable(const char *buffer) 
{
    assert(gPtrValid(buffer));
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

static bool isCalculation(const char *buffer)
{
    assert(gPtrValid(buffer));

    char *iter = (char*)buffer;
    while (isspace(*iter)) 
        ++iter;
 
    if(*iter != '(' || *iter == '\0')
        return false;
    ++iter;

    char *closeBrack = findFirstExternalOp(iter, ')');
    if (closeBrack == NULL)
        return false;

    while (isspace(*(++closeBrack))) {}
    
    if (*closeBrack == '\0')
        return true;

    return false;    
}

static gassembly_status gassembly_putOperand(const char *operand, FILE *out, const bool fixupRun, FILE *newLogStream) 
{
    FILE *logStream = stderr;
    if (gPtrValid(newLogStream))
        logStream = newLogStream;

    GASSEMBLY_ASSERT_LOG(gPtrValid(operand), gassembly_status_BadOperandPtr);
    GASSEMBLY_ASSERT_LOG(gPtrValid(out), gassembly_status_BadOutPtr);
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
    if (iter - operand < operandLen - 1)
        *(iter + 1) = '\0';

    if (operandLen == 0)
        return gassembly_status_Empty;

    int isOk = 0;
    operandFormat format = {};
    char calcOp = 0;
    char *brackPos = NULL;

    if (isInteger(operand)) {
        #ifdef EXTRA_VERBOSE
            fprintf(logStream, "Literal integer case; operand = #%s#\n", operand);
        #endif
        /* Literal integer case */                     
        format.isMemCall  = 0;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        GASSEMBLY_ASSERT_LOG(fwrite(&format, sizeof(format), 1, out) == 1, gassembly_status_ErrFile);

        SPU_INTEG_TYPE val = 0;          
        GASSEMBLY_ASSERT_LOG(sscanf(operand, "%lli%n", &val, &isOk) == 1, gassembly_status_ErrLit);

        GASSEMBLY_ASSERT_LOG(fwrite(&val, sizeof(SPU_INTEG_TYPE), 1, out) == 1, gassembly_status_ErrFile);

    } else if (isDouble(operand)) {
        #ifdef EXTRA_VERBOSE
            fprintf(logStream, "Literal double case; operand = #%s#\n", operand);
        #endif
        /* Literal double case */
        format.isMemCall  = 0;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        GASSEMBLY_ASSERT_LOG(fwrite(&format, sizeof(format), 1, out) == 1, gassembly_status_ErrFile);

        SPU_FLOAT_TYPE val = 0;          
        GASSEMBLY_ASSERT_LOG(sscanf(operand, "%lg%n", &val, &isOk) == 1, gassembly_status_ErrLit);

        GASSEMBLY_ASSERT_LOG(fwrite(&val, sizeof(SPU_INTEG_TYPE), 1, out) == 1, gassembly_status_ErrFile);
    } else if (isRegister(operand)) {
        #ifdef EXTRA_VERBOSE
            fprintf(logStream, "Register case; operand = #%s#\n", operand);
        #endif
        /* Register case */
        format.isMemCall  = 0;
        format.isRegister = 1;
        format.calculation = gCalc_none;
        GASSEMBLY_ASSERT_LOG(fwrite(&format, sizeof(format), 1, out) == 1, gassembly_status_ErrFile);

        char reg = 0;
        GASSEMBLY_ASSERT_LOG(sscanf(operand, "%cx%n", &reg, &isOk) == 1, gassembly_status_ErrReg);

        char regCode = reg - 'a' + 1;
        assert(regCode > 0);

        GASSEMBLY_ASSERT_LOG(fputc(regCode, out) != EOF, gassembly_status_ErrFile);

    } else if (isLable(operand)) {
        #ifdef EXTRA_VERBOSE
            fprintf(logStream, "Lable case; operand = #%s#\n", operand);
        #endif
        /* Lable case */
        format.isMemCall  = 0;
        format.isRegister = 0;
        format.calculation = gCalc_none;
        GASSEMBLY_ASSERT_LOG(fwrite(&format, sizeof(format), 1, out) == 1, gassembly_status_ErrFile);

        char lable[GASSEMBLY_MAX_LINE_SIZE] = {};

        GASSEMBLY_ASSERT_LOG(sscanf(operand, "%s", lable) == 1, gassembly_status_ErrLable);

        size_t i = 0;
        if (!fixupRun) {
    
            while (i < GASSEMBLY_MAX_LABLES && strcmp(gassembly_Lables[i], lable) != 0)
                ++i;
            GASSEMBLY_ASSERT_LOG(i != GASSEMBLY_MAX_LABLES, gassembly_status_ErrLable);      
        }
        GASSEMBLY_ASSERT_LOG(fwrite(&gassembly_Fixups[i], sizeof(SPU_INTEG_TYPE), 1, out) == 1, gassembly_status_ErrFile);

    } else if (isMemCall(operand)) {
        #ifdef EXTRA_VERBOSE
            fprintf(logStream, "Mem call case; operand = #%s#", operand);
        #endif
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
        GASSEMBLY_ASSERT_LOG(fwrite(&format, sizeof(format), 1, out) == 1, gassembly_status_ErrFile);
        
        #ifdef EXTRA_VERBOSE
            fprintf(logStream, " subOperand = #%s#\n", subOperand);
        #endif
        gassembly_putOperand(subOperand, out, fixupRun, logStream);
    } 
    else if (isCalculation(operand)) {
        #ifdef EXTRA_VERBOSE
            fprintf(logStream, "Brackets calculation case; operand = #%s#\n", operand);
        #endif
        /* Brackets calculation case */
        
        char subOperand[GASSEMBLY_MAX_LINE_SIZE] = {};
        strncpy(subOperand, operand + 1, operandLen - 2);

        gassembly_putOperand(subOperand, out, fixupRun, logStream);
    } else {
        #ifdef EXTRA_VERBOSE
            fprintf(logStream, "Calculation case; operand = #%s#", operand);
        #endif
        /* Resolving calculations case */
        char *addPos = findFirstExternalOp(operand, '+');
        char *subPos = findFirstExternalOp(operand, '-');
        char *opPos  = NULL;

        if (addPos == NULL && subPos == NULL) {
            /* Mult case */
            format.isMemCall  = 0;
            format.isRegister = 0;
            format.calculation = gCalc_mul;
            GASSEMBLY_ASSERT_LOG(fwrite(&format, sizeof(format), 1, out) == 1, gassembly_status_ErrFile);

            calcOp = '*';
            opPos = findFirstExternalOp(operand, '*');
        } else if (addPos != NULL && (subPos == NULL || addPos < subPos)) {
            /* Plus case */
            format.isMemCall  = 0;
            format.isRegister = 0;
            format.calculation = gCalc_add;
            GASSEMBLY_ASSERT_LOG(fwrite(&format, sizeof(format), 1, out) == 1, gassembly_status_ErrFile);
            
            opPos = addPos;
            calcOp = '+';
        } else {
            /* Minus case */
            format.isMemCall  = 0;
            format.isRegister = 0;
            format.calculation = gCalc_sub;
            GASSEMBLY_ASSERT_LOG(fwrite(&format, sizeof(format), 1, out) == 1, gassembly_status_ErrFile);
            
            opPos = subPos;
            calcOp = '-';
        }

        if (opPos == NULL) {
            fprintf(logStream, "ERROR: parsing error, no `+` or `-` or `*` in a calculation\n");
            return gassembly_status_ErrCalc;
        }
        assert(opPos > operand);
        
        char subOperand_1[GASSEMBLY_MAX_LINE_SIZE] = {};
        char subOperand_2[GASSEMBLY_MAX_LINE_SIZE] = {};

        strncpy(subOperand_1, operand, opPos - operand);
        ++opPos;
        strncpy(subOperand_2, opPos,  operandLen - (opPos - operand));

        #ifdef EXTRA_VERBOSE
            fprintf(logStream, " subOperand_1 = #%s# subOperand_2 = #%s#\n", subOperand_1, subOperand_2);
        #endif
        int status_1 = gassembly_putOperand(subOperand_1, out, fixupRun, logStream);
        int status_2 = gassembly_putOperand(subOperand_2, out, fixupRun, logStream);
        if (status_1 != 0 || status_2 != 0)
            return gassembly_status_ErrCalc;
    }

    return gassembly_status_OK;
}

static gassembly_status gassembly_putOpening(FILE *out, const bool fixupRun, const size_t offset, FILE *newLogStream)
{    
    FILE *logStream = stderr;
    if (gPtrValid(newLogStream))
        logStream = newLogStream;

    GASSEMBLY_ASSERT_LOG(gPtrValid(out), gassembly_status_BadOutPtr);
 
    char buffer_1[GASSEMBLY_MAX_LINE_SIZE] = "call main";
    gassembly_status status = gassembly_assembleFromLine("call main", out, fixupRun, offset, logStream);
    if (status > 1) {
        fprintf(logStream, "Error occured during pre-assembling, error_code = %d (%s)\n>>%s\n", status, gassembly_statusMsg[status], "call main   ; This is an automaticly added opening line");
        return status;
    }
    
    char buffer_2[GASSEMBLY_MAX_LINE_SIZE] = "exit";
    status = gassembly_assembleFromLine("exit", out, fixupRun, offset, logStream);
    if (status > 1) {
        fprintf(logStream, "Error occured during pre-assembling, error_code = %d (%s)\n>>%s\n", status, gassembly_statusMsg[status], "exit   ; This is an automaticly added opening line"); 
        return status;
    }

    return gassembly_status_OK;
}

gassembly_status gassembly_assembleFromFile(FILE *in, FILE *out, FILE *newLogStream) 
{  
    FILE *logStream = stderr;
    if (gPtrValid(newLogStream))
        logStream = newLogStream;

    GASSEMBLY_ASSERT_LOG(gPtrValid(in),  gassembly_status_BadInPtr);
    GASSEMBLY_ASSERT_LOG(gPtrValid(out), gassembly_status_BadOutPtr);
 
    char buffer[GASSEMBLY_MAX_LINE_SIZE] = {};
    
    gassembly_status status; 
    const long startPos = ftell(in);
    const long offset   = ftell(out);
    FILE *devNull = tmpfile(); 
    bool fixupRun = true;

    size_t line = 1;
    GASSEMBLY_ASSERT_LOG(getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in), gassembly_status_ErrFile);

    #ifdef EXTRA_VERBOSE
    fprintf(logStream, "line = #%s#\n", buffer);
    #endif
    status = gassembly_putOpening(devNull, fixupRun, offset, logStream);
    if (status != gassembly_status_OK)
        return status;

    while (!feof(in)) {
        status = gassembly_assembleFromLine(buffer, devNull, fixupRun, offset, logStream);
        if (status > 1) {
            fprintf(logStream, "Error occured during pre-assembling, error_code = %d (%s)\n>>%s\n", status, gassembly_statusMsg[status], buffer);
            return status;
        }
        GASSEMBLY_ASSERT_LOG(getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in), gassembly_status_ErrFile);
        ++line;
    }
    fclose(devNull);

    line = 1;
    fixupRun = false;
    fseek(in, startPos, SEEK_SET);
    GASSEMBLY_ASSERT_LOG(getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in), gassembly_status_ErrFile);
        
    status = gassembly_putOpening(out, fixupRun, offset, logStream);
    if (status != gassembly_status_OK)
        return status;

    while (!feof(in)) {
        status = gassembly_assembleFromLine(buffer, out, fixupRun, offset, logStream);
        if (status > 1) {
            fprintf(logStream, "Error occured during assembling, error_code = %d (%s)\n>>%s\n", status, gassembly_statusMsg[status], buffer);
            return status;
        }
        GASSEMBLY_ASSERT_LOG(getline(buffer, GASSEMBLY_MAX_LINE_SIZE, in), gassembly_status_ErrFile);
        ++line;
    }

    #ifdef EXTRA_VERBOSE
    for (size_t i = 0; *gassembly_Lables[i] != '\0'; ++i)
        fprintf(logStream, "Lable = #%s#, Fixup = %d\n", gassembly_Lables[i], (SPU_INTEG_TYPE)gassembly_Fixups[i]); //DEBUG
    #endif
    return gassembly_status_OK;
}

static int gOpcodeByKeyword(char *keyword)
{
    assert(gPtrValid(keyword));
    size_t i = 0;
    while (i < gCnt && strcmp(keyword, gDisassambleTable[i])) {
        ++i;
    }
    if (i == gCnt)
        return -1;
    return i;
}

static bool gassembly_getLable(const char *buffer, char **beg, char **end)
{
    assert(gPtrValid(buffer));
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

gassembly_status gassembly_assembleFromLine(const char *buffer, FILE *out, const bool fixupRun, const long offset, FILE *newLogStream) 
{
    FILE *logStream = stderr;
    if (gPtrValid(newLogStream))
        logStream = newLogStream;

    GASSEMBLY_ASSERT_LOG(gPtrValid(buffer), gassembly_status_BadOpPtr);
    GASSEMBLY_ASSERT_LOG(gPtrValid(out),    gassembly_status_BadOutPtr);
 
    /* 
     * WARNING: `buffer` must be a null terminated
     * string with len eq GASSEMBLY_MAX_LINE_SIZE 
     */

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
                fprintf(logStream, "ERROR: Fixup table is full!\n");
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

    GASSEMBLY_ASSERT_LOG(sscanf(buffer, "%s", keyword) == 1, gassembly_status_ErrFile);

    char opcode = (gOpcodeByKeyword(keyword));
    #ifdef EXTRA_VERBOSE
    fprintf(logStream, "opcode = %d (%s)\n", opcode, keyword);
    #endif
    if (opcode == -1) {
        fprintf(logStream, "Bad opcode provided\n");
        return gassembly_status_ErrOpcode;
    }
    GASSEMBLY_ASSERT_LOG(fputc(opcode, out) != EOF, gassembly_status_ErrFile);
    
    /* 
     * By now buffer shell not have opening spaces
     */
    char *operands = (char*)buffer;
    while(*operands != '\0' && !isspace(*operands) && *operands != '[')
        ++operands;

    while (isspace(*operands))
        ++operands;

    char *delim = NULL;
    char operand[GASSEMBLY_MAX_LINE_SIZE] = {};

    if (*operands == '\0')
        goto finish;

    delim = findFirstExternalOp(operands, ',');

    while (delim != NULL) {
        char operand[GASSEMBLY_MAX_LINE_SIZE] = {};
        assert(delim > operands);
        strncpy(operand, operands, delim - operands);
        status = gassembly_putOperand(operand, out, fixupRun, logStream);             //TODO handle returned error codes
        if (status > 1)
            return status;

        operands = delim + 1;
        delim = findFirstExternalOp(operands, ',');
    }
    strcpy(operand, operands);
    status = gassembly_putOperand(operand, out, fixupRun, logStream);
    if (status > 1)
        return status;

finish:
    operandFormat emptyFormat = {};
    GASSEMBLY_ASSERT_LOG(fwrite(&emptyFormat, sizeof(emptyFormat), 1, out) == 1, gassembly_status_ErrFile);
    return gassembly_status_OK;
}

static gassembly_status gassembly_getOperand(FILE *in, FILE *out, FILE *newLogStream) 
{
    FILE *logStream = stderr;
    if (gPtrValid(newLogStream))
        logStream = newLogStream;

    GASSEMBLY_ASSERT_LOG(gPtrValid(in),  gassembly_status_BadInPtr);
    GASSEMBLY_ASSERT_LOG(gPtrValid(out), gassembly_status_BadOutPtr);
 
 
    gassembly_status status = gassembly_status_OK;
    operandFormat format;
    if (fread(&format, sizeof(format), 1, in) != 1) {
        fprintf(logStream, "failed to read format!\n");
        GASSEMBLY_ASSERT_LOG(!feof(in) && !ferror(in), gassembly_status_ErrFile);
        return gassembly_status_BadFormat;
    }


    GASSEMBLY_ASSERT_LOG(operandFormat_formatVerify(format), gassembly_status_BadFormat);

    if (operandFormat_isEmpty(format)) {
        return gassembly_status_Empty;
    }

    if (format.isMemCall) {
        fprintf(out, "[");
        status = gassembly_getOperand(in, out, logStream);
        if (status > 1)
            return status;
        fprintf(out, "]");
    } else if (format.calculation != gCalc_none) {
        fprintf(out, "(");
        status = gassembly_getOperand(in, out, logStream);
        if (status > 1)
            return status;

        fprintf(out, " %c ", DELIMS_LIST[format.calculation]);   
        
        status = gassembly_getOperand(in, out, logStream);
        if (status > 1)
            return status;
        fprintf(out, ")");
    } else if (format.isRegister) {
        char regCode = 0;
        regCode = fgetc(in);

        GASSEMBLY_ASSERT_LOG(regCode != EOF, gassembly_status_ErrFile);
        
        GASSEMBLY_ASSERT_LOG(regCode >= 1 && regCode < GASSEMBLY_MAX_REGISTERS, gassembly_status_ErrReg);

        fprintf(out, "%cx", regCode + 'a' - 1);
    } else {
        SPU_FLOAT_TYPE val = 0;
        if (!fread(&val, sizeof(SPU_FLOAT_TYPE), 1, in)) {
            GASSEMBLY_ASSERT_LOG(!feof(in) && !ferror(in), gassembly_status_ErrFile);
            return gassembly_status_ErrLit;
        }

        fprintf(out, ELEM_PRINTF_FORM, val);
    }
    return gassembly_status_OK;
}

gassembly_status gassembly_disassembleFromFile(FILE *in, FILE *out, FILE *newLogStream) 
{   
    FILE *logStream = stderr;
    if (gPtrValid(newLogStream))
        logStream = newLogStream;

    GASSEMBLY_ASSERT_LOG(gPtrValid(in),  gassembly_status_BadInPtr);
    GASSEMBLY_ASSERT_LOG(gPtrValid(out), gassembly_status_BadOutPtr);
 
    char opcode;
    gassembly_status status = gassembly_status_OK;
    fread(&opcode, sizeof(char), 1, in);
    if (ferror(in))
        return gassembly_status_ErrFile;

    while (!feof(in)) {
        assert(opcode < gCnt);
        fprintf(out, "%s ", gDisassambleTable[opcode]);
        while ((status = gassembly_getOperand(in, out, logStream)) == 0) {
            GASSEMBLY_ASSERT_LOG(fputc(' ', out) != EOF, gassembly_status_ErrFile);
        }

        if (status != 1) {
            fprintf(stderr, "Error occured while reading operand, error_code = %d (%s)\n", status, gassembly_statusMsg[status]);
            return status;
        }
        
        fprintf(out, "\n");
        fread(&opcode, sizeof(char), 1, in);
        GASSEMBLY_ASSERT_LOG(!ferror(in), gassembly_status_ErrFile);
    }
    return gassembly_status_OK;
}

