#include "gassembly.cpp"

int main() {
    /* 
    char buffer_1[] = "[abs + 13 - 13f * 93] * 10 31";
    char buffer_2[] = "abs * 10 [1sdf + 19 * ksdb + 13]";
    char buffer_3[] = "[abs + * 10 w1";
    char buffer_4[] = "[abs] * 10 asdnv";
    char buffer_5[] = "[abs] * 10 ";
    char *buffer = buffer_2;
    char operand_1[100] = "";
    char operand_2[100] = "";
    int status = gassembly_parseOperand(buffer, strlen(buffer), operand_1, operand_2);
    printf("\n\nstatus = %d\n\nbuffer    = #%s#\noperand_1 = #%s#\noperand_2 = #%s#\n", status, buffer, operand_1, operand_2);
    */

    /*
    char operand_1[] = "ax"; 
    char operand_2[] = "7 * ax + 8 * bx + cx"; 
    char operand_3[] = "[8 * ax + bx] + 13"; 
    char operand_4[] = "[8 * [ax + [cx + ax]] * 8] + 13"; 
    char operand_5[] = "[8 * [ax + [cx + ax]] * 8] * [13 ] + [ rx ]"; 
    char operand_6[] = "ax [8 * [ax + [cx + ax]] * 8] * [13 ] + [ rx ]"; 
    char *operand = operand_6;
    //printf("strspn = %d\n", strchrcnt(operand, DELIMS_LIST));
    char reg = 0;
    int cnt = 0;

    // printf("sscanf = %d \t reg = %d \t cnt = %d \n", sscanf(operand_3, "%sx %n", &reg, &cnt), reg, cnt);
    // for (size_t i = 0; i < 5; ++i)
    //     printf("%c", *(&reg + i));

    // return 1;

    int status = gassembly_putOperand(operand, stdout);
    if (status)
        printf("status = %d", status);
    */

    /*
    char line_1[] = "mov ax bx ; HELLO WORLD, MOTHERFUCKER!";
    char line_2[] = "mov [ax + bx] [[bx * cx]] ; HELLO WORLD, MOTHERFUCKER!";
    char line_3[] = "mov [[ax + bx]] [[bx * cx]] ; HELLO WORLD, MOTHERFUCKER!";
    char line_4[] = "mov 12 + [[ax + bx]] [[bx * cx] + 4] * wx ; HELLO WORLD, MOTHERFUCKER!";
    char line_5[] = "push 100";

    char *line = line_4;
    gassembly_assembleFromLine(line, stdout);
    */

    char buffer_1[] = "[abs + 13 - 13f * 93] * 10 31";
    char buffer_2[] = "abs * 10 [1sdf + 19 * ksdb + 13]";
    char buffer_3[] = "[abs + * 10 w1";
    char buffer_4[] = "[ax] * 10 rx";
    char buffer_5[] = "[0xc * ax + [0x9 + ex] * dx] * 0xa + [bx]";
    char buffer_6[] = "179";
    char *buffer = buffer_6;

    gassembly_putOperand(buffer, stdout);
    // int status = gassembly_getOperand(stdin, stdout);
    // printf("\ngetOperand status = %d\n", status);
}

