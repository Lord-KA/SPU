#ifndef GINTERPRETER_H
#define GINTERPRETER_H

typedef int GINTERPRETER_TYPE;

#define FULL_DEBUG
typedef GINTERPRETER_TYPE STACK_TYPE;
#include "gstack.h"

enum gcommand {
    push,
    pop,
    mul,
    add,
    sub,
    out
};

struct ginterpreter {
    stack Stack;
    void *commandJumpTable;
} typedef ginterpreter;


#endif /* GINTERPRETER_H */
