#include "gassembly.cpp"

int main() 
{
    int status = gassembly_getOperand(stdin, stdout);
    printf("\nstatus = %d\n", status);
}
