/**
  * COMMAND(name, Name, isFirst, argc, code)
  * WARNING: isFirst must be `true` or `false`, not a digit or a string! 
  *
  * `SPU_FLOAT_TYPE **valList` is a null-terminated list of opcode operands with length of `MAX_OPERANDS + 1`
  */


#define POP(valPtr) stack_pop (&context->Stack, (valPtr))
#define PUSH(val)   stack_push(&context->Stack, (val))

#define ARG_1 **valList
#define ARG_2 **(valList + 1)
/* WARNING: default max operands number is 2, you can change that in ./include/gconfig.h */
#define ARG_3 **(valList + 2) 

COMMAND(idle, Idle, true, 0, ({

    return;
}))

COMMAND(push, Push, true, 1, ({
    PUSH(ARG_1);
}))


COMMAND(pop, Pop, true, 0, ({
    POP(NULL);
}))

COMMAND(pop, Pop, false, 1, ({
    POP(&ARG_1);
}))

COMMAND(add, Add, true, 0, ({
    SPU_FLOAT_TYPE val_1, val_2;

    POP(&val_1);
    POP(&val_2);

    PUSH(val_1 + val_2);
}))

COMMAND(add, Add, false, 2, ({
    ARG_1 += ARG_2;
}))

COMMAND(sub, Sub, true, 0, ({
    SPU_FLOAT_TYPE val_1, val_2;

    POP(&val_1);
    POP(&val_2);
 
    PUSH(val_1 - val_2);
}))

COMMAND(sub, Sub, false, 2, ({
    ARG_1 -= ARG_2;
}))

COMMAND(mul, Mul, true, 0, ({
    SPU_FLOAT_TYPE val_1, val_2;

    POP(&val_1);
    POP(&val_2);
    
    PUSH(val_1 * val_2);
}))

COMMAND(mul, Mul, false, 2, ({
    ARG_1 *= ARG_2;
}))

COMMAND(mov, Mov, true, 2, ({
    ARG_1 = ARG_2;
}))

COMMAND(out, Out, true, 0, ({
    SPU_FLOAT_TYPE val;
    POP(&val);
    printf("%d\n", val);
}))

COMMAND(out, Out, false, 1, ({
    printf("%d\n", ARG_1);
}))

COMMAND(jmp, Jmp, true, 1, ({
    SPU_INTEG_TYPE pos = ARG_1;
    fseek(context->inStream, pos, SEEK_SET);
}))


#undef PUSH
#undef POP
#undef ARG_1
#undef ARG_2
#undef ARG_3
