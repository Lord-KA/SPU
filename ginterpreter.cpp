#include "ginterpreter.h"

void ginterpreter_ctor(ginterpreter *interpreter)
{
    stack_ctor(&interpreter->Stack);

    interpreter->RAM = (SPU_VAL_TYPE*)calloc(MAX_RAM_SIZE, sizeof(SPU_VAL_TYPE));
}

void ginterpreter_dtor(ginterpreter *interpreter)
{
    stack_dtor(&interpreter->Stack);

    free(interpreter->RAM);
}

void ginterpreter_idle(ginterpreter *interpreter)
{
    return;
}

void ginterpreter_push_one(ginterpreter *interpreter, SPU_VAL_TYPE *value)
{
    fprintf(stderr, "PUSH %d\n", *value);
    stack_push(&interpreter->Stack, *value);
}

void ginterpreter_pop(ginterpreter *interpreter)
{
    stack_pop(&interpreter->Stack, NULL);
}

void ginterpreter_pop_one(ginterpreter *interpreter, SPU_VAL_TYPE *value)
{
    stack_pop(&interpreter->Stack, value);
}

void ginterpreter_add(ginterpreter *interpreter)
{
    SPU_VAL_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);

    stack_push(&interpreter->Stack, val_1 + val_2);
}

void ginterpreter_add_two(ginterpreter *interpreter, SPU_VAL_TYPE *val_1, SPU_VAL_TYPE *val_2)
{
    *val_1 += *val_2;
}

void ginterpreter_sub(ginterpreter *interpreter)
{
    SPU_VAL_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);
 
    stack_push(&interpreter->Stack, val_1 - val_2);
}

void ginterpreter_sub_two(ginterpreter *interpreter, SPU_VAL_TYPE *val_1, SPU_VAL_TYPE *val_2)
{
    *val_1 -= *val_2;
}

void ginterpreter_mul(ginterpreter *interpreter)
{
    SPU_VAL_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);

    stack_push(&interpreter->Stack, val_1 * val_2);
}

void ginterpreter_mul_two(ginterpreter *interpreter, SPU_VAL_TYPE *val_1, SPU_VAL_TYPE *val_2)
{
    *val_1 *= *val_2;
}

void ginterpreter_mov_two(ginterpreter *interpreter, SPU_VAL_TYPE *val_1, SPU_VAL_TYPE *val_2)
{
    *val_1 = *val_2;
}

void ginterpreter_out(ginterpreter *interpreter)
{
    SPU_VAL_TYPE val;
    stack_pop(&interpreter->Stack, &val);
    printf("%d", val);
}

void ginterpreter_out_one(ginterpreter *interpreter, SPU_VAL_TYPE *value)
{
    printf("%d", *value);
}


static SPU_VAL_TYPE ret_val = {};
 
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
int ginterpreter_calcOperand(ginterpreter *interpreter, FILE *in, SPU_VAL_TYPE **valuePtr)
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
        status = ginterpreter_calcOperand(interpreter, in, valuePtr); //TODO
        *valuePtr = interpreter->RAM + **valuePtr;
    } else if (format.isRegister) {
        char regCode = 0;
        regCode = fgetc(in);
        if (regCode == EOF)
            return 7;
        
        if (regCode < 1 || regCode >= MAX_REGISTERS) 
        *valuePtr = interpreter->Registers + regCode;
    } else if (format.calculation != gCalc_none) {
        status = ginterpreter_calcOperand(interpreter, in, valuePtr); //TODO
        ret_val = **valuePtr;
        status = ginterpreter_calcOperand(interpreter, in, valuePtr); //TODO

        if (format.calculation == gCalc_mul) 
            ret_val *= **valuePtr;
        else if (format.calculation == gCalc_add)
            ret_val += **valuePtr;
        else if (format.calculation == gCalc_sub)
            ret_val -= **valuePtr;
        *valuePtr = &ret_val;
    } else {
        if (!fread(&ret_val, sizeof(SPU_VAL_TYPE), 1, in)) {
            if (feof(in) || ferror(in))
                return 7;
            return 5;
        }
        *valuePtr = &ret_val;
    }
    return 0;
}

void ginterpreter_runFromFile(ginterpreter *interpreter, FILE *in)
{
    char opcode;
    fread(&opcode, sizeof(char), 1, in);        //TODO check fread for errors
    while (!feof(in)) {
        fprintf(stderr, "opcode = %d (%s)\n", opcode, gDisassambleTable[opcode]);
        size_t operandsCnt = 0;
        
        SPU_VAL_TYPE *operand_1 = NULL;
        SPU_VAL_TYPE *operand_2 = NULL;
        
        ginterpreter_calcOperand(interpreter, in, &operand_1);    //TODO check for error codes
        ginterpreter_calcOperand(interpreter, in, &operand_2);
    
        if (operand_1)
            fprintf(stderr, "operand_1 = %d\n", *operand_1);
        if (operand_2)
            fprintf(stderr, "operand_2 = %d\n", *operand_2);

        (*(interpreter->commandJumpTable[opcode][1]))(operand_1);
        return;
        
        if (operand_1 != NULL && operand_2 != NULL) 
            (*((void (*)(SPU_VAL_TYPE *, SPU_VAL_TYPE *))interpreter->commandJumpTable[opcode][2]))(operand_1, operand_2);
        else if (operand_1 != NULL && operand_2 == NULL)
            (*((void (*)(SPU_VAL_TYPE *))interpreter->commandJumpTable[opcode][1]))(operand_1);
        else if (operand_1 == NULL && operand_2 == NULL)
            (*((void (*)())interpreter->commandJumpTable[opcode][0]))();
        else
            assert(!"ERROR: bad operand encoded, operand_1 == NULL while operand_2 != NULL");
        
        fread(&opcode, sizeof(char), 1, in);
    }
}


