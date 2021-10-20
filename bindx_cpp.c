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
#include "bindx_cpp.h"
#include "bindx_parse.h"
#include "bindx_parse_int.h"
#include "bindx_util.h"


static int write_type(FILE *fp, const type_data *d, const char *name)
{
     switch(d->type) {
          case LEX_BINDX_TYPE_ENUM:
               if (! name)
                    fprintf(fp, "%s", d->name);
               else
                    fprintf(fp, "%s::%s", name, d->name);
               break;
          case LEX_BINDX_TYPE_CHAR:
               fprintf(fp, "char");
               break;
          case LEX_BINDX_TYPE_INT:
               fprintf(fp, "int");
               break;
          case LEX_BINDX_TYPE_DOUBLE:
               fprintf(fp, "double");
               break;
          case LEX_BINDX_TYPE_STRUCTURE:
               fprintf(fp, "%s", d->name);
               break;
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return 0;
}



static int write_dimens_args(FILE *fp, const bindx_data *d, const type_data *type,
                             enum lex_subprogram_argument_usage usage)
{
     return bindx_write_c_dimens_args(fp, d, type, usage);
}



static int write_dimens_return(FILE *fp, const bindx_data *d, const type_data *type)
{
     return bindx_write_c_dimens_return(fp, d, type);
}



static int write_enumerations(FILE *fp, const bindx_data *d,
                              const enumeration_data *enums, int indent)
{
     enum_member_data *enum_member;
     enumeration_data *enumeration;

     list_for_each(enums, enumeration) {
          fprintf(fp, "%senum %s {\n", bxis(indent), enumeration->name);

          list_for_each(enumeration->members, enum_member) {
               fprintf(fp, "%s%s = %s_%s", bxis(indent + 1), enum_member->name,
                       d->PREFIX, enum_member->name);

               if (! list_is_last_elem(enumeration->members, enum_member))
                    fprintf(fp, ",");
               fprintf(fp, "\n");
          }

          fprintf(fp, "%s};\n", bxis(indent));

          fprintf(fp, "\n");
     }

     return 0;
}



static int write_prototypes(FILE *fp, const bindx_data *d,
                            enum subprogram_type sub_type,
                            const subprogram_data *subs,
                            const char *name, int indent)
{
     argument_data *argument;
     subprogram_data *subprogram;

     list_for_each(subs, subprogram) {
          fprintf(fp, "%s", bxis(indent));

          if (sub_type == SUBPROGRAM_TYPE_INIT)
               fprintf(fp, "%s(", name);
          else
          if (sub_type == SUBPROGRAM_TYPE_FREE)
               fprintf(fp, "~%s(", name);
          else {
               if (subprogram->has_return_value ||
                   subprogram_n_scaler_out_args(subprogram) == 1) {
                    write_type(fp, &subprogram->type, name);
                    write_dimens_return(fp, d, &subprogram->type);
               }
               else
                    fprintf(fp, "void");
               fprintf(fp, " %s(", subprogram->name);
          }

          list_for_each(subprogram->args, argument) {
               if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_EXTERNAL)
                    write_type(fp, &argument->type, argument->options.enum_external_class);
               else
                    write_type(fp, &argument->type, NULL);
               fprintf(fp, " ");
               write_dimens_args(fp, d, &argument->type, argument->usage);
               fprintf(fp, "%s", argument->name);
               if (! list_is_last_elem(subprogram->args, argument))
                    fprintf(fp, ", ");
          }

          fprintf(fp, ");\n");
     }

     return 0;
}



static int write_subprograms(FILE *fp, const bindx_data *d,
                             enum subprogram_type sub_type,
                             const subprogram_data *subs,
                             const char *name, int indent)
{
     argument_data *argument;
     subprogram_data *subprogram;

     list_for_each(subs, subprogram) {
          fprintf(fp, "%s", bxis(indent));

          if (sub_type == SUBPROGRAM_TYPE_INIT)
               fprintf(fp, "%s::%s(", name, name);
          else
          if (sub_type == SUBPROGRAM_TYPE_FREE)
               fprintf(fp, "%s::~%s(", name, name);
          else {
               if (subprogram->has_return_value ||
                   subprogram_n_scaler_out_args(subprogram) == 1) {
                    write_type(fp, &subprogram->type, name);
                    write_dimens_return(fp, d, &subprogram->type);
               }
               else
                    fprintf(fp, "void");
               fprintf(fp, " %s::%s(", name, subprogram->name);
          }

          list_for_each(subprogram->args, argument) {
               if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_EXTERNAL)
                    write_type(fp, &argument->type, argument->options.enum_external_class);
               else
                    write_type(fp, &argument->type, name);
               fprintf(fp, " ");
               write_dimens_args(fp, d, &argument->type, argument->usage);
               fprintf(fp, "%s", argument->name);
               if (! list_is_last_elem(subprogram->args, argument))
                    fprintf(fp, ", ");
          }

          fprintf(fp, ")\n");
          fprintf(fp, "{\n");

          indent++;

          fprintf(fp, "%s", bxis(indent));
          bindx_write_c_type(fp, d, &subprogram->type, NULL);
          fprintf(fp, " r;\n");

          fprintf(fp, "%sr = %s_%s(&d", bxis(indent), d->prefix, subprogram->name);

          list_for_each(subprogram->args, argument) {
               fprintf(fp, ", ");
               if (argument->type.type == LEX_BINDX_TYPE_ENUM) {
                    if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_EXTERNAL)
                         fprintf(fp, "(enum %s_%s", argument->options.enum_external_type,
                                                    argument->type.name);
                    else
                         fprintf(fp, "(enum %s_%s", d->prefix, argument->type.name);
                    if (argument->type.rank > 0)
                         fprintf(fp, " ");
                    write_dimens_args(fp, d, &argument->type, argument->usage);
                    fprintf(fp, ") ");
               }
               fprintf(fp, "%s", argument->name);
          }

          fprintf(fp, ");\n");

          fprintf(fp, "%sif (%s)\n", bxis(indent),
                  xrtm_error_conditional(subprogram->type.type));
          indent++;
          fprintf(fp, "%sthrow %s::ERROR;\n", bxis(indent), name);
          indent--;

          if (subprogram->has_return_value ||
              subprogram_n_scaler_out_args(subprogram) == 1) {
               fprintf(fp, "%sreturn ", bxis(indent));
               if (subprogram->type.type == LEX_BINDX_TYPE_ENUM) {
                    fprintf(fp, "(");
                    write_type(fp, &subprogram->type, name);
                    fprintf(fp, ") ");
               }
               fprintf(fp, "r;\n");
          }

          indent--;

          fprintf(fp, "}\n");

          fprintf(fp, "\n");
          fprintf(fp, "\n");
     }

     return 0;
}



int bindx_write_cpp(FILE **fp, const bindx_data *d, const char *name)
{
     bindx_write_c_header_top(fp[0]);
     fprintf(fp[0], "\n");
     fprintf(fp[0], "#ifndef %s_INT_CPP_H\n", d->PREFIX);
     fprintf(fp[0], "#define %s_INT_CPP_H\n", d->PREFIX);
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#include <gutil.h>\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "#include <%s_interface.h>\n", d->prefix);
     fprintf(fp[0], "\n");
     if (d->include) {
          fprintf(fp[0], "#include \"%s_int_cpp.h\"\n", d->include);
          fprintf(fp[0], "\n");
     }
     fprintf(fp[0], "\n");

     fprintf(fp[0], "class %s\n", name);
     fprintf(fp[0], "{\n");
     fprintf(fp[0], "private:\n");
     fprintf(fp[0], "     %s_data d;\n", d->prefix);
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "public:\n");
     fprintf(fp[0], "     enum %s_errors { ERROR };\n", d->prefix);
     fprintf(fp[0], "\n");

     write_enumerations(fp[0], d, &d->enums, 1);
     fprintf(fp[0], "\n");

     write_prototypes(fp[0], d, SUBPROGRAM_TYPE_INIT,    &d->subs_init,    name, 1);
     write_prototypes(fp[0], d, SUBPROGRAM_TYPE_FREE,    &d->subs_free,    name, 1);
     write_prototypes(fp[0], d, SUBPROGRAM_TYPE_GENERAL, &d->subs_general, name, 1);
     fprintf(fp[0], "};\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#endif /* %s_INT_CPP_H */\n", d->PREFIX);


     bindx_write_c_header_top(fp[1]);
     fprintf(fp[1], "\n");

     fprintf(fp[1], "#include <gutil.h>\n");
     fprintf(fp[1], "\n");
     fprintf(fp[1], "#include <%s_interface.h>\n", d->prefix);
     fprintf(fp[1], "\n");
     fprintf(fp[1], "#include \"%s_int_cpp.h\"\n", d->prefix);
     fprintf(fp[1], "\n");
     fprintf(fp[1], "\n");

     write_subprograms(fp[1], d, SUBPROGRAM_TYPE_INIT,    &d->subs_init,    name, 0);
     write_subprograms(fp[1], d, SUBPROGRAM_TYPE_FREE,    &d->subs_free,    name, 0);
     write_subprograms(fp[1], d, SUBPROGRAM_TYPE_GENERAL, &d->subs_general, name, 0);

     return 0;
}
