#include "ginterpreter.h"
#include "gassembly.h"

int main() {
    ginterpreter inter;
    ginterpreter_ctor(&inter);
    int status = ginterpreter_runFromFile(&inter, stdin);
    if (status)
        fprintf(stderr, "ERROR: error occured during interpretation, exit_code = %d (%s)\n", status, ginterpreter_statusMsg[status]);
    stack_dump(&inter.Stack);

    ginterpreter_dtor(&inter);
}
