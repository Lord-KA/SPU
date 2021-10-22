#include "ginterpreter.h"

void ginterpreter_ctor(ginterpreter *interpreter)
{
    stack_ctor(&interpreter->Stack);
}

void ginterpreter_dtor(ginterpreter *interpreter)
{
    stack_dtor(&interpreter->Stack);
}

void ginterpreter_push(ginterpreter *interpreter, GINTERPRETER_TYPE value)
{
    stack_push(&interpreter->Stack, value);
}

void ginterpreter_pop(ginterpreter *interpreter)
{
    stack_pop(&interpreter->Stack, 0);
}

void ginterpreter_add(ginterpreter *interpreter)
{
    GINTERPRETER_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);

    stack_push(&interpreter->Stack, val_1 + val_2);
}

void ginterpreter_sub(ginterpreter *interpreter)
{
    GINTERPRETER_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);

    stack_push(&interpreter->Stack, val_1 - val_2);
}

void ginterpreter_mul(ginterpreter *interpreter)
{
    GINTERPRETER_TYPE val_1, val_2;

    stack_pop(&interpreter->Stack, &val_1);
    stack_pop(&interpreter->Stack, &val_2);

    stack_push(&interpreter->Stack, val_1 * val_2);
}

void ginterpreter_out(ginterpreter *interpreter)
{
    GINTERPRETER_TYPE val;
    stack_pop(&interpreter->Stack, &val);
    printf("%d", val);
}


void ginterpreter_run(ginterpreter *interpreter, const char *buffer)
{
    char keyword[GINTERPRETER_BUFLEN];
    sscanf(buffer, "%s", keyword);
    printf("keyword = %s\n", keyword);
    if (!strcmp(keyword, "push")) {
        GINTERPRETER_TYPE val;
        sscanf(buffer, "%s %d", keyword, &val);
        ginterpreter_push(interpreter, val);
    } else if (!strcmp(keyword, "pop")) 
        ginterpreter_pop(interpreter);
    else if (!strcmp(keyword, "add")) 
        ginterpreter_add(interpreter);
    else if (!strcmp(keyword, "mul")) 
        ginterpreter_mul(interpreter);
    else if (!strcmp(keyword, "sub")) 
        ginterpreter_sub(interpreter);
    else if (!strcmp(keyword, "out")) 
        ginterpreter_out(interpreter);
    else
        assert(!"ERROR: no such command!");
}


