/*******************************************************************************
**
**    Copyright (C) 2011-2018 Greg McGarragh <greg.mcgarragh@colostate.edu>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#ifndef BINDX_INT_PARSE_H
#define BINDX_INT_PARSE_H

#include <glist.h>

#include "bindx_parse.h"
#include "bindx_util.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_DIMENS 16


enum lex_item {
     LEX_ITEM_PREFIX = 256,
     LEX_ITEM_INCLUDE,
     LEX_ITEM_ENUMERATION,
     LEX_ITEM_GLOBAL_CONST,
     LEX_ITEM_STRUCTURE,
     LEX_ITEM_SUBPROGRAM_GENERAL,
     LEX_ITEM_SUBPROGRAM_INIT,
     LEX_ITEM_SUBPROGRAM_FREE
};


enum lex_subprogram_argument_usage {
     LEX_SUBPROGRAM_ARGUMENT_USAGE_IN = 384,
     LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT,
     LEX_SUBPROGRAM_ARGUMENT_USAGE_IN_OUT
};


enum lex_subprogram_argument_option {
     LEX_SUBPROGRAM_ARGUMENT_OPTION_ENUM_EXTERNAL = 512,
     LEX_SUBPROGRAM_ARGUMENT_OPTION_ENUM_MASK,
     LEX_SUBPROGRAM_ARGUMENT_OPTION_ENUM_ARRAY,
     LEX_SUBPROGRAM_ARGUMENT_OPTION_LIST_SIZE
};


enum subprogram_type {
     SUBPROGRAM_TYPE_GENERAL,
     SUBPROGRAM_TYPE_INIT,
     SUBPROGRAM_TYPE_FREE
};


#define N_SUBPROGRAM_ARGUMENT_OPTIONS 4

enum subprogram_argument_option_mask {
     SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_EXTERNAL = (1<<0),
     SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_MASK     = (1<<1),
     SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY    = (1<<2),
     SUBPROGRAM_ARGUMENT_OPTION_MASK_LIST_SIZE     = (1<<3)
};


typedef struct {
     int type;
     char *name;
     int rank;
     char *dimens[MAX_DIMENS];
} type_data;


typedef struct {
     char *name;
     struct list_data *prev;
     struct list_data *next;

     int value;
} enum_member_data;


typedef struct {
     char *name;
     struct list_data *prev;
     struct list_data *next;

     enum_member_data *members;
} enumeration_data;


typedef struct {
     char *name;
     struct list_data *prev;
     struct list_data *next;

     type_data type;
     lex_type_data lex_type;
} global_const_data;


typedef struct {
     char *name;
     struct list_data *prev;
     struct list_data *next;

     size_t size;
} structure_data;


typedef struct {
     char *name;
     struct list_data *prev;
     struct list_data *next;

     int usage;
     type_data type;
     int options;
     char *enum_external_type;
     char *enum_external_class;
     char *enum_name_to_value;
} argument_data;


typedef struct {
     char *name;
     struct list_data *prev;
     struct list_data *next;

     type_data type;

     int has_return_value;
     int has_multi_dimen_args;

     int n_args;
     argument_data *args;
} subprogram_data;


typedef struct {
     char *include;
     char *prefix;
     char *PREFIX;
     enumeration_data enums;
     global_const_data consts;
     structure_data structs;
     subprogram_data subs_all;
     subprogram_data subs_general;
     subprogram_data subs_init;
     subprogram_data subs_free;
} bindx_data;


#define YY_DECL int yy_lex(locus_data *locus, lex_type_data *lex_type)


YY_DECL;

char *get_yytext();

#include "prototypes/bindx_parse_int_p.h"


#ifdef __cplusplus
}
#endif

#endif /* BINDX_INT_PARSE_H */
