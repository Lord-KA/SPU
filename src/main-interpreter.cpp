#define FULL_DEBUG
#include "ginterpreter.h"
#include "gassembly.h"

int main() {
    ginterpreter inter;
    ginterpreter_ctor(&inter, NULL, NULL);
    int status = ginterpreter_runFromFile(&inter, stdin);
    if (status != ginterpreter_status_OK)
        fprintf(stderr, "ERROR: error occured during interpretation, exit_code = %d (%s)\n", status, ginterpreter_statusMsg[status]);
    GENERIC(stack_dump)(&inter.Stack);
    ginterpreter_dtor(&inter);
}
