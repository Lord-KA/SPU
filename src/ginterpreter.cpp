#include "ginterpreter.h"

void ginterpreter_ctor(ginterpreter *interpreter)
{
    stack_ctor(&interpreter->Stack);

    interpreter->RAM = (SPU_FLOAT_TYPE*)calloc(MAX_RAM_SIZE, sizeof(SPU_FLOAT_TYPE));
}

void ginterpreter_dtor(ginterpreter *interpreter)
{
    stack_dtor(&interpreter->Stack);

    free(interpreter->RAM);
}


/**
 * `SPU_FLOAT_TYPE **valList` is a null-terminated list of opcode operands with length of `MAX_OPERANDS + 1`
 */

void ginterpreter_idle(ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    return;
}

void ginterpreter_push_1  (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    stack_push(&interpreter->Stack, **valList);
}

void ginterpreter_pop     (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    stack_pop(&interpreter->Stack, NULL);
}

void ginterpreter_pop_1   (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    stack_pop(&interpreter->Stack, *valList);
}

void ginterpreter_add     (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    SPU_FLOAT_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);

    stack_push(&interpreter->Stack, val_1 + val_2);
}

void ginterpreter_add_2   (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    **valList += **(valList + 1);
}

void ginterpreter_sub     (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    SPU_FLOAT_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);
 
    stack_push(&interpreter->Stack, val_1 - val_2);
}

void ginterpreter_sub_2  (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    **valList -= **(valList + 1);
}

void ginterpreter_mul    (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    SPU_FLOAT_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);

    stack_push(&interpreter->Stack, val_1 * val_2);
}

void ginterpreter_mul_2  (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    **valList *= **(valList + 1);
}

void ginterpreter_mov_2  (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    **valList = **(valList + 1);
}

void ginterpreter_out    (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    SPU_FLOAT_TYPE val;
    stack_pop(&interpreter->Stack, &val);
    printf("%d\n", val);
}

void ginterpreter_out_1  (ginterpreter *interpreter, SPU_FLOAT_TYPE **valList)
{
    printf("%d\n", **valList);
}

FILE *in_TMP = NULL;            //TODO DELETE IT!!!!

void ginterpreter_jmp_1  (ginterpreter *interpreter, SPU_FLOAT_TYPE**valList)
{
    SPU_INTEG_TYPE pos = **valList;
    fprintf(stderr, "pos = %lli\n", pos);
    fseek(in_TMP, pos, SEEK_SET);
}

static SPU_FLOAT_TYPE ret_val = {};
 
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
int ginterpreter_calcOperand(ginterpreter *interpreter, FILE *in, SPU_FLOAT_TYPE **valuePtr)
{
    operandFormat format = {};
    int status = 0;

    if (!fread(&format, sizeof(format), 1, in)){
        fprintf(stderr, "Failed to read format!\n");
        if (feof(in) || ferror(in))
            return 7;
        return 6;
    }

    if(!operandFormat_formatVerify(format))
        return 6;

    if (operandFormat_isEmpty(format)) {
        *valuePtr = NULL;
        return 1;
    }

    if (format.isMemCall) {
        fprintf(stderr, "Mem Call!\n");
        status = ginterpreter_calcOperand(interpreter, in, valuePtr); //TODO
        *valuePtr = interpreter->RAM + (SPU_INTEG_TYPE)**valuePtr;
    } else if (format.isRegister) {
        fprintf(stderr, "Register!\n");
        char regCode = 0;
        regCode = fgetc(in);
        if (regCode == EOF)
            return 7;
        
        if (regCode < 1 || regCode >= MAX_REGISTERS) 
            return 4;

        *valuePtr = interpreter->Registers + regCode;
    } else if (format.calculation != gCalc_none) {
        fprintf(stderr, "Calculation!\n");
        status = ginterpreter_calcOperand(interpreter, in, valuePtr);
        SPU_FLOAT_TYPE result = **valuePtr;
        status = ginterpreter_calcOperand(interpreter, in, valuePtr);


        if (format.calculation == gCalc_mul) 
            result *= **valuePtr;
        else if (format.calculation == gCalc_add)
            result += **valuePtr;
        else if (format.calculation == gCalc_sub)
            result -= **valuePtr;
        else {
            fprintf(stderr, "FATAL_ERROR: bad calculation option provided in bytecode\n");
            return 3;
        }

        ret_val = result;
        *valuePtr = &ret_val;
    } else {
        fprintf(stderr, "Literal!\n");
        if (!fread(&ret_val, sizeof(SPU_FLOAT_TYPE), 1, in)) {
            if (feof(in) || ferror(in))
                return 7;
            return 5;
        }
        *valuePtr = &ret_val;
        fprintf(stderr, "ret_val = %lli\n", ret_val);
    }
    return 0;
}


int ginterpreter_runFromFile(ginterpreter *interpreter, FILE *in)
{
    char opcode;
    in_TMP = in;
    fread(&opcode, sizeof(char), 1, in);        //TODO check fread for errors
    while (!feof(in)) {
        fprintf(stderr, "opcode = %d (%s)\n", opcode, gDisassambleTable[opcode]);
        size_t operandsCnt = 0;
        
        SPU_FLOAT_TYPE *Operands[MAX_OPERANDS + 1] = {};
        int status = 0;
        while ((status = ginterpreter_calcOperand(interpreter, in, &Operands[operandsCnt])) == 0) {
            if (Operands[operandsCnt] == NULL)
                return 6666;                            //TODO add some error code for the case
            ++operandsCnt;
        }
        if (status == 0)
            assert(!"WTF?!");
        if (status != 1)
            return status;

        (*((void (*)(ginterpreter *, SPU_FLOAT_TYPE **))interpreter->commandJumpTable[opcode][operandsCnt]))(interpreter, Operands);

        fread(&opcode, sizeof(char), 1, in);
    }

    return 0;
}


