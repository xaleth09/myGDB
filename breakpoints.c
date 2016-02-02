#include "breakpoints.h"


error_t restore_instruction(mygdb_info_t* gdb_info, int bp_index){
	long data;
	breakpoint_t* bp = &gdb_info->breakpoints.points[bp_index];
	
	errno = 0;
	data = ptrace(PTRACE_PEEKTEXT, gdb_info->child_pid, (void*)bp->addr, 0);
	
	if(errno)
		return E_FATAL;
		
	if( ptrace(PTRACE_POKETEXT, gdb_info->child_pid, (void*)bp->addr, ((data & HIGH_BIT_MASK) | (bp->original & 0xFF)) ) < 0 )
		return E_FATAL;
		
	return E_NONE;
}



error_t enable_breakpoint(mygdb_info_t* gdb_info, int bp_index){
	breakpoint_t* bp = &gdb_info->breakpoints.points[bp_index];
	errno = 0;
	
	bp->original = ptrace(PTRACE_PEEKTEXT, gdb_info->child_pid, (void*)bp->addr, 0);
	
	if(errno)
		return E_FATAL;
		
	ptrace(PTRACE_POKETEXT, gdb_info->child_pid, (void*)bp->addr, ((bp->original & HIGH_BIT_MASK) | 0xCC));   
		
	return E_NONE;
}



error_t insert_breakpoints(mygdb_info_t* gdb_info){
	int i;
	for(i = 0; i < gdb_info->breakpoints.count; i++){
		enable_breakpoint(gdb_info, i);
	}

	return E_NONE;
}



error_t at_breakpoint(mygdb_info_t *gdb_info){
	size_t i;
	struct user_regs_struct regs;
	
	if( ptrace(PTRACE_GETREGS, gdb_info->child_pid, NULL, &regs) < 0)
		return E_FATAL;
		
	gdb_info->active_breakpoint = -1;
	
	for(i = 0; i < gdb_info->breakpoints.count; i++){
		if(gdb_info->breakpoints.bp_arr[i] && ((regs.rip - 1) == (size_t)gdb_info->breakpoints.points[i].addr) ){
			gdb_info->active_breakpoint = i;
			break;
		}
	}

	return E_NONE;
}



void add_bp_to_bps(mygdb_info_t* gdb_info, cmd_info_t* cmd_info, Dwarf_Addr addr, int bp_ind){

	gdb_info->breakpoints.points[bp_ind].line_number = cmd_info->set_breakpoint.line_num;	
	gdb_info->breakpoints.points[bp_ind].addr = addr;
	gdb_info->breakpoints.bp_arr[bp_ind] = 1;
	gdb_info->breakpoints.count++;	

}



error_t get_addr(mygdb_info_t *gdb_info, cmd_info_t* cmd_info, Dwarf_Addr* addr){
	Dwarf_Error err;
	Dwarf_Signed cnt;
	Dwarf_Unsigned line_num;
	Dwarf_Line *linebuf;
	error_t result = E_NONE;
	int found = 0;
	size_t i;
	
	if(dwarf_srclines(gdb_info->cu_die, &linebuf, &cnt, &err) != DW_DLV_OK){	
		printf("dwarf_srclines() failed\n");
		return E_FATAL;
	}
	
	for(i = 0; i < cnt; i++){
		if(dwarf_lineno(linebuf[i], &line_num, &err) == DW_DLV_ERROR){
			printf("dwarf_lineno() failed\n");
			result = E_FATAL;
		}
		if(line_num == cmd_info->set_breakpoint.line_num){
			found = 1;
			if( dwarf_lineaddr(linebuf[i], addr, &err) == DW_DLV_ERROR )
				result = E_FATAL;
		}
	}
	
	if(!found){
		printf("Not a valid line number: %d\n", (unsigned int)cmd_info->set_breakpoint.line_num);
		result = E_NON_FATAL;
	}
	
	return result;
}









