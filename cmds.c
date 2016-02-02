#include "cmds.h"


//File CMD
error_t file_cmd(mygdb_info_t* gdb_info, cmd_info_t* info){

	if( gdb_info->dbg != NULL ){
		//release_dbg(gdb_info);
	}

	if( access(info->file.name, F_OK | R_OK | X_OK) ){
		printf("Not a valid file name: %s\n", info->file.name);		
		return E_NON_FATAL;
	}

	strcpy(gdb_info->src_file, info->file.name);
	
	return load_dbg(gdb_info);

}



//Set_Breakpoint CMD
error_t set_breakpoint_cmd(mygdb_info_t* gdb_info, cmd_info_t* cmd_info){
	Dwarf_Addr addr;
	int bp_ind = gdb_info->breakpoints.count;
	
	if(bp_ind == MAX_BREAKPOINTS){
		printf("Reached max breakpoints\n");
		return E_NON_FATAL;
	}
	
	error_t res;
	if( (res = get_addr(gdb_info, cmd_info, &addr)) == E_FATAL )
		return E_FATAL;
	else if(res == E_NON_FATAL)
		return E_NON_FATAL;
	
	add_bp_to_bps(gdb_info, cmd_info, addr, bp_ind);
		
	printf("Added Breakpoint %d at %p: file %s, line %u\n", bp_ind, (void*)gdb_info->breakpoints.points[bp_ind].addr, gdb_info->src_file, (unsigned int)gdb_info->breakpoints.points[bp_ind].line_number );
		
		
	//if child is running, enable breakpoint
	//otherwise it will	be set when run is called
	if( gdb_info->child_pid != -1){ 
		enable_breakpoint(gdb_info, bp_ind);
	}
	
	
	return E_NONE;
}



//Run CMD
error_t run_target(mygdb_info_t* gdb_info, cmd_info_t* cmd_info){
	
	if( gdb_info->dbg == 0 ){
		return E_NON_FATAL;
	}
	
	if( (gdb_info->child_pid = fork()) == -1 ){
		printf("fork failed");
		return E_FATAL;
	}
	
	if(gdb_info->child_pid){
		waitpid(gdb_info->child_pid, NULL, 0);
		insert_breakpoints(gdb_info);
		ptrace(PTRACE_CONT, gdb_info->child_pid, NULL, NULL);
	
	}else{  //CHILD
		if( ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0 ){		
			return E_FATAL;
		}
		
		if( execvp(cmd_info->run.args[0], cmd_info->run.args) == -1 ){
			printf("failed to exec: %s\n", gdb_info->src_file);
			return E_NON_FATAL;		
		}
	}
	
	return E_NONE;
}



//Print CMD
error_t print_cmd(mygdb_info_t* gdb_info, char* var_name){
	
	Dwarf_Die func_die;
	
	if( find_func_die(gdb_info, &func_die) == E_FATAL )
		return E_FATAL;

	if( find_var_die(gdb_info, func_die, var_name) == E_FATAL ){
		return E_FATAL;
	}
		
	return E_NONE;
}



//Continue CMD
error_t cont_cmd(mygdb_info_t* gdb_info){
	int status;
	struct user_regs_struct regs;

	if(gdb_info->child_pid == -1){
		printf("The program is not being run.\n");
		return E_NON_FATAL;
	}else{
		if( ptrace(PTRACE_GETREGS, gdb_info->child_pid, NULL, &regs) < 0 )
			return E_FATAL;
			
		regs.rip = regs.rip - 1;
		
		if( ptrace(PTRACE_SETREGS, gdb_info->child_pid, NULL, &regs) < 0 )
			return E_FATAL;
			
		restore_instruction(gdb_info, gdb_info->active_breakpoint);
		
		if( ptrace(PTRACE_SINGLESTEP, gdb_info->child_pid, NULL, NULL) < 0 )
			return E_FATAL;
			
		waitpid(gdb_info->child_pid, &status, 0);
		
		if(WIFEXITED(status)){
			gdb_info->child_pid = -1;
			return E_NON_FATAL;	
		}
		
		enable_breakpoint(gdb_info, gdb_info->active_breakpoint);
		
		if( ptrace(PTRACE_CONT, gdb_info->child_pid, NULL,NULL) < 0 )
			return E_FATAL;
	}

	return E_NONE;
}



//Quit CMD
void quit_cmd(mygdb_info_t* gdb_info){
	
	
	if(gdb_info->child_pid != -1){
				kill(gdb_info->child_pid, SIGKILL);
				gdb_info->child_pid = -1;			
	}
	
	if(gdb_info->dbg)
		release_dbg(gdb_info);
		
	
	exit(EXIT_SUCCESS);

}


