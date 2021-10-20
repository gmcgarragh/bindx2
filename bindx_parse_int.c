/*******************************************************************************
**
**    Copyright (C) 2011-2018 Greg McGarragh <greg.mcgarragh@colostate.edu>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#include <gutil.h>
#include <gindex_name_value.h>

#include "bindx_parse.h"
#include "bindx_parse_int.h"
#include "bindx_util.h"
#include "bindx_yylex_int.h"


/*******************************************************************************
 *
 ******************************************************************************/
static const char *subprogram_argument_option_names[] = {
     "enum_external",
     "enum_mask",
     "enum_array",
     "list_size"
};


static long subprogram_argument_option_masks[] = {
     SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_EXTERNAL,
     SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_MASK,
     SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY,
     SUBPROGRAM_ARGUMENT_OPTION_MASK_LIST_SIZE
};


GINDEX_NAME_MASK_TEMPLATE_STATIC(subprogram_argument_option,
                                 "subprogram argument option",
                                 N_SUBPROGRAM_ARGUMENT_OPTIONS)


/*******************************************************************************
 *
 ******************************************************************************/
static void parse_char(locus_data *locus, char c)
{
     lex_type_data lex_type;

     if (yy_lex(locus, &lex_type) != c)
          parse_error(locus, "expected %c at %s", c, get_yytext());
}



static char *parse_string(locus_data *locus)
{
     lex_type_data lex_type;

     if (yy_lex(locus, &lex_type) != LEX_TYPE_STRING)
          parse_error(locus, "expected a string at %s", get_yytext());

     return lex_type.s;
}



static int parse_int(locus_data *locus)
{
     lex_type_data lex_type;

     if (yy_lex(locus, &lex_type) != LEX_TYPE_LONG)
          parse_error(locus, "expected an integer at %s", get_yytext());

     return lex_type.l;
}



static double parse_double(locus_data *locus)
{
     lex_type_data lex_type;

     if (yy_lex(locus, &lex_type) != LEX_TYPE_DOUBLE)
          parse_error(locus, "expected a double at %s", get_yytext());

     return lex_type.d;
}



static char *parse_identifier(locus_data *locus)
{
     lex_type_data lex_type;

     if (yy_lex(locus, &lex_type) != LEX_TYPE_IDENTIFIER)
          parse_error(locus, "expected an identifier at %s", get_yytext());

     return lex_type.s;
}



/*******************************************************************************
 *
 ******************************************************************************/
static type_data parse_type(locus_data *locus)
{
     int i;

     int r;

     lex_type_data lex_type;

     type_data type;

     r = yy_lex(locus, &lex_type);

     switch(r) {
          case LEX_BINDX_TYPE_VOID:
               type.type = LEX_BINDX_TYPE_VOID;
               type.name = "void";
               type.rank = parse_int(locus);
               break;
          case LEX_BINDX_TYPE_ENUM:
               type.type = LEX_BINDX_TYPE_ENUM;
               type.name = parse_identifier(locus);
               type.rank = parse_int(locus);
               break;
          case LEX_BINDX_TYPE_CHAR:
               type.type = LEX_BINDX_TYPE_CHAR;
               type.name = "char";
               type.rank = parse_int(locus);
               break;
          case LEX_BINDX_TYPE_INT:
               type.type = LEX_BINDX_TYPE_INT;
               type.name = "int";
               type.rank = parse_int(locus);
               break;
          case LEX_BINDX_TYPE_DOUBLE:
               type.type = LEX_BINDX_TYPE_DOUBLE;
               type.name = "double";
               type.rank = parse_int(locus);
               break;
          case LEX_TYPE_IDENTIFIER:
               type.type = LEX_BINDX_TYPE_STRUCTURE;
               type.name = lex_type.s;
               type.rank = parse_int(locus);
               break;
          default:
               parse_error(locus, "Invalid type specificaton: %s", get_yytext());
               break;
     }

     if (type.rank > 0) {
          for (i = 0; i < type.rank; ++i)
               type.dimens[i] = parse_string(locus);
     }

     return type;
}



/*******************************************************************************
 *
 ******************************************************************************/
static enum_member_data *parse_enum_member(locus_data *locus)
{
     enum_member_data *enum_member;

     enum_member = malloc(sizeof(enum_member_data));

     enum_member->name = parse_identifier(locus);
     parse_char(locus, '=');
     enum_member->value = parse_int(locus);

     return enum_member;
}



/*******************************************************************************
 *
 ******************************************************************************/
static enumeration_data *parse_enumeration(locus_data *locus)
{
     int r;

     enum_member_data *enum_member;

     enumeration_data *enumeration;

     lex_type_data lex_type;

     enumeration = malloc(sizeof(enumeration_data));

     enumeration->name = parse_identifier(locus);

     enumeration->members = malloc(sizeof(enum_member_data));
     list_init(enumeration->members);

     parse_char(locus, ',');

     do {
          enum_member = parse_enum_member(locus);

          if (list_append(enumeration->members, enum_member, 1) == NULL)
               parse_error(locus, "duplicate enumeration name: %s", enumeration->name);
     } while ((r = yy_lex(locus, &lex_type)) == ',');

     if (r != ';')
          parse_error(locus, "expected an \';\' at %s", get_yytext());

     return enumeration;
}



/*******************************************************************************
 *
 ******************************************************************************/
static global_const_data *parse_global_const(locus_data *locus)
{
     global_const_data *global_const;

     global_const = malloc(sizeof(global_const_data));

     global_const->type = parse_type(locus);
     global_const->name = parse_identifier(locus);
     parse_char(locus, '=');

     switch(global_const->type.type) {
     case LEX_BINDX_TYPE_INT:
          global_const->lex_type.l = parse_int(locus);
          break;
     case LEX_BINDX_TYPE_DOUBLE:
          global_const->lex_type.d = parse_double(locus);
          break;
     default:
          parse_error(locus, "Invalid lex_bindx_type value: %s", global_const->type.name);
          break;
     }

     parse_char(locus, ';');

     return global_const;
}



/*******************************************************************************
 *
 ******************************************************************************/
static structure_data *parse_structure(locus_data *locus)
{
     structure_data *structure;

     structure = malloc(sizeof(structure_data));

     structure->name = parse_identifier(locus);
     structure->size = parse_int(locus);

     parse_char(locus, ';');

     return structure;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int parse_usage(locus_data *locus)
{
     int r;

     lex_type_data lex_type;

     r = yy_lex(locus, &lex_type);

     switch(r) {
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_IN:
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT:
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_IN_OUT:
               break;
          default:
               parse_error(locus, "Invalid argument usage: %s", get_yytext());
               break;
     }

     return r;
}



/*******************************************************************************
 *
 ******************************************************************************/
static option_data parse_options(locus_data *locus, const char *delims, int *r, int flag)
{
     int i;
     int n;

     lex_type_data lex_type;

     option_data options;

     options.flags = 0;

     options.enum_external_type  = NULL;
     options.enum_external_class = NULL;
     options.enum_name_to_value  = NULL;
     options.enum_index_to_mask  = NULL;
     options.enum_index_to_name  = NULL;
     options.enum_value_to_name  = NULL;

     while (1) {
          *r = yy_lex(locus, &lex_type);

          n = strlen(delims);
          for (i = 0; i < n; ++i) {
               if (*r == delims[i])
                    goto L1;
          }

          switch(*r) {
               case LEX_SUBPROGRAM_ARGUMENT_OPTION_ENUM_EXTERNAL:
                    options.flags |= SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_EXTERNAL;
                    options.enum_external_type  = parse_string(locus);
                    options.enum_external_class = parse_string(locus);
                    break;
               case LEX_SUBPROGRAM_ARGUMENT_OPTION_ENUM_MASK:
                    options.flags |= SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_MASK;
                    if (! flag)
                         options.enum_name_to_value = parse_identifier(locus);
                    else {
                         options.enum_index_to_mask = parse_identifier(locus);
                         options.enum_index_to_name = parse_identifier(locus);
                    }
                    break;
               case LEX_SUBPROGRAM_ARGUMENT_OPTION_ENUM_ARRAY:
                    options.flags |= SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY;
                    options.enum_name_to_value = parse_identifier(locus);
                    break;
               case LEX_SUBPROGRAM_ARGUMENT_OPTION_LIST_SIZE:
                    options.flags |= SUBPROGRAM_ARGUMENT_OPTION_MASK_LIST_SIZE;
                    break;
               default:
                    parse_error(locus, "Invalid argument option: %s", get_yytext());
                    break;
          }
     }

L1:  return options;
}



/*******************************************************************************
 *
 ******************************************************************************/
static argument_data *parse_argument(locus_data *locus, const char *delims, int *r)
{
     argument_data *argument;

     argument = malloc(sizeof(argument_data));

     argument->type    = parse_type(locus);
     argument->name    = parse_identifier(locus);
     argument->usage   = parse_usage(locus);
     argument->options = parse_options(locus, delims, r, 0);

     return argument;
}



/*******************************************************************************
 *
 ******************************************************************************/
static subprogram_data *parse_subprogram(locus_data *locus)
{
     int r;

     char *delims = ",;";

     argument_data *argument;

     subprogram_data *subprogram;

     subprogram = malloc(sizeof(subprogram_data));

     subprogram->type             = parse_type(locus);
     subprogram->name             = parse_identifier(locus);
     subprogram->has_return_value = parse_int(locus);
     subprogram->options          = parse_options(locus, delims, &r, 1);

     subprogram->args = malloc(sizeof(argument_data));
     list_init(subprogram->args);

     subprogram->has_multi_dimen_args = 0;

     if (r == ';')
          ;
     else
     if (r == ',') {
          do {
               argument = parse_argument(locus, ",;", &r);

               if (list_append(subprogram->args, argument, 1) == NULL)
                    parse_error(locus, "duplicate argument name: %s", argument->name);

               if (r == ',')
                    continue;
               else
               if (r == ';')
                    break;
               else
                    parse_error(locus, "unexpected character at: %s", get_yytext());
          } while(1);

          list_for_each(subprogram->args, argument) {
               if (argument->type.rank > 1) {
                    subprogram->has_multi_dimen_args = 1;
                    break;
               }
          }
     }
     else
          parse_error(locus, "unexpected character at: %s", get_yytext());

     return subprogram;
}



static subprogram_data *subprogram_duplicate(subprogram_data *d)
{
     subprogram_data *subprogram;

     subprogram = malloc(sizeof(subprogram_data));

     *subprogram = *d;

     return subprogram;
}



int subprogram_n_in_args(subprogram_data *d)
{
     int i = 0;

     argument_data *argument;

     list_for_each(d->args, argument) {
          if (argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_IN)
               i++;
     }

     return i;
}



int subprogram_n_out_args(subprogram_data *d)
{
     int i = 0;

     argument_data *argument;

     list_for_each(d->args, argument) {
          if (argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT)
               i++;
     }

     return i;
}



int subprogram_n_scaler_in_args(subprogram_data *d)
{
     int i = 0;

     argument_data *argument;

     list_for_each(d->args, argument) {
          if (argument->type.rank == 0 &&
              argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_IN)
               i++;
     }

     return i;
}



int subprogram_n_scaler_out_args(subprogram_data *d)
{
     int i = 0;

     argument_data *argument;

     list_for_each(d->args, argument) {
          if (argument->type.rank == 0 &&
              argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT)
               i++;
     }

     return i;
}


/*******************************************************************************
 *
 ******************************************************************************/
void bindx_init(bindx_data *d)
{
     d->include = NULL;

     list_init(&d->enums);
     list_init(&d->consts);
     list_init(&d->structs);
     list_init(&d->subs_all);
     list_init(&d->subs_general);
     list_init(&d->subs_init);
     list_init(&d->subs_free);
}



/*******************************************************************************
 *
 ******************************************************************************/
void bindx_parse(bindx_data *d, locus_data *locus)
{
     int r;

     lex_type_data lex_type;

     enumeration_data *enumeration;
     global_const_data *global_const;
     structure_data *structure;
     subprogram_data *subprogram;

     while ((r = yy_lex(locus, &lex_type))) {
          switch(r) {
               case LEX_ITEM_PREFIX:
                    d->prefix = parse_identifier(locus);
                    d->PREFIX = strdup(d->prefix);
                    strtoupper(d->PREFIX, d->PREFIX);
                    parse_char(locus, ';');
                    break;
               case LEX_ITEM_INCLUDE:
                    d->include = parse_string(locus);
                    parse_char(locus, ';');
                    break;
               case LEX_ITEM_ENUMERATION:
                    enumeration = parse_enumeration(locus);
                    if (list_append(&d->enums, enumeration, 1) == NULL)
                         parse_error(locus, "duplicate enumeration name: %s", enumeration->name);
                    break;
               case LEX_ITEM_GLOBAL_CONST:
                    global_const = parse_global_const(locus);
                    if (list_append(&d->consts, global_const, 1) == NULL)
                         parse_error(locus, "duplicate global_const name: %s", global_const->name);
                    break;
               case LEX_ITEM_STRUCTURE:
                    structure = parse_structure(locus);
                    if (list_append(&d->structs, structure, 1) == NULL)
                         parse_error(locus, "duplicate structure name: %s", structure->name);
                    break;
               case LEX_ITEM_SUBPROGRAM_GENERAL:
                    subprogram = parse_subprogram(locus);
                    if (list_append(&d->subs_general, subprogram, 1) == NULL)
                         parse_error(locus, "duplicate subprogram name: %s", subprogram->name);
                    if (list_append(&d->subs_all, subprogram_duplicate(subprogram), 1) == NULL)
                         parse_error(locus, "duplicate subprogram name: %s", subprogram->name);
                    break;
               case LEX_ITEM_SUBPROGRAM_INIT:
                    subprogram = parse_subprogram(locus);
                    if (! list_is_empty(&d->subs_init))
                         parse_error(locus, "more than one init subprogram defined: %s", subprogram->name);
                    list_append(&d->subs_init, subprogram, 1);
                    if (list_append(&d->subs_all, subprogram_duplicate(subprogram), 1) == NULL)
                         parse_error(locus, "duplicate subprogram name: %s", subprogram->name);
                    break;
               case LEX_ITEM_SUBPROGRAM_FREE:
                    subprogram = parse_subprogram(locus);
                    if (! list_is_empty(&d->subs_free))
                         parse_error(locus, "more than one free subprogram defined: %s", subprogram->name);
                    list_append(&d->subs_free, subprogram, 1);
                    if (list_append(&d->subs_all, subprogram_duplicate(subprogram), 1) == NULL)
                         parse_error(locus, "duplicate subprogram name: %s", subprogram->name);
                    break;
               default:
                    parse_error(locus, "Invalid interface item: %s", get_yytext());
                    break;
          }
     }
}



/*******************************************************************************
 *
 ******************************************************************************/
void bindx_finialize(bindx_data *d)
{
     if (list_is_empty(&d->subs_init)) {
          fprintf(stderr, "ERROR: An init subprogram has not been defined");
          exit(1);
     }
     if (list_is_empty(&d->subs_free)) {
          fprintf(stderr, "ERROR: An free subprogram has not been defined");
          exit(1);
     }
}



/*******************************************************************************
 *
 ******************************************************************************/
static void free_enum_member(enum_member_data *d)
{
     free(d->name);
}



/*******************************************************************************
 *
 ******************************************************************************/
static void free_enumeration(enumeration_data *d)
{
     enum_member_data *enum_member;

     free(d->name);

     list_for_each(d->members, enum_member)
          free_enum_member(enum_member);
     list_free(d->members);
     free(d->members);
}



/*******************************************************************************
 *
 ******************************************************************************/
static void free_global_const(global_const_data *d)
{
     free(d->name);
}



/*******************************************************************************
 *
 ******************************************************************************/
static void free_structure(structure_data *d)
{
     free(d->name);
}



/*******************************************************************************
 *
 ******************************************************************************/
static void free_type(type_data *d)
{
     switch(d->type) {
          case LEX_BINDX_TYPE_VOID:
               break;
          case LEX_BINDX_TYPE_ENUM:
               free(d->name);
          case LEX_BINDX_TYPE_CHAR:
               break;
          case LEX_BINDX_TYPE_INT:
               break;
          case LEX_BINDX_TYPE_DOUBLE:
               break;
          case LEX_BINDX_TYPE_STRUCTURE:
               free(d->name);
               break;
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }
}



/*******************************************************************************
 *
 ******************************************************************************/
static void free_argument(argument_data *d)
{
     int i;

     free(d->name);
     free_type(&d->type);

     if (d->type.rank > 1) {
          for (i = 0; i < d->type.rank; ++i)
               free(d->type.dimens[i]);
     }

     free(d->options.enum_name_to_value);
     free(d->options.enum_external_type);
     free(d->options.enum_external_class);
}



/*******************************************************************************
 *
 ******************************************************************************/
static void free_subprogram(subprogram_data *d)
{
     argument_data *argument;

     free(d->name);
     free_type(&d->type);

     list_for_each(d->args, argument)
          free_argument(argument);
     list_free(d->args);
     free(d->args);
}



/*******************************************************************************
 *
 ******************************************************************************/
void bindx_free(bindx_data *d)
{
     enumeration_data *enumeration;
     global_const_data *global_const;
     structure_data *structure;
     subprogram_data *subprogram;

     free(d->prefix);
     free(d->PREFIX);

     free(d->include);

     list_for_each(&d->enums, enumeration)
          free_enumeration(enumeration);
     list_free(&d->enums);

     list_for_each(&d->consts, global_const)
          free_global_const(global_const);
     list_free(&d->consts);

     list_for_each(&d->structs, structure)
          free_structure(structure);
     list_free(&d->structs);

     list_for_each(&d->subs_all, subprogram)
          free_subprogram(subprogram);
     list_free(&d->subs_all);

     list_free(&d->subs_general);
     list_free(&d->subs_init);
     list_free(&d->subs_free);
}



/*******************************************************************************
 *
 ******************************************************************************/
static int write_type(FILE *fp, const type_data *d)
{
     int i;

     switch(d->type) {
          case LEX_BINDX_TYPE_VOID:
               break;
          case LEX_BINDX_TYPE_ENUM:
               fprintf(fp, "enum ");
               break;
          case LEX_BINDX_TYPE_CHAR:
               break;
          case LEX_BINDX_TYPE_INT:
               break;
          case LEX_BINDX_TYPE_DOUBLE:
               break;
          case LEX_BINDX_TYPE_STRUCTURE:
               break;
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     fprintf(fp, "%s ", d->name);
     fprintf(fp, "%d",  d->rank);

     if (d->rank > 0) {
          for (i = 0; i < d->rank; ++i)
               fprintf(fp, " \"%s\"", d->dimens[i]);
     }

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int write_enum_member(FILE *fp, const enum_member_data *d)
{
     fprintf(fp, "%s = %d", d->name, d->value);

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int write_enumeration(FILE *fp, const enumeration_data *d)
{
     char *indent = "     ";

     enum_member_data *enum_member;

     fprintf(fp, "enumeration ");

     fprintf(fp, "%s,\n", d->name);

     list_for_each(d->members, enum_member) {
          fprintf(fp, "%s", indent);
          write_enum_member(fp, enum_member);

          if (! list_is_last_elem(d->members, enum_member))
               fprintf(fp, ",\n");
     }

     fprintf(fp, ";\n");

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int write_global_const(FILE *fp, const global_const_data *d)
{
     fprintf(fp, "global_const ");
     write_type(fp, &d->type);
     fprintf(fp, " %s = ", d->name);
     switch(d->type.type) {
     case LEX_BINDX_TYPE_INT:
          fprintf(fp, "%ld", d->lex_type.l);
          break;
     case LEX_BINDX_TYPE_DOUBLE:
          fprintf(fp, "%f",  d->lex_type.d);
          break;
     default:
          INTERNAL_ERROR("Invalid lex_bindx_type value: %s", d->type.name);
          break;
     }
     fprintf(fp, ";\n");

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int write_structure(FILE *fp, const structure_data *d)
{
     fprintf(fp, "structure %s %lu;\n", d->name, d->size);

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int write_usage(FILE *fp, int usage)
{
     switch(usage) {
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_IN:
               fprintf(fp, "in");
               break;
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT:
               fprintf(fp, "out");
               break;
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_IN_OUT:
               fprintf(fp, "in_out");
               break;
          default:
               INTERNAL_ERROR("Invalid lex_subprogram_argument_usage: %d", usage);
               break;
     }

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int write_options(FILE *fp, const option_data *d, int flag)
{
     int i;
     int n;

     long options[N_SUBPROGRAM_ARGUMENT_OPTIONS];

     n = subprogram_argument_option_mask_to_value_list(d->flags, options,
                                                       N_SUBPROGRAM_ARGUMENT_OPTIONS);

     for (i = 0; i < n; ++i) {
          switch(options[i]) {
               case SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_EXTERNAL:
                    fprintf(fp, " enum_external %s %s", d->enum_external_type,
                            d->enum_external_class);
                    break;
               case SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_MASK:
                    if (! flag)
                         fprintf(fp, " enum_mask %s", d->enum_name_to_value);
                    else
                         fprintf(fp, " enum_mask %s %s", d->enum_index_to_mask,
                                                         d->enum_index_to_name);
                    break;
               case SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY:
                    fprintf(fp, " enum_array %s", d->enum_name_to_value);
                    break;
               case SUBPROGRAM_ARGUMENT_OPTION_MASK_LIST_SIZE:
                    fprintf(fp, " list_size");
                    break;
               default:
                    INTERNAL_ERROR("Invalid subprogram_argument_option_mask: %d",
                                   options[i]);
                    break;
          }
     }

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int write_argument(FILE *fp, const argument_data *d)
{
     write_type(fp, &d->type);
     fprintf(fp, " ");

     fprintf(fp, "%s ", d->name);
     write_usage(fp, d->usage);

     write_options(fp, &d->options, 0);

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int write_subprogram(FILE *fp, const subprogram_data *d, const char *tag)
{
     argument_data *argument;

     fprintf(fp, "subprogram%s ", tag);

     write_type(fp, &d->type);
     fprintf(fp, " ");
     fprintf(fp, "%s ", d->name);
     fprintf(fp, "%d", d->has_return_value);

     write_options(fp, &d->options, 1);

     if (! list_is_empty(d->args))
          fprintf(fp, ", ");

     list_for_each(d->args, argument) {
          write_argument(fp, argument);

          if (! list_is_last_elem(d->args, argument))
               fprintf(fp, ", ");
     }

     fprintf(fp, ";");

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
int bindx_write(FILE *fp, const bindx_data *d)
{
     enumeration_data *enumeration;
     global_const_data *global_const;
     structure_data *structure;
     subprogram_data *subprogram;

     fprintf(fp, "prefix %s;\n", d->prefix);
     fprintf(fp, "\n");

     list_for_each(&d->enums, enumeration) {
          write_enumeration(fp, enumeration);
          fprintf(fp, "\n");
     }

     list_for_each(&d->consts, global_const) {
          write_global_const(fp, global_const);
          fprintf(fp, "\n");
     }

     list_for_each(&d->structs, structure) {
          write_structure(fp, structure);
          fprintf(fp, "\n");
     }

     list_for_each(&d->subs_init, subprogram) {
          write_subprogram(fp, subprogram, "_init");
          fprintf(fp, "\n");
     }

     list_for_each(&d->subs_free, subprogram) {
          write_subprogram(fp, subprogram, "_free");
          fprintf(fp, "\n");
     }

     list_for_each(&d->subs_general, subprogram) {
          write_subprogram(fp, subprogram, "_general");
          fprintf(fp, "\n");
     }

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
int min_argument_rank(argument_data *args)
{
     int min = INT_MAX;

     argument_data *argument;

     list_for_each(args, argument) {
          if (argument->type.rank < min)
               min = argument->type.rank;
     }

     return min;
}



int max_argument_rank(argument_data *args)
{
     int max = 0;

     argument_data *argument;

     list_for_each(args, argument) {
          if (argument->type.rank > max)
               max = argument->type.rank;
     }

     return max;
}
