#ifndef PROC_CMD_H
#define PROC_CMD_H

#include "cmds.h"

error_t process_cmd(mygdb_info_t* gdb_info, cmd_type_t type, cmd_info_t* info);

cmd_type_t process_input(mygdb_info_t* gdb_info, cmd_info_t* info);

#endif
