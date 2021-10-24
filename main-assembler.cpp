#include "ginterpreter.h"
#include "gassembly.h"
#include <unistd.h>

enum gMode {
    gNone = 0,
    gAssemble,
    gDisassamble
};

int main(int argc, char **argv) {
    
    int c = 0;
    gMode mode = gNone;
    char *fileName = NULL;
    FILE *in  = stdin;
    FILE *out = stdout;
    
    while ((c = getopt(argc, argv, "adho:")) != -1) {
        switch(c) {
        case 'h':
            printf("This is demo of gAssembler\n"
                   "Usage: -h          [show this help]\n"
                   "       -a          [assemble]\n"
                   "       -d          [disassemble]\n"
                   "       -o FileName [output to file]\n");
            return 0;
        case 'a':
            mode = gAssemble;
            break;
        case 'd':
            mode = gDisassamble;
            break;
        case 'o':
            fileName = optarg;
            break;
        }
    }
    if (fileName != NULL) {
        out = fopen(fileName, "w");
    }

    // printf("mode = %d\n", mode);
    if (mode == gAssemble)
        gassembly_assembleFromFile(in, out);
    else if (mode == gDisassamble)
        gassembly_disassembleFromFile(in, out);
    else
        fprintf(stderr, "ERROR: unknown mode, -h for help\n");
}
