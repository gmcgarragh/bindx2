#*******************************************************************************
#
# Copyright (C) 2014-2017 Greg McGarragh <gregm@atmos.colostate.edu>
#
# This source code is licensed under the GNU General Public License (GPL),
# Version 3.  See the file COPYING for more details.
#
#*******************************************************************************
.SUFFIXES: .c

OBJECTS   = bindx.o \
            bindx_c.o \
            bindx_cpp.o \
            bindx_f77.o \
            bindx_f90.o \
            bindx_idl.o \
            bindx_parse.o \
            bindx_parse_int.o \
            bindx_py.o \
            bindx_util.o \
            bindx_yylex_int.o \
            gindex_name_value.o \
            glist.o \
            gutil_error.o \
            gutil_string.o

EXTRA_CLEANS = bindx_yylex_int.c bindx_yylex_int.h

include make.inc

all: bindx


bindx: $(OBJECTS)
	$(CC) $(CCFLAGS) -o bindx $(OBJECTS)

bindx_yylex_int.o: bindx_yylex_int.c bindx_yylex_int.h
	$(CC) $(CCFLAGS) -Wno-unused-function -c $(CCDEFINES) $(INCDIRS) -o bindx_yylex_int.o bindx_yylex_int.c -I.

bindx_yylex_int.c bindx_yylex_int.h: bindx_yylex_int.l
	flex --header-file=bindx_yylex_int.h --outfile=bindx_yylex_int.c --prefix=yy_ bindx_yylex_int.l

README: readme_source.txt
	fold --spaces --width=80 readme_source.txt > README
	sed -i 's/[ \t]*$$//' README

clean:
	rm -f *.o bindx $(EXTRA_CLEANS)

.c.o:
	$(CC) $(CCFLAGS) $(INCDIRS) -I. -c -o $*.o $<

include dep.inc
