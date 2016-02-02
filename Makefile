CFLAGS=-g -I/usr/include/libdwarf -Wall

PROGRAMS = proc_cmd.c cmds.c cmds_helper_funcs.c breakpoints.c my_gdb.c 
HEADERS = my_gdb.h proc_cmd.h cmds.h cmds_helper_funcs.h breakpoints.h

.PHONY: all clean

all: my_gdb

my_gdb: $(HEADERS) $(PROGRAMS)
	gcc $(CFLAGS) $(PROGRAMS) -ldwarf -lelf -o $@
	
	
testfile: testfile.c
	gcc $(CFLAGS) testfile.c -o testfile	
	
clean:
	rm -rf *~ my_gdb
