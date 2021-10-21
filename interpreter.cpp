#include "interpreter.h"

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
