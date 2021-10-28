#include "ginterpreter.h"
#include "gassembly.h"

int main() {
    ginterpreter inter;
    ginterpreter_ctor(&inter);
    ginterpreter_runFromFile(&inter, stdin);
    stack_dump(&inter.Stack);

    ginterpreter_dtor(&inter);
}
