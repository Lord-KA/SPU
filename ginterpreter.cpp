#include "ginterpreter.h"

void ginterpreter_ctor(ginterpreter *interpreter)
{
    stack_ctor(&interpreter->Stack);
}

void ginterpreter_dtor(ginterpreter *interpreter)
{
    stack_dtor(&interpreter->Stack);
}

void ginterpreter_push(ginterpreter *interpreter, SPU_VAL_TYPE value)
{
    stack_push(&interpreter->Stack, value);
}

void ginterpreter_pop(ginterpreter *interpreter)
{
    stack_pop(&interpreter->Stack, 0);
}

void ginterpreter_add(ginterpreter *interpreter)
{
    SPU_VAL_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);

    stack_push(&interpreter->Stack, val_1 + val_2);
}

void ginterpreter_sub(ginterpreter *interpreter)
{
    SPU_VAL_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);

    stack_push(&interpreter->Stack, val_1 - val_2);
}

void ginterpreter_mul(ginterpreter *interpreter)
{
    SPU_VAL_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);

    stack_push(&interpreter->Stack, val_1 * val_2);
}

void ginterpreter_out(ginterpreter *interpreter)
{
    SPU_VAL_TYPE val;
    stack_pop(&interpreter->Stack, &val);
    printf("%d", val);
}

void *opcodeFuncTable[] = {NULL, (void*)&ginterpreter_push, (void*)&ginterpreter_pop, (void*)&ginterpreter_mul, (void*)&ginterpreter_add, (void*)&ginterpreter_sub, (void*)&ginterpreter_out};

void ginterpreter_runFromFile(ginterpreter *interpreter, FILE *in)
{
    char opcode;
    fread(&opcode, sizeof(char), 1, in);
    while (!feof(in)) {
        printf("opcode = %s", gDisassambleTable[opcode]);
        if (opcode == 1) {
            SPU_VAL_TYPE val;
            assert(!feof(in) && "ERROR: file ended before push argument!");
            fread(&val, sizeof(SPU_VAL_TYPE), 1, in);
            printf(" %d\n", val);
            ginterpreter_push(interpreter, val);
        } else if (opcode < gCnt) {
            printf("\n");
            ((void (*)(ginterpreter*))opcodeFuncTable[opcode])(interpreter);
        }
        else
            assert(!"ERROR: no such command!");
        fread(&opcode, sizeof(char), 1, in);
    }
}


