#include "gassembly.h"
#include "ginterpreter.h"
#include "gtest/gtest.h"

TEST(manual, putOperand)
{
    char binBuf[1000] = {};
    char outBuf[1000] = {};
    char program_1[1000] = "main:\n"
                           "push 179 + 12\n"
                           "out\n"
                           "ret\n";

    char expected_out[1000] = "191\n";
 
    FILE *bin_out     = fmemopen(binBuf, sizeof(char), "wb");
    FILE *program_in  = fmemopen(program_1, sizeof(char), "r");
    gassembly_status status = gassembly_status_OK;

    status = gassembly_assembleFromFile(program_in, bin_out);
    if (status != gassembly_status_OK)
        fprintf(stderr, "error_code = %d (%s)\n", status, gassembly_statusMsg[status]);

    fclose(bin_out);
    fclose(program_in);
    
    FILE *bin_in      = fmemopen(binBuf, sizeof(char), "rb");
    FILE *program_out = fmemopen(outBuf, sizeof(char), "w");

    ginterpreter inter;
    ginterpreter_status status_2;
    status_2 = ginterpreter_ctor(&inter, program_out);
    if (status_2 != gassembly_status_OK)
        fprintf(stderr, "error_code = %d (%s)\n", status_2, ginterpreter_statusMsg[status_2]);

    status_2 = ginterpreter_runFromFile(&inter, bin_in);
    if (status_2 != gassembly_status_OK)
        fprintf(stderr, "error_code = %d (%s)\n", status_2, ginterpreter_statusMsg[status_2]);
    fclose(bin_in);
    fclose(program_out);

    fprintf(stderr, "program_out = #%s#\n", outBuf);
    EXPECT_FALSE(strcmp(outBuf, expected_out));
}
