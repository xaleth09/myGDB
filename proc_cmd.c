#include "proc_cmd.h"


//Switch based on given string cmd
error_t process_cmd(mygdb_info_t* gdb_info, cmd_type_t type, cmd_info_t* info){
	error_t res;

	switch(type){
		case CT_FILE:
			return file_cmd(gdb_info, info);

		case CT_RUN:
			if(run_target(gdb_info, info) == E_FATAL){
				return E_FATAL;
			}
			break;
		
		case CT_SET_BREAKPOINT:
				
			if( (res = set_breakpoint_cmd(gdb_info, info)) == E_FATAL ){
				printf("set_breakpoint() failed! Exiting...\n");
				return E_FATAL;
			}
			
			break;

		case CT_CONTINUE:
			cont_cmd(gdb_info);
			break;


		case CT_PRINT:
			print_cmd(gdb_info, info->print.var_name);
			break;

		case CT_QUIT:
			quit_cmd(gdb_info);
			break;
		
		case CT_INVALID:
		case CT_NONE:
		case CT_EOF:
			break;
	}

	return E_NONE;
}


//Parse user input
cmd_type_t process_input(mygdb_info_t* gdb_info,cmd_info_t* info){

	char* tmp;
	char in_cmd[MAX_IN_LINE_SZ];
	//char* parsed_cmd[MAX_CMD_ARGS];
	
	fgets(in_cmd, MAX_IN_LINE_SZ, stdin);
	if(strcmp(in_cmd, "\n") == 0){		
		return CT_NONE;
	}
	
	tmp = strdup(in_cmd);
	tmp = strtok(tmp," \n");

	if(strcmp(tmp, STR_FILE) == 0){
		tmp = strtok(NULL, " \n");
		if(tmp == NULL){
			printf("No file name given\n");
			return CT_INVALID;			
		}	
		strcpy(info->file.name, tmp);
		return CT_FILE;
		
	}else if(strcmp(tmp, STR_RUN) == 0){
		if(gdb_info->child_pid != -1){
			printf("The program being debugged has been started already.\n");
			return CT_INVALID;
		}
	
		if( strcmp(gdb_info->src_file,"\0") != 0 ){
			char* runtmp;
			runtmp = strdup(in_cmd);
			parse_args(gdb_info, info, runtmp);
			return CT_RUN;
		}		
		printf("No source is loaded. use the ""file"" command.\n");
		return CT_INVALID;

	}else if(strcmp(tmp, STR_SET_BREAK) == 0){
		tmp = strtok(NULL, " \n");
		if(tmp == NULL){
			printf("No line number given\n");
			return CT_INVALID;
		}
		if( strcmp(gdb_info->src_file,"\0") != 0 ){
			strcpy(info->set_breakpoint.file, gdb_info->src_file);
			info->set_breakpoint.line_num = atoi(tmp);	
			return CT_SET_BREAKPOINT;
		}else{
			printf("No source is loaded. Use the ""file"" command.\n");
			return CT_INVALID;		
		}
		
	}else if(strcmp(tmp, STR_PRINT) == 0){
		if(gdb_info->child_pid == -1){
			printf("No context, no child running.\n");
			return CT_INVALID;
		}
		tmp = strtok(NULL, " \n");
		if(tmp == NULL){
			printf("No var name given\n");
			return CT_INVALID;
		}
		
		strcpy(info->print.var_name, tmp);
		return CT_PRINT;
		
	}else if(strcmp(tmp, STR_CONT) == 0){
		return CT_CONTINUE;
		
	}else if(strcmp(tmp, STR_QUIT) == 0){
		return CT_QUIT;
	}

	printf("Not a valid command: %s\nTry: %s, %s, %s, %s or %s.\n", tmp, STR_FILE, STR_RUN, STR_SET_BREAK, STR_CONT, STR_QUIT);
	free(tmp);
	return CT_INVALID;
}




