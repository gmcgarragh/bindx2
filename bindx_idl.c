/*******************************************************************************
**
**    Copyright (C) 2011-2018 Greg McGarragh <greg.mcgarragh@colostate.edu>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#include <gutil.h>

#include "bindx_c.h"
#include "bindx_idl.h"
#include "bindx_parse.h"
#include "bindx_parse_int.h"
#include "bindx_util.h"


static const char *get_idl_type(const type_data *d)
{
     switch(d->type) {
          case LEX_BINDX_TYPE_ENUM:
               return "IDL_TYP_LONG";
/*
          case LEX_BINDX_TYPE_CHAR:
               break;
*/
          case LEX_BINDX_TYPE_INT:
               return "IDL_TYP_LONG";
          case LEX_BINDX_TYPE_DOUBLE:
               return "IDL_TYP_DOUBLE";
/*
          case LEX_BINDX_TYPE_STRUCTURE:
               break;
*/
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return 0;
}



static const char *get_idl_type_alt(const type_data *d)
{
     switch(d->type) {
/*
          case LEX_BINDX_TYPE_ENUM:
               break;
          case LEX_BINDX_TYPE_CHAR:
               break;
*/
          case LEX_BINDX_TYPE_INT:
               return "IDL_TYP_INT";
          case LEX_BINDX_TYPE_DOUBLE:
               return "IDL_TYP_FLOAT";
/*
          case LEX_BINDX_TYPE_STRUCTURE:
               break;
*/
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return 0;
}



static const char *get_idl_value_var(const type_data *d)
{
     switch(d->type) {
          case LEX_BINDX_TYPE_ENUM:
               return "l";
/*
          case LEX_BINDX_TYPE_CHAR:
               break;
*/
          case LEX_BINDX_TYPE_INT:
               return "l";
          case LEX_BINDX_TYPE_DOUBLE:
               return "d";
/*
          case LEX_BINDX_TYPE_STRUCTURE:
               break;
*/
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return 0;
}



static const char *get_idl_value_var_alt(const type_data *d)
{
     switch(d->type) {
/*
          case LEX_BINDX_TYPE_ENUM:
               break;
          case LEX_BINDX_TYPE_CHAR:
               break;
*/
          case LEX_BINDX_TYPE_INT:
               return "i";
          case LEX_BINDX_TYPE_DOUBLE:
               return "f";
/*
          case LEX_BINDX_TYPE_STRUCTURE:
               break;
*/
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return 0;
}



static const char *get_idl_type_name(const type_data *d)
{
     switch(d->type) {
/*
          case LEX_BINDX_TYPE_ENUM:
               break;
          case LEX_BINDX_TYPE_CHAR:
               break;
*/
          case LEX_BINDX_TYPE_INT:
               return "long";
          case LEX_BINDX_TYPE_DOUBLE:
               return "double";
/*
          case LEX_BINDX_TYPE_STRUCTURE:
               break;
*/
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return 0;
}



static const char *get_idl_type_name_alt(const type_data *d)
{
     switch(d->type) {
/*
          case LEX_BINDX_TYPE_ENUM:
               break;
          case LEX_BINDX_TYPE_CHAR:
               break;
*/
          case LEX_BINDX_TYPE_INT:
               return "int";
          case LEX_BINDX_TYPE_DOUBLE:
               return "float";
/*
          case LEX_BINDX_TYPE_STRUCTURE:
               break;
*/
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return 0;
}



static int write_dlm_header_top(FILE *fp)
{
     fprintf(fp, "#*******************************************************************************\n");
     fprintf(fp, "# This file was generated by bindx version %s.  Edit at your own risk.\n", bindx_version());
     fprintf(fp, "#*******************************************************************************\n");

     return 0;
}



static int write_prototypes(FILE *fp, const bindx_data *d,
                            const subprogram_data *subs, int indent)
{
     subprogram_data *subprogram;

     list_for_each(subs, subprogram)
          fprintf(fp, "%svoid IDL_CDECL %s_%s_dlm(int argc, IDL_VPTR argv[], char *argk);\n",
                  bxis(indent), d->prefix, subprogram->name);

     return 0;
}



static int write_idl_sysfun_defs(FILE *fp, const bindx_data *d,
                                 const subprogram_data *subs, int indent,
                                 int for_version_5_3)
{
     char PREFIX[NM];
     char NAME_ [NM];

     int count;

     argument_data *argument;
     subprogram_data *subprogram;

     strtoupper(d->prefix, PREFIX);

     list_for_each(subs, subprogram) {
          strtoupper(subprogram->name, NAME_);
          count = 1;
          list_for_each(subprogram->args, argument) {
               if (! (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_LIST_SIZE))
                    count++;
          }

          if (subprogram->has_return_value)
               count++;

          if (! for_version_5_3)
               fprintf(fp, "%s{{(IDL_FUN_RET) %s_%s_dlm}, \"%s_%s\", %d, %d, 0}",
                       bxis(indent), d->prefix, subprogram->name, PREFIX, NAME_,
                       count, count);
          else
               fprintf(fp, "%s{{(IDL_FUN_RET) %s_%s_dlm}, \"%s_%s\", %d, %d, 0, 0}",
                       bxis(indent), d->prefix, subprogram->name, PREFIX, NAME_,
                       count, count);
          if (! list_is_last_elem(subs, subprogram))
               fprintf(fp, ",");
          fprintf(fp, "\n");
    }

     return 0;
}



static int write_parse_argument_enum_scalar(FILE *fp, int indent, int i_arg,
                                            argument_data *argument)
{
     fprintf(fp, "%sIDL_ENSURE_SCALAR(argv[%d]);\n", bxis(indent), i_arg);
     fprintf(fp, "%sif (argv[%d]->type != IDL_TYP_STRING)\n", bxis(indent), i_arg);
          fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s must be of type string\");\n", bxis(indent + 1), argument->name);
     fprintf(fp, "%sif ((r = %s(IDL_STRING_STR(&argv[%d]->value.str))) == -1)\n", bxis(indent), argument->options.enum_name_to_value, i_arg);
          fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s()\");\n", bxis(indent + 1), argument->options.enum_name_to_value);
     fprintf(fp, "%s%s = r;\n", bxis(indent), argument->name);

     return 0;
}



static int write_parse_argument_enum_mask_scalar(FILE *fp, int indent, int i_arg,
                                                 argument_data *argument)
{
     fprintf(fp, "%s{\n", bxis(indent));
     fprintf(fp, "%sint i;\n", bxis(indent));
     fprintf(fp, "%s%s = 0;\n", bxis(indent), argument->name);
     fprintf(fp, "%sIDL_ENSURE_ARRAY(argv[%d]);\n", bxis(indent), i_arg);
     fprintf(fp, "%sfor (i = 0; i < argv[%d]->value.arr->n_elts; ++i) {\n", bxis(indent), i_arg);
          fprintf(fp, "%sif ((r = %s(((IDL_STRING *) argv[%d]->value.arr->data)[i].s)) == -1)\n", bxis(indent + 1), argument->options.enum_name_to_value, i_arg);
               fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s()\");\n", bxis(indent + 2), argument->options.enum_name_to_value);
          fprintf(fp, "%s%s |= r;\n", bxis(indent + 1), argument->name);
     fprintf(fp, "%s}}\n", bxis(indent));

     return 0;
}



static int write_parse_argument_number_scalar(FILE *fp, int indent, int i_arg,
                                              argument_data *argument)
{
     fprintf(fp, "%sIDL_ENSURE_SCALAR(argv[%d]);\n", bxis(indent), i_arg);
     fprintf(fp, "%sif (argv[%d]->type != %s && argv[%d]->type != %s)\n", bxis(indent), i_arg, get_idl_type(&argument->type), i_arg, get_idl_type_alt(&argument->type));
          fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s must be of type %s or %s\");\n", bxis(indent + 1), argument->name, get_idl_type_name(&argument->type), get_idl_type_name_alt(&argument->type));
     fprintf(fp, "%sif (argv[%d]->type == %s)\n", bxis(indent), i_arg, get_idl_type(&argument->type));
          fprintf(fp, "%s%s = argv[%d]->value.%s;\n", bxis(indent + 1), argument->name, i_arg, get_idl_value_var    (&argument->type));
     fprintf(fp, "%selse\n", bxis(indent));
          fprintf(fp, "%s%s = argv[%d]->value.%s;\n", bxis(indent + 1), argument->name, i_arg, get_idl_value_var_alt(&argument->type));

     return 0;
}



static int write_parse_argument_enum_array(FILE *fp, int indent, int i_arg,
                                           argument_data *argument)
{
     fprintf(fp, "%s{\n", bxis(indent));
     fprintf(fp, "%sint i;\n", bxis(indent));
     fprintf(fp, "%sIDL_ENSURE_ARRAY(argv[%d]);\n", bxis(indent), i_arg);
     fprintf(fp, "%sn_%s = argv[%d]->value.arr->n_elts;\n", bxis(indent), argument->name, i_arg);
     fprintf(fp, "%s%s = malloc(n_%s * sizeof(int));\n", bxis(indent), argument->name, argument->name);
     fprintf(fp, "%sfor (i = 0; i < argv[%d]->value.arr->n_elts; ++i) {\n", bxis(indent), i_arg);
          fprintf(fp, "%sif ((r = %s(((IDL_STRING *) argv[%d]->value.arr->data)[i].s)) == -1)\n", bxis(indent + 1), argument->options.enum_name_to_value, i_arg);
               fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s()\");\n", bxis(indent + 2), argument->options.enum_name_to_value);
          fprintf(fp, "%s%s[i] = r;\n", bxis(indent + 1), argument->name);
     fprintf(fp, "%s}}\n", bxis(indent));

     return 0;
}



static int write_parse_argument_number_array(FILE *fp, const bindx_data *d,
                                             int indent, int i_arg,
                                             argument_data *argument)
{
     int j;

     if (argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_IN) {
          fprintf(fp, "%sIDL_ENSURE_ARRAY(argv[%d]);\n", bxis(indent), i_arg);
          fprintf(fp, "%sif (argv[%d]->type != %s)\n", bxis(indent), i_arg, get_idl_type(&argument->type));
               fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s must be of type %s\");\n", bxis(indent + 1), argument->name, get_idl_type_name(&argument->type));
          fprintf(fp, "%sif (argv[%d]->value.arr->n_dim != %d)\n", bxis(indent), i_arg, argument->type.rank);
               fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s must be an array with 2 dimensions\");\n", bxis(indent + 1), argument->name);

          for (j = 0; j < argument->type.rank; ++j) {
               fprintf(fp, "%sif (argv[%d]->value.arr->dim[%d] != (%s))\n", bxis(indent), i_arg, argument->type.rank - j - 1, argument->type.dimens[j]);
                    fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s dimension %d must have %s elements\");\n", bxis(indent + 1), argument->name, argument->type.rank - j, argument->type.dimens[j]);
          }

          if (argument->type.rank == 1) {
               fprintf(fp, "%s%s = (", bxis(indent), argument->name);
               bindx_write_c_declaration(fp, d, &argument->type, NULL);
               fprintf(fp, ") argv[%d]->value.arr->data;\n", i_arg);
          }
          else {
               for (j = 0; j < argument->type.rank; ++j)
                    fprintf(fp, "%sdim[%d] = %s;\n", bxis(indent), j, argument->type.dimens[j]);
               fprintf(fp, "%s%s = (", bxis(indent), argument->name);
               bindx_write_c_declaration(fp, d, &argument->type, NULL);
               fprintf(fp, ") array_from_mem((void *) argv[%d]->value.arr->data, %d, dim, sizeof(", i_arg, argument->type.rank);
               bindx_write_c_type(fp, d, &argument->type, NULL);
               fprintf(fp, "), 1);\n");
          }
     }
     else {
          if (argument->type.rank == 1)
               fprintf(fp, "%s%s = malloc(", bxis(indent), argument->name);
          else
               fprintf(fp, "%s%s_data = malloc(", bxis(indent), argument->name);

          for (j = 0; j < argument->type.rank; ++j)
               fprintf(fp, "(%s) * ", argument->type.dimens[j]);
               fprintf(fp, "sizeof(");
               bindx_write_c_type(fp, d, &argument->type, NULL);
               fprintf(fp, "));\n");

          if (argument->type.rank > 1) {
               for (j = 0; j < argument->type.rank; ++j)
                    fprintf(fp, "%sdim[%d] = %s;\n", bxis(indent), j, argument->type.dimens[j]);
               fprintf(fp, "%s%s = (", bxis(indent), argument->name);
               bindx_write_c_declaration(fp, d, &argument->type, NULL);
               fprintf(fp, ") array_from_mem(%s_data, %d, dim, sizeof(double), 1);\n", argument->name, argument->type.rank);
          }
     }

     return 0;
}



static int write_subprograms(FILE *fp, const bindx_data *d,
                             enum subprogram_type sub_type,
                             const subprogram_data *subs, int indent)
{
     int i;
     int j;

     argument_data *argument;
     subprogram_data *subprogram;

     list_for_each(subs, subprogram) {
          fprintf(fp, "%svoid IDL_CDECL %s_%s_dlm(int argc, IDL_VPTR argv[], char *argk)\n",
                  bxis(indent), d->prefix, subprogram->name);
          fprintf(fp, "{\n");

          indent++;

          fprintf(fp, "%s", bxis(indent));
          bindx_write_c_type(fp, d, &subprogram->type, NULL);
          fprintf(fp, " r;\n");

          if (max_argument_rank(subprogram->args) > 1)
               fprintf(fp, "%ssize_t dim[%d];\n", bxis(indent), MAX_DIMENS);

          fprintf(fp, "%s%s_data *d;\n", bxis(indent), d->prefix);

          if (subprogram->has_return_value)
               fprintf(fp, "%sIDL_VARIABLE var;\n", bxis(indent));
          else {
               list_for_each(subprogram->args, argument) {
                    if (argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT &&
                        argument->type.rank == 0) {
                         fprintf(fp, "%sIDL_VARIABLE var;\n", bxis(indent));
                         break;
                    }
               }
          }

          if (sub_type == SUBPROGRAM_TYPE_INIT) {
               fprintf(fp, "%sIDL_VPTR ptr;\n", bxis(indent));
               fprintf(fp, "%sIDL_MEMINT dim_idl[%d];\n", bxis(indent), MAX_DIMENS);
          }
          else {
               list_for_each(subprogram->args, argument) {
                    if (argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT &&
                        argument->type.rank > 0) {
                         fprintf(fp, "%sIDL_VPTR ptr;\n", bxis(indent));
                         fprintf(fp, "%sIDL_MEMINT dim_idl[%d];\n", bxis(indent), MAX_DIMENS);
                         break;
                    }
               }
          }

          list_for_each(subprogram->args, argument) {
               fprintf(fp, "%s", bxis(indent));
               bindx_write_c_declaration(fp, d, &argument->type, NULL);
               fprintf(fp, "%s;\n", argument->name);

               if (argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT  &&
                   argument->type.rank > 1)
                    fprintf(fp, "%svoid *%s_data;\n", bxis(indent), argument->name);

               i++;
          }

          if (sub_type != SUBPROGRAM_TYPE_INIT) {
               fprintf(fp, "%sIDL_ENSURE_ARRAY(argv[0]);\n", bxis(indent));
               fprintf(fp, "%sif (argv[0]->type != IDL_TYP_BYTE)\n", bxis(indent));
               fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: Invalid %s instance\");\n", bxis(indent + 1), d->prefix);
               fprintf(fp, "%sd = (%s_data *) argv[0]->value.arr->data;\n", bxis(indent), d->prefix);
          }

          i = 1;
          list_for_each(subprogram->args, argument) {
               if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_LIST_SIZE)
                    continue;

               if (argument->type.rank == 0) {
                    if (argument->type.type == LEX_BINDX_TYPE_ENUM)
                         write_parse_argument_enum_scalar(fp, indent, i, argument);
                    else
                    if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_MASK)
                         write_parse_argument_enum_mask_scalar(fp, indent, i, argument);
                    else {
                         switch(argument->type.type) {
/*
                              case LEX_BINDX_TYPE_ENUM:
                                   break;
                              case LEX_BINDX_TYPE_CHAR:
                                   break;
*/
                              case LEX_BINDX_TYPE_INT:
                              case LEX_BINDX_TYPE_DOUBLE:
                                   write_parse_argument_number_scalar(fp, indent, i, argument);
                                   break;
/*
                              case LEX_BINDX_TYPE_STRUCTURE:
                                   break;
*/
                              default:
                                   INTERNAL_ERROR("Invalid lex_bindx_type value: %d",
                                                  argument->type.type);
                                   break;
                         }
                    }
               }
               else {
                    if (argument->type.type == LEX_BINDX_TYPE_ENUM &&
                        argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY)
                         write_parse_argument_enum_array(fp, indent, i, argument);
                    else {
                         switch(argument->type.type) {
/*
                              case LEX_BINDX_TYPE_ENUM:
                                   break;
                              case LEX_BINDX_TYPE_CHAR:
                                   break;
*/
                              case LEX_BINDX_TYPE_INT:
                              case LEX_BINDX_TYPE_DOUBLE:
                                   write_parse_argument_number_array(fp, d, indent, i, argument);
                                   break;
/*
                              case LEX_BINDX_TYPE_STRUCTURE:
                                   break;
*/
                              default:
                                   INTERNAL_ERROR("Invalid lex_bindx_type value: %d",
                                                  argument->type.type);
                                   break;
                         }
                    }
               }

               i++;
          }

          if (sub_type == SUBPROGRAM_TYPE_INIT)
               fprintf(fp, "%sd = malloc(sizeof(%s_data));\n", bxis(indent), d->prefix);

          fprintf(fp, "%sr = %s_%s(d", bxis(indent), d->prefix, subprogram->name);

          list_for_each(subprogram->args, argument) {
               fprintf(fp, ", ");
               if (argument->type.type == LEX_BINDX_TYPE_ENUM) {
                    fprintf(fp, "(enum %s_%s", d->prefix, argument->type.name);
                    if (argument->type.rank > 0)
                         fprintf(fp, " ");
                    bindx_write_c_dimens_args(fp, d, &argument->type, argument->usage);
                    fprintf(fp, ") ");
               }

               if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_MASK ||
                   argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY)
                    fprintf(fp,  "%s", argument->name);
               else
               if (argument->type.rank == 0  &&
                   argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_IN)
                    fprintf(fp,  "%s", argument->name);
               else
               if (argument->type.rank == 0  &&
                   argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT)
                    fprintf(fp, "&%s", argument->name);
               else
                    fprintf(fp,  "%s", argument->name);
          }

          fprintf(fp, ");\n");

          list_for_each(subprogram->args, argument) {
               if (argument->type.rank > 1)
                    fprintf(fp, "%sfree_array((void *) %s, %d);\n", bxis(indent), argument->name, argument->type.rank);
          }

          fprintf(fp, "%sif (%s)\n", bxis(indent), xrtm_error_conditional(subprogram->type.type));
          fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s_%s()\");\n", bxis(indent + 1), d->prefix, subprogram->name);

          list_for_each(subprogram->args, argument) {
               if (argument->type.type == LEX_BINDX_TYPE_ENUM &&
                   argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY)
                    fprintf(fp, "%sfree(%s);\n", bxis(indent), argument->name);
          }

          if (sub_type == SUBPROGRAM_TYPE_INIT) {
               fprintf(fp, "%sdim_idl[0] = sizeof(%s_data);\n", bxis(indent), d->prefix);
               fprintf(fp, "%sptr = IDL_ImportArray(1, (IDL_MEMINT *) dim_idl, IDL_TYP_BYTE, (UCHAR *) d, 0, NULL);\n", bxis(indent));
               fprintf(fp, "%sIDL_VarCopy(ptr, argv[0]);\n", bxis(indent));
          }
          else {
               i = 1;
               list_for_each(subprogram->args, argument) {
                    if (argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT) {
                         if (argument->type.rank == 0) {
                              fprintf(fp, "%svar.type = %s;\n", bxis(indent), get_idl_type(&argument->type));
                              fprintf(fp, "%svar.flags = 0;\n", bxis(indent));
                              fprintf(fp, "%svar.value.%s = r;\n", bxis(indent), get_idl_value_var(&argument->type));
                              fprintf(fp, "%sif (var.value.d == XRTM_DBL_ERROR)\n", bxis(indent));
                                   fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s_%s()\");\n", bxis(indent), d->prefix, subprogram->name);
                              fprintf(fp, "%sIDL_VarCopy((IDL_VPTR) &var, argv[%d]);\n", bxis(indent), i);
                         }
                         else {
                              for (j = 0; j < argument->type.rank; ++j)
                                   fprintf(fp, "%sdim_idl[%d] = %s;\n", bxis(indent), argument->type.rank - j - 1, argument->type.dimens[j]);
                              if (argument->type.rank == 1)
                                   fprintf(fp, "%sptr = IDL_ImportArray(%d, (IDL_MEMINT *) dim_idl, %s, (UCHAR *) %s, 0, NULL);\n", bxis(indent), argument->type.rank, get_idl_type(&argument->type), argument->name);
                              else
                                   fprintf(fp, "%sptr = IDL_ImportArray(%d, (IDL_MEMINT *) dim_idl, %s, (UCHAR *) %s_data, 0, NULL);\n", bxis(indent), argument->type.rank, get_idl_type(&argument->type), argument->name);
                              fprintf(fp, "%sIDL_VarCopy((IDL_VPTR) ptr, argv[%d]);\n", bxis(indent), i);
                         }
                    }

                    i++;
               }
          }

          if (subprogram->has_return_value) {
               if (subprogram->type.rank == 0) {
                    fprintf(fp, "%svar.type = %s;\n", bxis(indent), get_idl_type(&subprogram->type));
                    fprintf(fp, "%svar.flags = 0;\n", bxis(indent));
                    fprintf(fp, "%svar.value.%s = r;\n", bxis(indent), get_idl_value_var(&subprogram->type));
                    fprintf(fp, "%sif (var.value.d == XRTM_DBL_ERROR)\n", bxis(indent));
                         fprintf(fp, "%sIDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, \"ERROR: %s_%s()\");\n", bxis(indent), d->prefix, subprogram->name);
                    fprintf(fp, "%sIDL_VarCopy((IDL_VPTR) &var, argv[%d]);\n", bxis(indent), i);
               }
               else {

               }
          }

          fprintf(fp, "%sreturn;\n", bxis(indent));

          indent--;

          fprintf(fp, "}\n");
          fprintf(fp, "\n");
          fprintf(fp, "\n");
     }

     return 0;
}



static int write_dlm_procedures(FILE *fp, const bindx_data *d,
                                const subprogram_data *subs)
{
     char PREFIX[NM];
     char NAME_ [NM];

     int count;

     argument_data *argument;
     subprogram_data *subprogram;

     strtoupper(d->prefix, PREFIX);

     list_for_each(subs, subprogram) {
          strtoupper(subprogram->name, NAME_);
          count = 1;
          list_for_each(subprogram->args, argument) {
               if (! (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_LIST_SIZE))
                    count++;
          }

          if (subprogram->has_return_value)
               count++;

          fprintf(fp, "PROCEDURE   %s_%s %d %d\n", PREFIX, NAME_, count, count);
     }

     return 0;
}



int bindx_write_idl(FILE **fp, const bindx_data *d, const char *name)
{
     bindx_write_c_header_top(fp[0]);
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#include <gutil.h>\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#include <export.h>\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#include <%s_interface.h>\n", d->prefix);
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "int  %s_int_startup(void);\n", d->prefix);
     fprintf(fp[0], "void %s_int_exit_handler(void);\n", d->prefix);
     fprintf(fp[0], "\n");
     write_prototypes(fp[0], d, &d->subs_all, 0);
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#ifdef __IDLPRE53__\n");
     fprintf(fp[0], "     static IDL_SYSFUN_DEF %s_int_procedures[] = {\n", d->prefix);
     write_idl_sysfun_defs(fp[0], d, &d->subs_all, 2, 0);
     fprintf(fp[0], "     };\n");
     fprintf(fp[0], "#else\n");
     fprintf(fp[0], "     static IDL_SYSFUN_DEF2 %s_int_procedures[] = {\n", d->prefix);
     write_idl_sysfun_defs(fp[0], d, &d->subs_all, 2, 1);
     fprintf(fp[0], "     };\n");
     fprintf(fp[0], "#endif\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "int IDL_Load(void)\n");
     fprintf(fp[0], "{\n");
     fprintf(fp[0], "#ifdef __IDLPRE53__\n");
     fprintf(fp[0], "     if (! IDL_AddSystemRoutine(%s_int_procedures, FALSE,\n", d->prefix);
     fprintf(fp[0], "                                ARRLEN(%s_int_procedures))) {\n", d->prefix);
     fprintf(fp[0], "          return IDL_FALSE;\n");
     fprintf(fp[0], "     }\n");
     fprintf(fp[0], "#else\n");
     fprintf(fp[0], "     if (! IDL_SysRtnAdd       (%s_int_procedures, FALSE,\n", d->prefix);
     fprintf(fp[0], "                                ARRLEN(%s_int_procedures))) {\n", d->prefix);
     fprintf(fp[0], "          return IDL_FALSE;\n");
     fprintf(fp[0], "     }\n");
     fprintf(fp[0], "#endif\n");
     fprintf(fp[0], "     IDL_ExitRegister(%s_int_exit_handler);\n", d->prefix);
     fprintf(fp[0], "     return(IDL_TRUE);\n");
     fprintf(fp[0], "}\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "void %s_int_exit_handler(void)\n", d->prefix);
     fprintf(fp[0], "{\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "}\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     write_subprograms(fp[0], d, SUBPROGRAM_TYPE_INIT,    &d->subs_init, 0);
     write_subprograms(fp[0], d, SUBPROGRAM_TYPE_FREE,    &d->subs_free, 0);
     write_subprograms(fp[0], d, SUBPROGRAM_TYPE_GENERAL, &d->subs_general, 0);


     write_dlm_header_top(fp[1]);
     fprintf(fp[1], "\n");

     fprintf(fp[1], "MODULE      %s\n", d->prefix);
     fprintf(fp[1], "DESCRIPTION %s\n", d->prefix);
     fprintf(fp[1], "VERSION     0.1\n");
     fprintf(fp[1], "SOURCE      %s developers\n", d->prefix);
     fprintf(fp[1], "BUILD_DATE  xxxx/xx/xx\n");
     write_dlm_procedures(fp[1], d, &d->subs_all);

     return 0;
}