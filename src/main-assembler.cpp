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
        out = fopen(fileName, "wb");         //TODO check fopen
        if (out == NULL) {
            fprintf(stderr, "Failed to open the file!\n");
            return 1;
        }
    }

    // fprintf(stderr, "mode = %d\n", mode);
    gassembly_status status = gassembly_status_OK;
    if (mode == gAssemble) {
        status = gassembly_assembleFromFile(in, out);
        if (status != gassembly_status_OK)
            fprintf(stderr, "Error during assembling, error_code = %d (%s)\n", status, gassembly_statusMsg[status]);
    }
    else if (mode == gDisassamble) {
        status = gassembly_disassembleFromFile(in, out);
        if (status != gassembly_status_OK)
            fprintf(stderr, "Error during disassembling, error_code = %d (%s)\n", status, gassembly_statusMsg[status]);
    }
    else {
        fprintf(stderr, "ERROR: unknown mode, -h for help\n");
        return 1;
    }
    return 0;
}
