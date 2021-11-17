#include "ginterpreter.h"

ginterpreter_status ginterpreter_ctor(ginterpreter *context, FILE *newOutStream, FILE *newLogStream)
{
    if (!gPtrValid(context)) {                                          
        FILE *out;                                                   
        if (!gPtrValid(newLogStream))                                
            out = stderr;                                            
        else                                                         
            out = newLogStream;                                      
        fprintf(out, "ERROR: bad structure ptr provided to interpreter ctor!\n");
        return ginterpreter_status_BadSelfPtr;                         
    }

    if (ptrValid(newOutStream))
        context->outStream = newOutStream;
    else
        context->outStream = stdout;
   
    if (ptrValid(newLogStream))
        context->logStream = newLogStream;
    else
        context->logStream = stderr;

    GENERIC(stack_ctor)(&context->Stack);

    context->videoHeight = GASSEMBLY_DEFAULT_WINDOW_HEIGHT;
    context->videoWidth  = GASSEMBLY_DEFAULT_WINDOW_WIDTH;

    context->videoRAM = (char*)calloc(context->videoWidth * context->videoHeight, sizeof(char));

    context->RAM = (SPU_FLOAT_TYPE*)calloc(GASSEMBLY_MAX_RAM_SIZE, sizeof(SPU_FLOAT_TYPE));

    GINTERPRETER_ASSERT_LOG(context->RAM != NULL, ginterpreter_status_AllocErr);
   
    /* Filling commandJumpTable with fuction pointers defined in commands.tpl */
    #define COMMAND(name, Name, isFirst, argc, code) context->commandJumpTable[g##Name][argc] = (OpcodeFunctionPtr)&ginterpreter_##name##_##argc;
    #include "commands.tpl"
    #undef COMMAND

    context->exited = false;

    return ginterpreter_status_OK;
}

ginterpreter_status ginterpreter_dtor(ginterpreter *context)
{
    SELF_PTR_CHECK(context);

    GENERIC(stack_dtor)(&context->Stack);

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
    SELF_PTR_CHECK(context);

    operandFormat format = {};

    ginterpreter_status status = ginterpreter_status_OK;

    format = *(operandFormat*)(context->bufCur);
    context->bufCur += sizeof(format);

    GINTERPRETER_ASSERT_LOG(operandFormat_formatVerify(format), ginterpreter_status_BadFormat);

    if (operandFormat_isEmpty(format)) {
        *valuePtr = NULL;
        return ginterpreter_status_EmptyFormat;
    }

    if (format.isMemCall) {
        #ifdef EXTRA_VERBOSE
            fprintf(context->logStream, "Mem Call!\n");
        #endif
        GINTERPRETER_ASSERT_LOG(ginterpreter_calcOperand(context, valuePtr) == 0, ginterpreter_status_BadMemCall);
        *valuePtr = context->RAM + (SPU_INTEG_TYPE)**valuePtr;
    } else if (format.isRegister) {
        #ifdef EXTRA_VERBOSE
            fprintf(context->logStream, "Register!\n");
        #endif
        char regCode = 0;
        regCode = *context->bufCur;
        ++context->bufCur;
        
        GINTERPRETER_ASSERT_LOG(regCode >= 1 && regCode < GASSEMBLY_MAX_REGISTERS, ginterpreter_status_BadReg); 

        *valuePtr = context->Registers + regCode;
    } else if (format.calculation != gCalc_none) {
        #ifdef EXTRA_VERBOSE
            fprintf(context->logStream, "Calculation!\n");
        #endif
        GINTERPRETER_ASSERT_LOG(ginterpreter_calcOperand(context, valuePtr) == 0, ginterpreter_status_BadCalc);

        SPU_FLOAT_TYPE result = **valuePtr;

        GINTERPRETER_ASSERT_LOG(ginterpreter_calcOperand(context, valuePtr) == 0, ginterpreter_status_BadCalc);

        if (format.calculation == gCalc_mul) 
            result *= **valuePtr;
        else if (format.calculation == gCalc_add)
            result += **valuePtr;
        else if (format.calculation == gCalc_sub)
            result -= **valuePtr;
        else {
            fprintf(context->logStream, "FATAL_ERROR: bad calculation option provided in bytecode\n");
            return ginterpreter_status_BadCalc;
        }
        
        GENERIC(stack_push)(&context->calcOp_stack, result);
        GENERIC(stack_top)(&context->calcOp_stack, valuePtr);
    } else {
        #ifdef EXTRA_VERBOSE
            fprintf(context->logStream, "Literal!\n");
        #endif
        GENERIC(stack_push)(&context->calcOp_stack, *(SPU_FLOAT_TYPE*)(context->bufCur));
        context->bufCur += sizeof(SPU_FLOAT_TYPE);
        GENERIC(stack_top)(&context->calcOp_stack, valuePtr);
    }
    #ifdef EXTRA_VERBOSE
    fprintf(context->logStream, "ret_val = %lli\n", **valuePtr);
    #endif
    return ginterpreter_status_OK;
}


ginterpreter_status ginterpreter_runFromFile(ginterpreter *context, FILE *in)
{
    SELF_PTR_CHECK(context);
 
    fseek(in, 0L, SEEK_END);
    context->buflen = ftell(in);
    fseek(in, 0L, SEEK_SET);
    context->Buffer = (char*)calloc(context->buflen, sizeof(char));

    fread(context->Buffer, sizeof(char), context->buflen, in);

    GINTERPRETER_ASSERT_LOG(!ferror(in), ginterpreter_status_FileErr);

    return ginterpreter_runFromBuffer(context);
}

ginterpreter_status ginterpreter_runFromBuffer(ginterpreter *context)
{
    SELF_PTR_CHECK(context);
    ginterpreter_status status = ginterpreter_status_OK;
    char opcode = 0;

    context->bufCur = context->Buffer;

  
    while (context->bufCur < context->Buffer + context->buflen && !context->exited) {
        opcode = *context->bufCur;
        ++context->bufCur;
        GENERIC(stack_ctor)(&context->calcOp_stack);

        #ifdef EXTRA_VERBOSE
        fprintf(context->logStream, "opcode = %d (%s)\n", opcode, gDisassambleTable[opcode]);      
        #endif  

        size_t operandsCnt = 0;
        
        SPU_FLOAT_TYPE *Operands[GASSEMBLY_MAX_OPERANDS + 1] = {};
        while ((status = ginterpreter_calcOperand(context, &Operands[operandsCnt])) == ginterpreter_status_OK) {
            if (Operands[operandsCnt] == NULL) {
                status = ginterpreter_status_BadOperand;
                GENERIC(stack_dtor)(&context->calcOp_stack);
                goto finish;
            }
            ++operandsCnt;
        }

        assert(status != ginterpreter_status_OK && "This should never happen, because all operand sequences end with empty format");

        if (status != ginterpreter_status_EmptyFormat) {
            GENERIC(stack_dtor)(&context->calcOp_stack);
            goto finish;
        }
        
        (*((void (*)(ginterpreter *, SPU_FLOAT_TYPE **))context->commandJumpTable[opcode][operandsCnt]))(context, Operands);

        GENERIC(stack_dtor)(&context->calcOp_stack);
    }

finish:
    if (status == ginterpreter_status_EmptyFormat)
        status = ginterpreter_status_OK;
    GINTERPRETER_ASSERT_LOG(status == ginterpreter_status_OK, status);
    return status;
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
        GINTERPRETER_ASSERT_LOG(false, ginterpreter_status_AllocErr);
    }

    return ginterpreter_status_OK;
}
