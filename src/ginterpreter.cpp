#include "ginterpreter.h"

ginterpreter_status ginterpreter_ctor(ginterpreter *context, FILE *out)
{
    if (ptrValid(out))
        context->outStream = out;
    else
        context->outStream = stdout;

    stack_ctor(&context->Stack);

    context->videoHeight = GASSEMBLY_DEFAULT_WINDOW_HEIGHT;
    context->videoWidth  = GASSEMBLY_DEFAULT_WINDOW_WIDTH;

    context->videoRAM = (char*)calloc(context->videoWidth * context->videoHeight, sizeof(char));

    context->RAM = (SPU_FLOAT_TYPE*)calloc(GASSEMBLY_MAX_RAM_SIZE, sizeof(SPU_FLOAT_TYPE));

    if (context->RAM == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate RAM memory!\n");
        return ginterpreter_status_AllocErr;
    }
    /* Filling commandJumpTable with fuction pointers defined in commands.tpl */
    #define COMMAND(name, Name, isFirst, argc, code) context->commandJumpTable[g##Name][argc] = (OpcodeFunctionPtr)&ginterpreter_##name##_##argc;
    #include "commands.tpl"
    #undef COMMAND

    context->exited = false;

    return ginterpreter_status_OK;
}

ginterpreter_status ginterpreter_dtor(ginterpreter *context)
{

    stack_dtor(&context->Stack);

    for (size_t i = 0; i < context->videoHeight * context->videoWidth; ++i)
        fprintf(stdout, "%d", context->videoRAM[i]);
    fputc('\n', stdout);
    free(context->videoRAM);
    free(context->RAM);
    free(context->Buffer);
    return ginterpreter_status_OK;
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

    format = *(operandFormat*)(context->bufCur);
    context->bufCur += sizeof(format);

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
        regCode = *context->bufCur;
        ++context->bufCur;
        
        if (regCode < 1 || regCode >= GASSEMBLY_MAX_REGISTERS) 
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
        
        stack_push(&context->calcOp_stack, result);
        stack_top(&context->calcOp_stack, valuePtr);
    } else {
        fprintf(stderr, "Literal!\n");

        stack_push(&context->calcOp_stack, *(SPU_FLOAT_TYPE*)(context->bufCur));
        context->bufCur += sizeof(SPU_FLOAT_TYPE);
        stack_top(&context->calcOp_stack, valuePtr);
        fprintf(stderr, "ret_val = %lli\n", **valuePtr);
    }
    return ginterpreter_status_OK;
}


ginterpreter_status ginterpreter_runFromFile(ginterpreter *context, FILE *in)
{
    char opcode;

    fseek(in, 0L, SEEK_END);
    context->buflen = ftell(in);
    fseek(in, 0L, SEEK_SET);
    context->Buffer = (char*)calloc(context->buflen, sizeof(char));

    fread(context->Buffer, sizeof(char), context->buflen, in);

    if (ferror(in))
        return ginterpreter_status_FileErr;

    return ginterpreter_runFromBuffer(context);
}

ginterpreter_status ginterpreter_runFromBuffer(ginterpreter *context)
{
    char opcode;

    context->bufCur = context->Buffer;

    stack_ctor(&context->calcOp_stack);
  
    while (context->bufCur < context->Buffer + context->buflen && !context->exited) {
        opcode = *context->bufCur;
        ++context->bufCur;

        fprintf(stderr, "opcode = %d (%s)\n", opcode, gDisassambleTable[opcode]);
        size_t operandsCnt = 0;
        
        SPU_FLOAT_TYPE *Operands[GASSEMBLY_MAX_OPERANDS + 1] = {};
        ginterpreter_status status = ginterpreter_status_OK;
        while ((status = ginterpreter_calcOperand(context, &Operands[operandsCnt])) == ginterpreter_status_OK) {
            if (Operands[operandsCnt] == NULL) {
                stack_dtor(&context->calcOp_stack);
                return ginterpreter_status_BadOperand;                            
            }
            ++operandsCnt;
        }

        assert(status != ginterpreter_status_OK && "This should never happen, because all operand sequences end with empty format");

        if (status != ginterpreter_status_EmptyFormat) {
            stack_dtor(&context->calcOp_stack);
            return status;
        }

        (*((void (*)(ginterpreter *, SPU_FLOAT_TYPE **))context->commandJumpTable[opcode][operandsCnt]))(context, Operands);
    }
    stack_dtor(&context->calcOp_stack);
    return ginterpreter_status_OK;
}

ginterpreter_status ginterpreter_setPseudographicsWindow(ginterpreter *context, size_t height, size_t width) 
{
    context->videoHeight = height;
    context->videoWidth  = width;

    free(context->videoRAM);

    context->videoRAM = (char*)calloc(context->videoWidth * context->videoHeight, sizeof(char));

    if (context->videoRAM == NULL) {
        context->videoHeight = -1;
        context->videoWidth  = -1;
        return ginterpreter_status_AllocErr;
    }

    return ginterpreter_status_OK;
}
