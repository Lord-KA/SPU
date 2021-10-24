#ifndef GINTERPRETER_H
#define GINTERPRETER_H

#include "gconfig.h"
#include "gopcodes.h"

typedef int GINTERPRETER_TYPE;

#define FULL_DEBUG

#include "gstack.h"

static const GINTERPRETER_TYPE GINTERPRETER_BUFLEN = 64;


struct ginterpreter {
    stack Stack;
    void *commandJumpTable;
} typedef ginterpreter;

void ginterpreter_ctor(ginterpreter *interpreter);

void ginterpreter_dtor(ginterpreter *interpreter);

void ginterpreter_push(ginterpreter *interpreter, GINTERPRETER_TYPE value);

void ginterpreter_pop(ginterpreter *interpreter);

void ginterpreter_add(ginterpreter *interpreter);

void ginterpreter_sub(ginterpreter *interpreter);

void ginterpreter_mul(ginterpreter *interpreter);

void ginterpreter_out(ginterpreter *interpreter);

void ginterpreter_runFromFile(ginterpreter *interpreter, FILE *in);

#endif /* GINTERPRETER_H */
