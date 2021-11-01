#include "ginterpreter.h"

void ginterpreter_ctor(ginterpreter *context)
{
    stack_ctor(&context->Stack);

    context->RAM = (SPU_FLOAT_TYPE*)calloc(MAX_RAM_SIZE, sizeof(SPU_FLOAT_TYPE));
    if (context->RAM == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate RAM memory!\n");
    }
    /* Filling commandJumpTable with fuction pointers defined in commands.tpl */
    #define COMMAND(name, Name, isFirst, argc, code) context->commandJumpTable[g##Name][argc] = (OpcodeFunctionPtr)&ginterpreter_##name##_##argc;
    #include "commands.tpl"
    #undef COMMAND
}

void ginterpreter_dtor(ginterpreter *context)
{
    stack_dtor(&context->Stack);

    free(context->RAM);
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
ginterpreter_status ginterpreter_calcOperand(ginterpreter *context, SPU_FLOAT_TYPE **valuePtr)  //TODO add logs when error occures
{
    operandFormat format = {};

    ginterpreter_status status = ginterpreter_status_OK;

    if (fread(&format, sizeof(format), 1, context->inStream) != 1){
        fprintf(stderr, "Failed to read format!\n");
        if (feof(context->inStream) || ferror(context->inStream))
            return ginterpreter_status_FileErr;
        return ginterpreter_status_BadFormat;
    }

    if(!operandFormat_formatVerify(format))
        return ginterpreter_status_BadFormat;

    if (operandFormat_isEmpty(format)) {
        *valuePtr = NULL;
        return ginterpreter_status_EmptyFormat;
    }

    if (format.isMemCall) {
        fprintf(stderr, "Mem Call!\n");
        if(ginterpreter_calcOperand(context, valuePtr) != 0)
            return ginterpreter_status_BadMemCall;
        *valuePtr = context->RAM + (SPU_INTEG_TYPE)**valuePtr;
    } else if (format.isRegister) {
        fprintf(stderr, "Register!\n");
        char regCode = 0;
        regCode = fgetc(context->inStream);
        if (regCode == EOF)
            return ginterpreter_status_FileErr;
        
        if (regCode < 1 || regCode >= MAX_REGISTERS) 
            return ginterpreter_status_BadReg;

        *valuePtr = context->Registers + regCode;
    } else if (format.calculation != gCalc_none) {
        fprintf(stderr, "Calculation!\n");
        if(ginterpreter_calcOperand(context, valuePtr) != 0)
            return ginterpreter_status_BadCalc;

        SPU_FLOAT_TYPE result = **valuePtr;

        if(ginterpreter_calcOperand(context, valuePtr) != 0)
            return ginterpreter_status_BadCalc;

        if (format.calculation == gCalc_mul) 
            result *= **valuePtr;
        else if (format.calculation == gCalc_add)
            result += **valuePtr;
        else if (format.calculation == gCalc_sub)
            result -= **valuePtr;
        else {
            fprintf(stderr, "FATAL_ERROR: bad calculation option provided in bytecode\n");
            return ginterpreter_status_BadCalc;
        }

        context->calcOp_ret = result;
        *valuePtr = &context->calcOp_ret;
    } else {
        fprintf(stderr, "Literal!\n");
        if (fread(&context->calcOp_ret, sizeof(SPU_FLOAT_TYPE), 1, context->inStream) != 1) {
            if (feof(context->inStream) || ferror(context->inStream))
                return ginterpreter_status_FileErr;
            return ginterpreter_status_BadLit;
        }
        *valuePtr = &context->calcOp_ret;
        fprintf(stderr, "ret_val = %lli\n", context->calcOp_ret);
    }
    return ginterpreter_status_OK;
}


ginterpreter_status ginterpreter_runFromFile(ginterpreter *context, FILE *in)
{
    char opcode;
    context->inStream = in;
    fread(&opcode, sizeof(char), 1, context->inStream);
    if (ferror(in))
        return ginterpreter_status_FileErr;

    while (!feof(in)) {
        fprintf(stderr, "opcode = %d (%s)\n", opcode, gDisassambleTable[opcode]);
        size_t operandsCnt = 0;
        
        SPU_FLOAT_TYPE *Operands[MAX_OPERANDS + 1] = {};
        ginterpreter_status status = ginterpreter_status_OK;
        while ((status = ginterpreter_calcOperand(context, &Operands[operandsCnt])) == ginterpreter_status_OK) {
            if (Operands[operandsCnt] == NULL)
                return ginterpreter_status_BadOperand;                            
            ++operandsCnt;
        }

        assert(status != ginterpreter_status_OK && "This should never happen, because all operand sequences end with empty format");

        if (status != ginterpreter_status_EmptyFormat)
            return status;

        (*((void (*)(ginterpreter *, SPU_FLOAT_TYPE **))context->commandJumpTable[opcode][operandsCnt]))(context, Operands);

        fread(&opcode, sizeof(char), 1, context->inStream);
        if (ferror(in))
            return ginterpreter_status_FileErr;
    }
    return ginterpreter_status_OK;
}
