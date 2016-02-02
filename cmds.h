#include "cmds_helper_funcs.h"

error_t file_cmd(mygdb_info_t* gdb_info, cmd_info_t* info);

error_t set_breakpoint_cmd(mygdb_info_t* gdb_info, cmd_info_t* cmd_info);

error_t run_target(mygdb_info_t* gdb_info, cmd_info_t* cmd_info);

error_t print_cmd(mygdb_info_t* gdb_info, char* var_name);

error_t cont_cmd(mygdb_info_t* gdb_info);

void quit_cmd(mygdb_info_t* gdb_info);

