#include "ginterpreter.h"
#include "gassembly.h"

int main() {
    /*
    ginterpreter inter;
    ginterpreter_ctor(&inter);
    ginterpreter_run(&inter, "push 12");
    ginterpreter_run(&inter, "push 13");
    ginterpreter_run(&inter, "push 14");
    ginterpreter_run(&inter, "add");
    ginterpreter_run(&inter, "mul");
    ginterpreter_run(&inter, "out");
    stack_dump(&inter.Stack);
    */
    // gassembly_assembleFromLine("push 49", stdout);
    // gassembly_assembleFromLine("add", stdout);
    // gassembly_assembleFromLine("out", stdout);
    gassembly_disassembleFromFile(stdin, stdout);
}
