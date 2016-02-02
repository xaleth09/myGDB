#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include "my_gdb.h"

error_t restore_instruction(mygdb_info_t* gdb_info, int bp_index);

error_t enable_breakpoint(mygdb_info_t* gdb_info, int bp_index);

error_t insert_breakpoints(mygdb_info_t* gdb_info);

error_t at_breakpoint(mygdb_info_t* gdb_info);

void add_bp_to_bps(mygdb_info_t* gdb_info, cmd_info_t* cmd_info, Dwarf_Addr addr, int bp_ind);

error_t get_addr(mygdb_info_t *gdb_info, cmd_info_t* cmd_info, Dwarf_Addr* addr);

#endif
