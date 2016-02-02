#ifndef MY_GDB_H
#define MY_GDB_H

#include "dwarf.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include "libdwarf.h"
#include <sys/reg.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ptrace.h>

#define MAX_BREAKPOINTS 25
#define MAX_CMD_ARGS 20
#define MAX_IN_LINE_SZ 512

#define STR_FILE        "file"
#define STR_RUN         "run"
#define STR_SET_BREAK   "break"
#define STR_PRINT		"print"
#define STR_CONT        "cont"
#define STR_QUIT        "quit"

#define HIGH_BIT_MASK (~((long) 0xFF))

typedef enum{
	E_FATAL,
	E_NON_FATAL,
	E_NONE
} error_t;

typedef enum{
	CT_EOF,
	CT_FILE,
	CT_RUN,
	CT_SET_BREAKPOINT,
	CT_CONTINUE,
	CT_QUIT,
	CT_PRINT,
	CT_INVALID,
	CT_NONE
} cmd_type_t;

typedef union{

	struct{
		char name[MAX_IN_LINE_SZ];
	} file;

	struct{
		int num_args;
		char *args[MAX_CMD_ARGS];
	} run;

	struct{
		char var_name[MAX_IN_LINE_SZ];
	}print;

	struct{
		char file[MAX_IN_LINE_SZ];
		int line_num;
	} set_breakpoint;

} cmd_info_t;


typedef struct{
	size_t line_number;
	long original;
	Dwarf_Addr addr;
} breakpoint_t;

typedef struct{
	breakpoint_t points[MAX_BREAKPOINTS];
	int bp_arr[MAX_BREAKPOINTS];
	int count;
} breakpoints_t;


typedef struct{
	
	char cmd[MAX_IN_LINE_SZ];
	Dwarf_Debug dbg;
	Dwarf_Die cu_die;
	int dbg_fd;
	pid_t child_pid;
	breakpoints_t breakpoints;
	ssize_t active_breakpoint;  //current breakpoint
	char src_file[MAX_IN_LINE_SZ];

}mygdb_info_t;

#endif
