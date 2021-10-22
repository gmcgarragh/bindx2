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
#include "bindx_f90.h"
#include "bindx_parse.h"
#include "bindx_parse_int.h"


static int write_header(FILE *fp)
{
     fprintf(fp, "!*******************************************************************************\n");
     fprintf(fp, "!\n");
     fprintf(fp, "!*******************************************************************************\n");

     return 0;
}



static int write_header_top(FILE *fp)
{
     fprintf(fp, "!*******************************************************************************\n");
     fprintf(fp, "! This file was generated by bindx version %s.  Edit at your own risk.\n",
             bindx_version());
     fprintf(fp, "!*******************************************************************************\n");

     return 0;
}



static char *crap_data_type_replace(const char *s)
{
     char *p;
     char *s2;

     s2 = strdup(s);
     p = strstr(s2, "_data");
     if (p != NULL) {
          strcpy(p, "_type");
     }

     return s2;
}




static int write_type(FILE *fp, const type_data *d)
{
     char *crap;

     switch(d->type) {
          case LEX_BINDX_TYPE_ENUM:
               fprintf(fp, "integer");
               break;
          case LEX_BINDX_TYPE_CHAR:
               fprintf(fp, "character");
               break;
          case LEX_BINDX_TYPE_INT:
               fprintf(fp, "integer");
               break;
          case LEX_BINDX_TYPE_DOUBLE:
               fprintf(fp, "real(8)");
               break;
          case LEX_BINDX_TYPE_STRUCTURE:
               crap = crap_data_type_replace(d->name);
               fprintf(fp, "type(%s)", crap);
               free(crap);
               break;
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return 0;
}



static int write_type_bind_c(FILE *fp, const type_data *d)
{
     char *crap;

     switch(d->type) {
          case LEX_BINDX_TYPE_ENUM:
               fprintf(fp, "integer(c_int)");
               break;
          case LEX_BINDX_TYPE_CHAR:
               fprintf(fp, "character(c_char)");
               break;
          case LEX_BINDX_TYPE_INT:
               fprintf(fp, "integer(c_int)");
               break;
          case LEX_BINDX_TYPE_DOUBLE:
               fprintf(fp, "real(c_double)");
               break;
          case LEX_BINDX_TYPE_STRUCTURE:
               crap = crap_data_type_replace(d->name);
               fprintf(fp, "type(%s)", crap);
               free(crap);
               break;
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return 0;
}



static int write_usage(FILE *fp, int usage)
{
     switch(usage) {
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_IN:
               fprintf(fp, "intent(in)");
               break;
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT:
               fprintf(fp, "intent(out)");
               break;
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_IN_OUT:
               fprintf(fp, "intent(inout)");
               break;
          default:
               INTERNAL_ERROR("Invalid lex_subprogram_argument_usage value: %d", usage);
               break;
     }

     return 0;
}



static int write_dimens(FILE *fp, const type_data *d)
{
     int i;

     if (d->rank > 0) {
          fprintf(fp, "(");

          for (i = 0; i < d->rank; ++i) {
               fprintf(fp, ":");

               if (i + 1 < d->rank)
                    fprintf(fp, ",");
          }

          fprintf(fp, ")");
     }

     return 0;
}



static int bindx_write_c_util_header(FILE *fp, const bindx_data *d)
{
     fprintf(fp, "#include <gutil.h>\n");
     fprintf(fp, "\n");
     fprintf(fp, "#include <%s_interface.h>\n", d->prefix);
     fprintf(fp, "\n");
     fprintf(fp, "#ifdef __cplusplus\n");
     fprintf(fp, "extern \"C\" {\n");
     fprintf(fp, "#endif\n");
     fprintf(fp, "\n");
     fprintf(fp, "\n");

     return 0;
}



static int bindx_write_c_util_trailer(FILE *fp, const bindx_data *d)
{
     fprintf(fp, "#ifdef __cplusplus\n");
     fprintf(fp, "}\n");
     fprintf(fp, "#endif\n");

     return 0;
}



static int bindx_write_c_util_functions(FILE *fp, const bindx_data *d,
                                        const subprogram_data *subs)
{
     int i;

     char *indent = "     ";

     argument_data *argument;
     subprogram_data *subprogram;

     list_for_each(subs, subprogram) {
          if (! subprogram->has_multi_dimen_args)
               continue;

          bindx_write_c_type(fp, d, &subprogram->type, NULL);
          fprintf(fp, " %s_%s_bindx_f90(%s_data *d", d->prefix, subprogram->name,
                  d->prefix);

          list_for_each(subprogram->args, argument) {
               fprintf(fp, ", ");
               if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_EXTERNAL)
                    bindx_write_c_type(fp, d, &argument->type,
                                       argument->options.enum_external_type);
               else
                    bindx_write_c_type(fp, d, &argument->type, NULL);
               fprintf(fp, " ");
               if (argument->type.rank > 0)
                    fprintf(fp, "*");
               fprintf(fp, "%s", argument->name);
          }

          fprintf(fp, ")\n");
          fprintf(fp, "{\n");

          list_for_each(subprogram->args, argument) {
               if (argument->type.rank > 1) {
                    fprintf(fp, "%s", indent);
                    bindx_write_c_type(fp, d, &argument->type, NULL);
                    fprintf(fp, " ");
                    for (i = 0; i < argument->type.rank; ++i)
                         fprintf(fp, "*");
                    fprintf(fp, "%s2;", argument->name);
                    fprintf(fp, "\n");
               }
          }

          list_for_each(subprogram->args, argument) {
               if (argument->type.rank > 1) {
                    fprintf(fp, "%s%s2 = array_from_mem%d_d(%s, ", indent,
                            argument->name, argument->type.rank, argument->name);

                    for (i = 0; i < argument->type.rank; ++i) {
                         fprintf(fp, "%s", argument->type.dimens[i]);
                         if (i + 1 < argument->type.rank)
                              fprintf(fp, ", ");
                    }

                    fprintf(fp, ");\n");
               }
          }

          fprintf(fp, "%sif (%s_%s(d", indent, d->prefix, subprogram->name);

          list_for_each(subprogram->args, argument) {
               fprintf(fp, ", %s", argument->name);
               if (argument->type.rank > 1)
                    fprintf(fp, "2");
          }

          fprintf(fp, ")) {\n");

          fprintf(fp, "%s%sfprintf(stderr, \"ERROR: %s_%s()\\n\");\n", indent, indent,
                  d->prefix, subprogram->name);
          fprintf(fp, "%s%sreturn -1;\n", indent, indent);

          fprintf(fp, "%s}\n", indent);

          list_for_each(subprogram->args, argument) {
               if (argument->type.rank > 1)
                    fprintf(fp, "%sfree_array%d_d(%s2);\n", indent,
                            argument->type.rank, argument->name);
          }

          fprintf(fp, "%sreturn 0;\n", indent);

          fprintf(fp, "}\n");
          fprintf(fp, "\n");
          fprintf(fp, "\n");
     }

     return 0;
}



static int write_public_list(FILE *fp, const bindx_data *d,
                             const structure_data *structs,
                             const subprogram_data *subs)
{
     char *indent  = "";
     char *indent2 = "          ";

     structure_data *structure;
     subprogram_data *subprogram;

     if (! list_is_empty(structs) || ! list_is_empty(subs)) {
          fprintf(fp, "public :: ");

          list_for_each(structs, structure) {
               fprintf(fp, "%s%s", indent, structure->name);
               if (! list_is_last_elem(structs, structure))
                    fprintf(fp, ", &\n");
               indent = indent2;
          }

          if (! list_is_empty(subs))
               fprintf(fp, ", &\n");

          list_for_each(subs, subprogram) {
               fprintf(fp, "%s%s_%s_f90", indent, d->prefix, subprogram->name);
               if (! list_is_last_elem(subs, subprogram))
                    fprintf(fp, ", &\n");
               indent = indent2;
          }

          fprintf(fp, "\n");
     }

     return 0;
}



static int write_enumerations(FILE *fp, const bindx_data *d,
                              const enumeration_data *enums)
{
     enum_member_data *enum_member;
     enumeration_data *enumeration;

     list_for_each(enums, enumeration) {
          list_for_each(enumeration->members, enum_member)
               fprintf(fp, "integer, parameter, public :: %s_%s = %d_int64\n",
                       d->PREFIX, enum_member->name, enum_member->value);

          fprintf(fp, "\n");
     }

     return 0;
}



static int write_structures(FILE *fp, const bindx_data *d,
                            const structure_data *structs)
{
     structure_data *structure;

     list_for_each(structs, structure) {
          fprintf(fp, "type, bind(c) :: %s\n", structure->name);
          fprintf(fp, "     integer(c_signed_char) :: data(%lu)\n", structure->size);
          fprintf(fp, "end type %s\n", structure->name);
     }

     return 0;
}



static int write_interfaces(FILE *fp, const bindx_data *d,
                            const subprogram_data *subs)
{
     char *indent = "     ";

     argument_data *argument;
     subprogram_data *subprogram;

     list_for_each(subs, subprogram) {
          fprintf(fp, "interface\n");

          fprintf(fp, "%s", indent);
          write_type_bind_c(fp, &subprogram->type);
          fprintf(fp, " ");

          fprintf(fp, "function %s_%s", d->prefix, subprogram->name);
          if (subprogram->has_multi_dimen_args)
               fprintf(fp, "_bindx_f90");
          fprintf(fp, "(d");

          list_for_each(subprogram->args, argument)
               fprintf(fp, ", %s", argument->name);

          fprintf(fp, ") bind(c, name = \"%s_%s", d->prefix, subprogram->name);
          if (subprogram->has_multi_dimen_args)
               fprintf(fp, "_bindx_f90");
          fprintf(fp, "\")\n");
          fprintf(fp, "%suse iso_c_binding\n", indent);
          fprintf(fp, "%simport %s_type\n", indent, d->prefix);
          if (d->include)
               fprintf(fp, "%simport %s_type\n", indent, d->include);
          fprintf(fp, "%simplicit none\n", indent);

          fprintf(fp, "%stype(%s_type), intent(inout) :: d\n", indent, d->prefix);

          list_for_each(subprogram->args, argument) {
               fprintf(fp, "%s", indent);
               write_type_bind_c(fp, &argument->type);
               fprintf(fp, ", ");
               write_usage(fp, argument->usage);
               if (argument->type.rank == 0 &&
                   argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_IN)
                    fprintf(fp, ", value");
               fprintf(fp, " :: ");
               fprintf(fp, "%s", argument->name);
               if (argument->type.type != LEX_BINDX_TYPE_STRUCTURE &&
                   argument->type.rank > 0)
                    fprintf(fp, "(*)");
               fprintf(fp, "\n");
          }

          fprintf(fp, "%send function %s_%s", indent, d->prefix, subprogram->name);
          if (subprogram->has_multi_dimen_args)
               fprintf(fp, "_bindx_f90");
          fprintf(fp, "\n");
          fprintf(fp, "end interface\n");
          fprintf(fp, "\n");
          fprintf(fp, "\n");
     }

     return 0;
}



static int write_subprograms(FILE *fp, const bindx_data *d,
                             const subprogram_data *subs)
{
     char *indent = "     ";

     argument_data *argument;
     subprogram_data *subprogram;

     list_for_each(subs, subprogram) {
          if (subprogram->has_return_value ||
              subprogram_n_scaler_out_args(subprogram) == 1) {
               write_type(fp, &subprogram->type);
               fprintf(fp, " function %s_%s_f90(d", d->prefix, subprogram->name);
          }
          else
               fprintf(fp, "subroutine %s_%s_f90(d", d->prefix, subprogram->name);

          list_for_each(subprogram->args, argument)
               fprintf(fp, ", %s", argument->name);

          if (! subprogram->has_return_value &&
              subprogram_n_scaler_out_args(subprogram) != 1)
               fprintf(fp, ", error");

          fprintf(fp, ")\n");

          fprintf(fp, "%simplicit none\n", indent);

          fprintf(fp, "%stype(%s_type), intent(inout) :: d\n", indent, d->prefix);

          list_for_each(subprogram->args, argument) {
               fprintf(fp, "%s", indent);
               write_type(fp, &argument->type);
               fprintf(fp, ", ");
               write_usage(fp, argument->usage);
               fprintf(fp, " :: ");
               fprintf(fp, "%s", argument->name);
               if (argument->type.type != LEX_BINDX_TYPE_STRUCTURE)
                    write_dimens(fp, &argument->type);
               fprintf(fp, "\n");
          }

          if (! subprogram->has_return_value &&
              subprogram_n_scaler_out_args(subprogram) != 1)
               fprintf(fp, "%sinteger, intent(out) :: error\n", indent);

          if (subprogram->has_return_value ||
              subprogram_n_scaler_out_args(subprogram) == 1)
               fprintf(fp, "%s%s_%s_f90 = ", indent, d->prefix, subprogram->name);
          else
               fprintf(fp, "%serror = ", indent);

          fprintf(fp, "%s_%s", d->prefix, subprogram->name);
          if (subprogram->has_multi_dimen_args)
               fprintf(fp, "_bindx_f90");
          fprintf(fp, "(d");

          list_for_each(subprogram->args, argument)
               fprintf(fp, ", %s", argument->name);

          fprintf(fp, ")\n");

          if (subprogram->has_return_value ||
              subprogram_n_scaler_out_args(subprogram) == 1)
               fprintf(fp, "end function %s_%s_f90\n", d->prefix, subprogram->name);
          else
               fprintf(fp, "end subroutine %s_%s_f90\n", d->prefix, subprogram->name);
          fprintf(fp, "\n");
          fprintf(fp, "\n");
     }

     return 0;
}



int bindx_write_f90(FILE **fp, const bindx_data *d, const char *name)
{
     bindx_write_c_header_top(fp[0]);
     fprintf(fp[0], "\n");

     bindx_write_c_util_header(fp[0], d);
     bindx_write_c_util_functions(fp[0], d, &d->subs_all);
     bindx_write_c_util_trailer(fp[0], d);

     write_header_top(fp[1]);
     fprintf(fp[1], "\n");

     fprintf(fp[1], "module %s\n", name);
     fprintf(fp[1], "\n");
     fprintf(fp[1], "use iso_c_binding\n");
     fprintf(fp[1], "use iso_fortran_env\n");
     if (d->include)
          fprintf(fp[1], "use %s_int_f90\n", d->include);
     fprintf(fp[1], "\n");
     fprintf(fp[1], "\n");

     fprintf(fp[1], "private\n");
     fprintf(fp[1], "\n");
     write_public_list(fp[1], d, &d->structs, &d->subs_all);
     fprintf(fp[1], "\n");
     fprintf(fp[1], "\n");
     fprintf(fp[1], "\n");

     write_header(fp[1]);
     write_enumerations(fp[1], d, &d->enums);
     fprintf(fp[1], "\n");
     fprintf(fp[1], "\n");

     write_header(fp[1]);
     write_structures(fp[1], d, &d->structs);
     fprintf(fp[1], "\n");
     fprintf(fp[1], "\n");
     fprintf(fp[1], "\n");

     write_header(fp[1]);
     write_interfaces(fp[1], d, &d->subs_all);
     fprintf(fp[1], "\n");

     write_header(fp[1]);
     fprintf(fp[1], "contains\n");
     fprintf(fp[1], "\n");
     fprintf(fp[1], "\n");

     write_subprograms(fp[1], d, &d->subs_all);

     fprintf(fp[1], "end module %s\n", name);

     return 0;
}
