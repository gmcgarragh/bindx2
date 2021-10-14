#*******************************************************************************
#
# Copyright (C) 2014-2018 Greg McGarragh <greg.mcgarragh@colostate.edu>
#
# This source code is licensed under the GNU General Public License (GPL),
# Version 3.  See the file COPYING for more details.
#
#*******************************************************************************
.SUFFIXES: .c

include make.inc

INCDIRS += -I.

OBJECTS = bindx.o \
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

all: bindx

bindx: $(OBJECTS)
	$(CC) $(CCFLAGS) -o bindx $(OBJECTS) \
        $(INCDIRS) $(LIBDIRS) $(LINKS)

bindx_yylex_int.o: bindx_yylex_int.c bindx_yylex_int.h
	$(CC) $(CCFLAGS) -Wno-unused-function -c $(INCDIRS) -o bindx_yylex_int.o bindx_yylex_int.c -I.

bindx_yylex_int.c bindx_yylex_int.h: bindx_yylex_int.l
	flex --header-file=bindx_yylex_int.h --outfile=bindx_yylex_int.c --prefix=yy_ bindx_yylex_int.l

README: readme_source.txt
	fold --spaces --width=80 readme_source.txt > README
	sed -i 's/[ \t]*$$//' README

clean:
	rm -f *.o bindx $(EXTRA_CLEANS)

.c.o:
	$(CC) $(CCFLAGS) $(INCDIRS) -c -o $*.o $<

# Requires gcc
depend:
	@files=`find . -maxdepth 1 -name "*.c" | sort`; \
        if test $${#files} != 0; then \
             if (eval gcc -v 1> /dev/null 2>&1); then \
                  echo gcc -MM -w $$files "> depend.inc"; \
                  gcc -MM -w $$files > depend.inc; \
             else \
                  echo makedepend -f- -Y -- -- $$files "> depend.inc"; \
                  makedepend -f- -Y -- -- $$files > depend.inc; \
             fi \
        else \
             echo -n '' > dep.inc; \
        fi

# Requires cproto
proto_types:
	@if [ -e prototypes ]; then \
             files='$(OBJECTS)'; \
	     for object in $$files; do \
                  source=`echo $$object | sed 's|\.o|\.c|'`; \
                  if [ -e $$source ]; then \
                       header=`echo $$object | sed 's|\.o|_p\.h|'`; \
                       echo "cproto ${CCDEFINES} ${INCDIRS} $$source > prototypes/$$header"; \
                       cproto ${CCDEFINES} -DCPROTO ${INCDIRS} $$source > prototypes/$$header; \
                  fi \
	     done \
        fi

include depend.inc
