/*
 * Single file library for parsing command line arguments
 * Developed by: Andre Bezerra
 */

#ifndef _SLB_ARGS_H
#define _SLB_ARGS_H

#define ARG_MAX_LENGTH 256

typedef struct {
    int current_argument;
    int argc;
    char **argv;
} Slb_args;


Slb_args slb_args_init(int argc, char **argv);
char*    slb_args_next(Slb_args *args);


#ifdef SLB_ARGS_IMPLEMENTATION

Slb_args slb_args_init(int argc, char **argv)
{
    Slb_args args = {
        .current_argument = 0,
        .argc = argc,
        .argv = argv,
    };
    return args;
}

char* slb_args_next(Slb_args *args)
{
    if (args->current_argument + 1 > args->argc) return NULL;
    return args->argv[args->current_argument++];
}


#endif  //SLB_ARGS_IMPLEMENTATION


#endif //_SLB_ARGS_H
