#ifndef CMDS_HELPER_FUNCS_H
#define CMDS_HELPER_FUNCS_H

#include <stdbool.h>
#include "breakpoints.h"


//realease_dbg and load_dbg are file cmd helper functions
//to initialize the Dwarf_Debug in gdb_info
//and to clean it up when my_gdb is finished
error_t load_dbg(mygdb_info_t* gdb_info);

void release_dbg(mygdb_info_t* gdb_info);

//parse_args is the run_cmd helper function
//to parse given arguments and pass them to exec
//call in run_cmd()
void parse_args(mygdb_info_t* gdb_info, cmd_info_t* info, char* input);

//All below functions are print_cmd helper functions
//to traverse func dies and var dies to find var address
//and print it's value
error_t ip_in_func(Dwarf_Die the_die, size_t rip, bool* res);

error_t find_func_die(mygdb_info_t* gdb_info, Dwarf_Die* ret_die);

error_t is_right_var(Dwarf_Die var_die, char* var_name, bool* found);

error_t print_local_var(mygdb_info_t* gdb_info, Dwarf_Die var_die, char* var_name);

error_t find_var_die(mygdb_info_t* gdb_info, Dwarf_Die func_die, char* var_name);

#endif
