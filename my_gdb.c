#include "proc_cmd.h"


mygdb_info_t gdb_info;

void gdb_info_init(mygdb_info_t* gdb_info){

	gdb_info->dbg = 0;
	gdb_info->child_pid = -1;
	gdb_info->active_breakpoint = -1;
	gdb_info->breakpoints.count = 0;
	strcpy(gdb_info->src_file, "\0");
	
	

}


int main(){
	
	cmd_info_t info;
	cmd_type_t c_type;
	gdb_info_init(&gdb_info);	

	while(1){
		printf("(mygdb) ");
		c_type = process_input(&gdb_info, &info);
		if(c_type == CT_NONE)
			continue;

		if(c_type != CT_INVALID && (process_cmd(&gdb_info, c_type, &info) == E_FATAL)){
			fprintf(stderr,"Fatal Error in process_cmd\n");
			if(gdb_info.child_pid != -1){
				kill(gdb_info.child_pid, SIGKILL);
				gdb_info.child_pid = -1;			
			}
			return EXIT_FAILURE;
		}	
		int status;
		if( (c_type == CT_CONTINUE || c_type == CT_RUN) && gdb_info.child_pid != -1){
			waitpid(gdb_info.child_pid, &status, 0);
			if(WIFEXITED(status)){		
				printf("\nProcess %d exited normally.\n", gdb_info.child_pid);
				gdb_info.child_pid = -1;
				continue;
			}
			
			if(at_breakpoint(&gdb_info) == E_FATAL){
				printf("Failed to reach breakpoint\n");
				return EXIT_FAILURE;
			}
			
			if(gdb_info.active_breakpoint < 0){
				printf("Failed to determine break\n");
			}else{
				ssize_t active_bp = gdb_info.active_breakpoint;
				printf("At breakpoint %ld, at line: %u\n",active_bp, (unsigned int)gdb_info.breakpoints.points[active_bp].line_number);
			}
		}	
		
	}

	return 0;
}


