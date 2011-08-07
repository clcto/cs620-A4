#define LINE_LEN (80)
#define MAX_ARGS (64)
#define MAX_ARG_LEN (32)
#define MAX_PATHS (64) 
#define MAX_PATH_LEN (96)

#ifndef NULL
#define NULL (0)
#endif

typedef struct
{
   char *name;
   int argc;
   char *argv[MAX_ARGS];
   int concurrent;
} command_t;
