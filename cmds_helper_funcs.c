#include "cmds_helper_funcs.h"

//file_cmd helper funcs
//DBG init and exit functions
error_t load_dbg(mygdb_info_t* gdb_info){
	Dwarf_Error err;
	Dwarf_Die no_die = 0;
	Dwarf_Unsigned cu_header_length;
	Dwarf_Half version_stamp;
	Dwarf_Unsigned abbrev_offset;
	Dwarf_Half addr_size;
	Dwarf_Unsigned nxt_cu_hdr;


	gdb_info->dbg_fd = open(gdb_info->src_file, O_RDONLY);

	if( dwarf_init(gdb_info->dbg_fd,DW_DLC_READ,0,0, &gdb_info->dbg, &err) != DW_DLV_OK )
		return E_FATAL;

	printf("Succesfully loaded symbol info\n");
	int res;
	if( (res = dwarf_next_cu_header(gdb_info->dbg, &cu_header_length, &version_stamp, &abbrev_offset, &addr_size, &nxt_cu_hdr, &err)) == DW_DLV_ERROR ){
		printf("failed to get next header\n");		
		return E_FATAL;
	}

	if( dwarf_siblingof(gdb_info->dbg, no_die, &gdb_info->cu_die, &err) == DW_DLV_ERROR){
		printf("siblingof() failed\n");
		return E_FATAL;
	}

	
	return E_NONE;
}

void release_dbg(mygdb_info_t* gdb_info){

	Dwarf_Error* err = NULL;
	
	dwarf_finish(gdb_info->dbg, err);
	close(gdb_info->dbg_fd);
	gdb_info->dbg_fd = -1;

}



//run_cmd helper func
void parse_args(mygdb_info_t* gdb_info, cmd_info_t* info, char* input){
	int i;
	for(i = 0; i < MAX_CMD_ARGS; i++){
		info->run.args[i] = malloc(sizeof(char)*MAX_IN_LINE_SZ);
	}
	
	printf("input = %s\n", input);
	
	char tmp[MAX_IN_LINE_SZ];
	strcpy(tmp, "");
	strcat(tmp, "./");
	strcat(tmp, gdb_info->src_file);
	info->run.args[0] = strdup(tmp);

	i = 1;
	strtok(input, " \n");
	while((input = strtok(NULL, " \n")) != NULL && i < MAX_CMD_ARGS){
		info->run.args[i++] = strdup(input);
	}

	info->run.args[i] = NULL;

}

//print_cmd helper funcs
error_t ip_in_func(Dwarf_Die the_die, size_t rip, bool* res){
	char* die_name = 0;
	Dwarf_Error err;
	Dwarf_Half tag;
	Dwarf_Attribute* attrs;
	Dwarf_Addr lowpc, highpc;
	Dwarf_Signed attrcount, i;
	int rc = dwarf_diename(the_die, &die_name, &err);

	if (rc == DW_DLV_ERROR)
		return E_FATAL;
	else if (rc == DW_DLV_NO_ENTRY)
		return E_NONE;

	if (dwarf_tag(the_die, &tag, &err) != DW_DLV_OK)
		return E_FATAL;

	/* Only interested in subprogram DIEs here */
	if (tag != DW_TAG_subprogram)
		return E_NONE;

	/* Grab the DIEs attributes for display */
	if (dwarf_attrlist(the_die, &attrs, &attrcount, &err) != DW_DLV_OK)
		return E_FATAL;
	
	for (i = 0; i < attrcount; ++i) {
		Dwarf_Half attrcode;
		if (dwarf_whatattr(attrs[i], &attrcode, &err) != DW_DLV_OK)
			return E_FATAL;

		/* We only take some of the attributes for display here.
		** More can be picked with appropriate tag constants.
		*/
		if (attrcode == DW_AT_low_pc)
			dwarf_formaddr(attrs[i], &lowpc, 0);
		else if (attrcode == DW_AT_high_pc)
			dwarf_formaddr(attrs[i], &highpc, 0);
		
	}

	if( rip >= lowpc && rip <= highpc){
			*res = true;
			return E_NONE;
		}
	

	return E_NONE;

}


error_t find_func_die(mygdb_info_t* gdb_info, Dwarf_Die* ret_die){
	Dwarf_Error err;
	//Dwarf_Die child_die;
	/* Find compilation unit header */

	/* Expect the CU DIE to have children */
	if (dwarf_child(gdb_info->cu_die, ret_die, &err) == DW_DLV_ERROR)
		return E_FATAL;

	struct user_regs_struct regs;
	if( ptrace(PTRACE_GETREGS, gdb_info->child_pid, NULL, &regs) < 0)
		return E_FATAL;
	

	/* Now go over all children DIEs */
	while (1) {
		int rc;
		bool found = false;
		if( ip_in_func(*ret_die, regs.rip - 1, &found) == E_FATAL)
			return E_FATAL;

		if(found){
			return E_NONE;
		}
	
		rc = dwarf_siblingof(gdb_info->dbg, *ret_die, ret_die, &err);	
	
		if (rc == DW_DLV_ERROR)
			return E_FATAL;	
		else if (rc == DW_DLV_NO_ENTRY){
			return E_FATAL; //Func not found!
		}
	}

	return E_FATAL; // didnt find func

}


error_t is_right_var(Dwarf_Die var_die, char* var_name, bool* found){
	char* die_name = 0;
	Dwarf_Error err;
	Dwarf_Half tag;
	Dwarf_Attribute* attrs;
	Dwarf_Signed attrcount, i;
	int rc = dwarf_diename(var_die, &die_name, &err);

	if (rc == DW_DLV_ERROR)
		return E_FATAL;
	else if (rc == DW_DLV_NO_ENTRY)
		return E_NONE;

	if (dwarf_tag(var_die, &tag, &err) != DW_DLV_OK)
		return E_FATAL;

	/* Only interested in variable DIEs here */
	if (tag != DW_TAG_variable)
		return E_NONE;


	/* Grab the DIEs attributes for display */
	if (dwarf_attrlist(var_die, &attrs, &attrcount, &err) != DW_DLV_OK)
		return E_FATAL;
	
	for (i = 0; i < attrcount; ++i) {
		Dwarf_Half attrcode;
		if (dwarf_whatattr(attrs[i], &attrcode, &err) != DW_DLV_OK)
			return E_FATAL;

		
		if (attrcode == DW_AT_name){
			char* die_var_name;
			dwarf_formstring(attrs[i], &die_var_name, &err);
			if( strcmp(die_var_name, var_name) == 0 ){
				*found = true;
				return E_NONE;	
			}
		}		
		
	}

	return E_NONE; //Not var_die being searched for
}


error_t print_local_var(mygdb_info_t* gdb_info, Dwarf_Die var_die, char* var_name){
	Dwarf_Half tag;
	Dwarf_Error err;
	unsigned long var_addr;
	Dwarf_Attribute* attrs;
	Dwarf_Locdesc* locs_var;
	Dwarf_Signed attrcount, i, cnt_var;
	
	if (dwarf_tag(var_die, &tag, &err) != DW_DLV_OK)
		return E_FATAL;

	/* Make sure to only have a var die */
	if (tag != DW_TAG_variable)
		return E_FATAL;
		
		/* Grab the DIEs attributes */
	if (dwarf_attrlist(var_die, &attrs, &attrcount, &err) != DW_DLV_OK)
		return E_FATAL;
	
	for (i = 0; i < attrcount; ++i) {
		Dwarf_Half attrcode;
		if (dwarf_whatattr(attrs[i], &attrcode, &err) != DW_DLV_OK)
			return E_FATAL;

		
		if (attrcode == DW_AT_location)
			break;
		
	}
	
	if( attrs[i] == NULL ){
		printf("Failed to find the DW_AT_location attribute\n");
		return E_NON_FATAL;
	}
	
	if( dwarf_loclist(attrs[i], &locs_var, &cnt_var, &err) != DW_DLV_OK){
		printf("dwarf_loclist failed\n");
		return E_FATAL;
	}
	
	if((cnt_var != 1) || (locs_var[0].ld_cents != 1) || (locs_var[0].ld_s[0].lr_atom != DW_OP_fbreg)){
		fprintf(stderr, "Unexpected location information\n");
		return E_NON_FATAL;
  	}
  	
  	struct user_regs_struct regs;
  	if( ptrace(PTRACE_GETREGS, gdb_info->child_pid, NULL, &regs ) < 0 )
  		return E_FATAL;
  		
  		var_addr = (unsigned long)(regs.rbp + (long)locs_var[0].ld_s[0].lr_number + 16);
		
		errno = 0;
		unsigned long data;
		data = ptrace(PTRACE_PEEKTEXT, gdb_info->child_pid, (void*)var_addr);
		
		if(errno){
			printf("peektext failed\n");
			return E_FATAL;
		}
		
		printf("%s = %lu\n", var_name, data);
		
	return E_NONE;
}


error_t find_var_die(mygdb_info_t* gdb_info, Dwarf_Die func_die, char* var_name){
	Dwarf_Half tag;
	Dwarf_Error err;
	Dwarf_Die child_die;
	/* Find compilation unit header */

	if (dwarf_tag(func_die, &tag, &err) != DW_DLV_OK)
		return E_FATAL;

	/* Can't find var if this isn't a func_die */
	if (tag != DW_TAG_subprogram)
		return E_FATAL; //

	/* Expect the CU DIE to have children */
	if (dwarf_child(func_die, &child_die, &err) == DW_DLV_ERROR)
		return E_FATAL;
	
	/* Now go over all children DIEs */
	while (1) {
		int rc;
		bool found = false;
		
		if( is_right_var(child_die, var_name, &found) == E_FATAL)
			return E_FATAL;

		if(found){
			return print_local_var(gdb_info, child_die, var_name);
		}
	
		rc = dwarf_siblingof(gdb_info->dbg, child_die, &child_die, &err);	
	
		if (rc == DW_DLV_ERROR)
			return E_FATAL;	
		else if (rc == DW_DLV_NO_ENTRY){
			printf("Couldn't find var named: %s\n", var_name);
			return E_NON_FATAL; //var not found!
		}
	}


	return E_FATAL; //if reached here, didnt find var and didnt hit end of dies list
}					//must have broken something



