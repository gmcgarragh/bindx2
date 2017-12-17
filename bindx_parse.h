/******************************************************************************%
**
**    Copyright (C) 2011-2017 Greg McGarragh <gregm@atmos.colostate.edu>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#ifndef BINDX_PARSE_H
#define BINDX_PARSE_H

#include <glist.h>

#include "bindx_util.h"

#ifdef __cplusplus
extern "C" {
#endif


enum lex_type {
     LEX_TYPE_STRING = 1,
     LEX_TYPE_LONG,
     LEX_TYPE_DOUBLE,
     LEX_TYPE_IDENTIFIER
};


enum lex_bindx_type {
     LEX_BINDX_TYPE_VOID = 128,
     LEX_BINDX_TYPE_ENUM,
     LEX_BINDX_TYPE_CHAR,
     LEX_BINDX_TYPE_INT,
     LEX_BINDX_TYPE_DOUBLE,
     LEX_BINDX_TYPE_STRUCTURE
};


typedef union {
     char *s;
     long l;
     double d;
} lex_type_data;


typedef struct {
     char *file;
     int line;
     int character;
} locus_data;


#define INTERNAL_ERROR(FORMAT, ...) do {			\
    internal_error(__FILE__, __LINE__, FORMAT,  __VA_ARGS__);	\
} while (0)


#include "prototypes/bindx_parse_p.h"


#ifdef __cplusplus
}
#endif

#endif /* BINDX_PARSE_H */
