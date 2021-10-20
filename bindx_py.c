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
#include "bindx_py.h"
#include "bindx_parse.h"
#include "bindx_parse_int.h"
#include "bindx_util.h"


static const char *type_to_py_format(const type_data *d, int enum_mask, int enum_array)
{
     switch(d->type) {
          case LEX_BINDX_TYPE_ENUM:
               if (enum_array)
                    return "O";
               else
               if (d->rank > 0)
                    return "O!";
               else
                    return "s";
               break;
          case LEX_BINDX_TYPE_CHAR:
               return "c";
               break;
          case LEX_BINDX_TYPE_INT:
               if (enum_mask)
                    return "O";
               else
               if (d->rank > 0)
                    return "O";
               else
                    return "i";
               break;
          case LEX_BINDX_TYPE_DOUBLE:
               if (d->rank > 0)
                    return "O";
               else
                    return "d";
               break;
          case LEX_BINDX_TYPE_STRUCTURE:
               return "O";
               break;
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return 0;
}



static const char *type_to_numpy_typenum(const type_data *d)
{
     switch(d->type) {
          case LEX_BINDX_TYPE_CHAR:
               return "NPY_CHAR";
               break;
          case LEX_BINDX_TYPE_INT:
               return "NPY_INT";
               break;
          case LEX_BINDX_TYPE_DOUBLE:
               return "NPY_DOUBLE";
               break;
          default:
               INTERNAL_ERROR("Invalid lex_bindx_type value: %d", d->type);
               break;
     }

     return NULL;
}



static const char *usage_to_numpy_requirements(enum lex_subprogram_argument_usage usage)
{
     switch(usage) {
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_IN:
               return "NPY_ARRAY_IN_ARRAY";
               break;
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT:
               return "NPY_ARRAY_OUT_ARRAY";
               break;
          case LEX_SUBPROGRAM_ARGUMENT_USAGE_IN_OUT:
               return "NPY_ARRAY_INOUT_ARRAY";
               break;
          default:
               INTERNAL_ERROR("Invalid lex_subprogram_argument_usage value: %d", usage);
               break;
     }

     return NULL;
}



static const char *get_error_return_value(enum subprogram_type sub_type)
{
     switch(sub_type) {
          case SUBPROGRAM_TYPE_GENERAL:
               return "NULL";
               break;
          case SUBPROGRAM_TYPE_INIT:
               return "-1";
               break;
          case SUBPROGRAM_TYPE_FREE:
               return "";
               break;
          default:
               INTERNAL_ERROR("Invalid int_subprogram_type value: %d", sub_type);
               break;
     }

     return NULL;
}



static int write_utilities(FILE *fp, const bindx_data *d)
{
     fprintf(fp, "static int list_to_mask(PyObject *list, int *mask, int (*name_to_mask)(const char *name), const char *name)\n");
     fprintf(fp, "{\n");
     fprintf(fp, "     int i;\n");
     fprintf(fp, "     int n;\n");
     fprintf(fp, "     int r;\n");

     fprintf(fp, "     char *format;\n");
     fprintf(fp, "     char *error;\n");

     fprintf(fp, "     *mask = 0;\n");
     fprintf(fp, "     n = PyList_Size(list);\n");
     fprintf(fp, "     for (i = 0; i < n; ++i) {\n");
     fprintf(fp, "#if PY_MAJOR_VERSION < 3\n");
     fprintf(fp, "          r = name_to_mask(PyString_AsString(PyList_GetItem(list, i)));\n");
     fprintf(fp, "#else\n");
     fprintf(fp, "          r = name_to_mask(PyUnicode_AsUTF8 (PyList_GetItem(list, i)));\n");
     fprintf(fp, "#endif\n");
     fprintf(fp, "          if (r == -1) {\n");
     fprintf(fp, "               format = \"ERROR: %%s()\";\n");
     fprintf(fp, "               error  = malloc(strlen(format) - 2 + strlen(name) + 1);\n");
     fprintf(fp, "               sprintf(error, format, name);\n");
     fprintf(fp, "               PyErr_SetString(%sError, error);\n", d->PREFIX);
     fprintf(fp, "               free(error);\n");
     fprintf(fp, "               return -1;\n");
     fprintf(fp, "          }\n");
     fprintf(fp, "          *mask |= r;\n");
     fprintf(fp, "     }\n");

     fprintf(fp, "     return 0;\n");
     fprintf(fp, "}\n");
     fprintf(fp, "\n");
     fprintf(fp, "\n");


     fprintf(fp, "static int list_to_array(PyObject *list, int *n, int **array, int (*name_to_value)(const char *name), const char *name)\n");
     fprintf(fp, "{\n");
     fprintf(fp, "     int i;\n");
     fprintf(fp, "     int r;\n");

     fprintf(fp, "     char *format;\n");
     fprintf(fp, "     char *error;\n");

     fprintf(fp, "     *n = PyList_Size(list);\n");
     fprintf(fp, "     *array = malloc(*n * sizeof(int));\n");
     fprintf(fp, "     for (i = 0; i < *n; ++i) {\n");
     fprintf(fp, "#if PY_MAJOR_VERSION < 3\n");
     fprintf(fp, "          r = name_to_value(PyString_AsString(PyList_GetItem(list, i)));\n");
     fprintf(fp, "#else\n");
     fprintf(fp, "          r = name_to_value(PyUnicode_AsUTF8 (PyList_GetItem(list, i)));\n");
     fprintf(fp, "#endif\n");
     fprintf(fp, "          if (r == -1) {\n");
     fprintf(fp, "               format = \"ERROR: %%s()\";\n");
     fprintf(fp, "               error  = malloc(strlen(format) - 2 + strlen(name) + 1);\n");
     fprintf(fp, "               sprintf(error, format, name);\n");
     fprintf(fp, "               PyErr_SetString(%sError, error);\n", d->PREFIX);
     fprintf(fp, "               free(error);\n");
     fprintf(fp, "               return -1;\n");
     fprintf(fp, "          }\n");
     fprintf(fp, "          (*array)[i] = r;\n");
     fprintf(fp, "     }\n");

     fprintf(fp, "     return 0;\n");
     fprintf(fp, "}\n");
     fprintf(fp, "\n");
     fprintf(fp, "\n");


     fprintf(fp, "static int check_pyarray_shape(PyObject *array, const char *name, int n, ...)\n");
     fprintf(fp, "{\n");
     fprintf(fp, "     int i;\n");
     fprintf(fp, "     int dim;\n");
     fprintf(fp, "     int n_py;\n");
     fprintf(fp, "     int dim_py;\n");
     fprintf(fp, "     va_list valist;\n");

     fprintf(fp, "     n_py = PyArray_NDIM((PyArrayObject *) array);\n");
     fprintf(fp, "     if (n_py != n) {\n");
     fprintf(fp, "          PyErr_Format(%sError, \"ERROR: Number of dimensions for %%s input (%%d) must be == %%d\", name, n_py);\n", d->PREFIX);
     fprintf(fp, "          return -1;\n");
     fprintf(fp, "     }\n");

     fprintf(fp, "     va_start(valist, n);\n");

     fprintf(fp, "     for (i = 0; i < n; ++i) {\n");
     fprintf(fp, "          dim = va_arg(valist, int);\n");
     fprintf(fp, "          dim_py = PyArray_DIM((PyArrayObject *) array, i);\n");
     fprintf(fp, "          if (dim_py != dim) {\n");
     fprintf(fp, "              PyErr_Format(%sError, \"ERROR: Dimension %%d of %%s input (%%d) must be == %%d\", i, name, dim_py, dim);\n", d->PREFIX);
     fprintf(fp, "              return -1;\n");
     fprintf(fp, "          }\n");
     fprintf(fp, "     }\n");

     fprintf(fp, "     va_end(valist);\n");

     fprintf(fp, "     return 0;\n");
     fprintf(fp, "}\n");
     fprintf(fp, "\n");
     fprintf(fp, "\n");


     fprintf(fp, "static void *array_from_ndarray(PyObject *ndarray, size_t size)\n");
     fprintf(fp, "{\n");
     fprintf(fp, "     int i;\n");
     fprintf(fp, "     int n_dims;\n");
     fprintf(fp, "     size_t *dims;\n");
     fprintf(fp, "     void *array;\n");

     fprintf(fp, "     n_dims = PyArray_NDIM((PyArrayObject *) ndarray);\n");
     fprintf(fp, "     dims = malloc(n_dims * sizeof(long));\n");
     fprintf(fp, "     for (i = 0; i < n_dims; ++i)\n");
     fprintf(fp, "          dims[i] = PyArray_DIM((PyArrayObject *) ndarray, i);\n");

     fprintf(fp, "     array = array_from_mem(PyArray_DATA((PyArrayObject *) ndarray), n_dims, dims, size, 1);\n");

     fprintf(fp, "     free(dims);\n");

     fprintf(fp, "     return array;\n");
     fprintf(fp, "}\n");
     fprintf(fp, "\n");
     fprintf(fp, "\n");

     return 0;
}



static int write_methods(FILE *fp, const bindx_data *d,
                         const subprogram_data *subs, const char *name)
{
     subprogram_data *subprogram;

     fprintf(fp, "static PyMethodDef %s_methods[] = {\n", d->prefix);

     list_for_each(subs, subprogram)
          fprintf(fp, "     {\"%s\", (PyCFunction) %s_%s_py, METH_VARARGS, \"null\"},\n",
                  subprogram->name, d->prefix, subprogram->name);

     fprintf(fp, "     {NULL}\n");
     fprintf(fp, "};\n");
     fprintf(fp, "\n");
     fprintf(fp, "\n");

     return 0;
}




static int write_subprograms(FILE *fp, const bindx_data *d,
                             enum subprogram_type sub_type,
                             const subprogram_data *subs, const char *name)
{
     char temp[NM];

     const char *format;

     int i;

     int flag;

     int indent = 0;

     int max_dims;

     argument_data *argument;
     subprogram_data *subprogram;

     list_for_each(subs, subprogram) {
          if (sub_type == SUBPROGRAM_TYPE_INIT)
               fprintf(fp, "static int %s_init(%s_data_py *self, PyObject *args)\n",
                       d->prefix, d->prefix);
          else
          if (sub_type == SUBPROGRAM_TYPE_FREE)
               fprintf(fp, "static void %s_dealloc(%s_data_py *self)\n",
                       d->prefix, d->prefix);
          else
               fprintf(fp, "static PyObject *%s_%s_py(%s_data_py *self, PyObject *args)\n",
                       d->prefix, subprogram->name, d->prefix);
          fprintf(fp, "{\n");

          max_dims = 0;
          list_for_each(subprogram->args, argument) {
               if (argument->type.rank > max_dims)
                    max_dims = argument->type.rank;
          }

          indent++;

          fprintf(fp, "%s", bxis(indent));
          bindx_write_c_type(fp, d, &subprogram->type, NULL);
          fprintf(fp, " r;\n");

          fprintf(fp, "%s%s_data *d = &self->%s;\n", bxis(indent), d->prefix, d->prefix);

          flag = 0;
          temp[0] = '\0';
          list_for_each(subprogram->args, argument) {
               if (argument->type.rank == 0 ||
                   argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY) {
                    fprintf(fp, "%s", bxis(indent));
                    bindx_write_c_declaration(fp, d, &argument->type, NULL);
                    fprintf(fp, "%s;\n", argument->name);
               }

               if (argument->type.rank == 0 &&
                   argument->type.type == LEX_BINDX_TYPE_ENUM)
                    fprintf(fp, "%sconst char *%s_string = NULL;\n", bxis(indent), argument->name);
               else
               if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_MASK ||
                   argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY)
                    fprintf(fp, "%sPyObject *%s_list;\n", bxis(indent), argument->name);
               else
               if (argument->type.rank > 0 &&
                   argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_IN) {
                    fprintf(fp, "%sPyObject *%s_object  = NULL;\n", bxis(indent), argument->name);
                    fprintf(fp, "%sPyObject *%s_ndarray = NULL;\n", bxis(indent), argument->name);
                    fprintf(fp, "%s", bxis(indent));
                    bindx_write_c_declaration(fp, d, &argument->type, NULL);
                    fprintf(fp, "%s;\n", argument->name);
               }
               else
               if (argument->type.rank > 0 &&
                   argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT) {
                    if (! flag) {
                         flag = 1;
                         fprintf(fp, "%snpy_intp dims[%d];\n", bxis(indent), max_dims);
                    }
                    fprintf(fp, "%sPyObject *%s_ndarray = NULL;\n", bxis(indent), argument->name);
                    fprintf(fp, "%s", bxis(indent));
                    bindx_write_c_declaration(fp, d, &argument->type, NULL);
                    fprintf(fp, "%s;\n", argument->name);
               }

               if (argument->usage != LEX_SUBPROGRAM_ARGUMENT_USAGE_IN)
                    continue;

               if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_LIST_SIZE)
                    continue;

               format = type_to_py_format(&argument->type,
                                          argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_MASK,
                                          argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY);
               strcat(temp, format);
          }

          if (sub_type != SUBPROGRAM_TYPE_FREE) {
               fprintf(fp, "%sif (! PyArg_ParseTuple(args, \"%s\"", bxis(indent), temp);
               list_for_each(subprogram->args, argument) {
                    if (argument->usage != LEX_SUBPROGRAM_ARGUMENT_USAGE_IN)
                         continue;
                    if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_LIST_SIZE)
                         continue;

                    fprintf(fp, ", &%s", argument->name);
                    if (argument->type.rank == 0 && argument->type.type == LEX_BINDX_TYPE_ENUM)
                         fprintf(fp, "_string");
                    else
                    if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_MASK ||
                        argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY)
                         fprintf(fp, "_list");
                    else
                    if (argument->type.rank > 0)
                         fprintf(fp, "_object");

               }
               fprintf(fp, "))\n");
               indent++;
               fprintf(fp, "%sreturn %s;\n", bxis(indent), get_error_return_value(sub_type));
               indent--;
          }

          list_for_each(subprogram->args, argument) {
               if (argument->type.rank == 0 && argument->type.type == LEX_BINDX_TYPE_ENUM) {
                    fprintf(fp, "%s%s = %s(%s_string);\n", bxis(indent), argument->name, argument->options.enum_name_to_value, argument->name);
                    fprintf(fp, "%sif ((int) %s < 0) {\n", bxis(indent), argument->name);
                    indent++;
                    fprintf(fp, "%sPyErr_SetString(%sError, \"ERROR: %s()\");\n", bxis(indent), d->PREFIX, argument->options.enum_name_to_value);
                    fprintf(fp, "%sreturn %s;\n", bxis(indent), get_error_return_value(sub_type));
                    indent--;
                    fprintf(fp, "%s}\n", bxis(indent));
               }
               else
               if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_MASK) {
                    fprintf(fp, "%sif (list_to_mask(%s_list, &%s, (int (*)(const char *)) %s, \"%s\"))\n",
                            bxis(indent), argument->name, argument->name, argument->options.enum_name_to_value, argument->options.enum_name_to_value);
                    indent++;
                    fprintf(fp, "%sreturn %s;\n", bxis(indent), get_error_return_value(sub_type));
                    indent--;
               }
               else
               if (argument->options.flags & SUBPROGRAM_ARGUMENT_OPTION_MASK_ENUM_ARRAY) {
                    fprintf(fp, "%sif (list_to_array(%s_list, &n_%s, (int **) &%s, (int (*)(const char *)) %s, \"%s\"))\n",
                            bxis(indent), argument->name, argument->name, argument->name, argument->options.enum_name_to_value, argument->options.enum_name_to_value);
                    indent++;
                    fprintf(fp, "%sreturn %s;\n", bxis(indent), get_error_return_value(sub_type));
                    indent--;
               }
               else
               if (argument->type.rank > 0 && argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_IN) {
                    fprintf(fp, "%s%s_ndarray = PyArray_FROM_OTF(%s_object, %s, %s);\n",
                            bxis(indent), argument->name, argument->name, type_to_numpy_typenum(&argument->type), usage_to_numpy_requirements(argument->usage));
                    fprintf(fp, "%sif (%s_ndarray == NULL)\n", bxis(indent), argument->name);
                    indent++;
                    fprintf(fp, "%sreturn NULL;\n", bxis(indent));
                    indent--;
                    fprintf(fp, "%sif (check_pyarray_shape(%s_ndarray, \"%s\", %d", bxis(indent), argument->name, argument->name, argument->type.rank);
                    for (i = 0; i < argument->type.rank; ++i)
                         fprintf(fp, ", %s", argument->type.dimens[i]);
                    fprintf(fp, ") < 0)\n");
                    indent++;
                    fprintf(fp, "%sreturn NULL;\n", bxis(indent));
                    indent--;
                    fprintf(fp, "%s%s = (", bxis(indent), argument->name);
                    bindx_write_c_declaration(fp, d, &argument->type, NULL);
                    fprintf(fp, ") array_from_ndarray(%s_ndarray, %ld);\n", argument->name, bindx_c_type_size(&argument->type));
               }
               else
               if (argument->type.rank > 0 && argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT) {
                    for (i = 0; i < argument->type.rank; ++i)
                         fprintf(fp, "%sdims[%d] = %s;\n", bxis(indent), i, argument->type.dimens[i]);
                    fprintf(fp, "%s%s_ndarray = PyArray_SimpleNew(%d, dims, %s);\n",
                            bxis(indent), argument->name, argument->type.rank, type_to_numpy_typenum(&argument->type));
                    fprintf(fp, "%sif (%s_ndarray == NULL)\n", bxis(indent), argument->name);
                    indent++;
                    fprintf(fp, "%sreturn NULL;\n", bxis(indent));
                    indent--;
                    fprintf(fp, "%s%s = (", bxis(indent), argument->name);
                    bindx_write_c_declaration(fp, d, &argument->type, NULL);
                    fprintf(fp, ") array_from_ndarray(%s_ndarray, %ld);\n", argument->name, bindx_c_type_size(&argument->type));
               }
          }

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

          fprintf(fp, "%sif (%s) {\n", bxis(indent), xrtm_error_conditional(subprogram->type.type));
          indent++;
          fprintf(fp, "%sPyErr_SetString(%sError, \"ERROR: %s_%s()\");\n", bxis(indent), d->PREFIX, d->prefix, subprogram->name);
          fprintf(fp, "%sreturn %s;\n", bxis(indent), get_error_return_value(sub_type));
          indent--;
          fprintf(fp, "%s}\n", bxis(indent));

          list_for_each(subprogram->args, argument) {
               if (argument->type.type == LEX_BINDX_TYPE_ENUM && argument->type.rank > 0)
                    fprintf(fp, "%sfree(%s);\n", bxis(indent), argument->name);
               else
               if (argument->type.rank > 0 && argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_IN)
                    fprintf(fp, "%sPy_DECREF(%s_ndarray);\n", bxis(indent), argument->name);

               if (argument->type.rank > 1)
                    fprintf(fp, "%sfree_array(%s, %d);\n", bxis(indent), argument->name, argument->type.rank);
          }

          if (sub_type == SUBPROGRAM_TYPE_INIT)
               fprintf(fp, "%sreturn 0;\n", bxis(indent));
          else
          if (sub_type == SUBPROGRAM_TYPE_FREE) {
               fprintf(fp, "#if PY_MAJOR_VERSION < 3\n");
               fprintf(fp, "%sself->ob_type->tp_free((PyObject *) self);\n", bxis(indent));
               fprintf(fp, "#else\n");
               fprintf(fp, "%s((PyObject*)(self))->ob_type->tp_free((PyObject *) self);\n", bxis(indent));
               fprintf(fp, "#endif\n");
          }
          else {
               if (subprogram->has_return_value)
                    fprintf(fp, "%sreturn Py_BuildValue(\"%s\", r);\n", bxis(indent), type_to_py_format(&subprogram->type, 0, 0));
               else {
                    if (subprogram_n_out_args(subprogram) == 0)
                         fprintf(fp, "%sreturn Py_BuildValue(\"i\",  0);\n", bxis(indent));
                    else {
                         fprintf(fp, "%sreturn Py_BuildValue(\"", bxis(indent));
                         list_for_each(subprogram->args, argument) {
                              if (argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT)
                                   fprintf(fp, "%s", type_to_py_format(&argument->type, 0, 0));
                         }
                         fprintf(fp, "\"");
                         list_for_each(subprogram->args, argument) {
                               if (argument->usage == LEX_SUBPROGRAM_ARGUMENT_USAGE_OUT) {
                                    if (argument->type.rank == 0)
                                         fprintf(fp, ", %s", argument->name);
                                    else
                                         fprintf(fp, ", %s_ndarray", argument->name);
                               }
                         }
                         fprintf(fp, ");\n");
                    }
               }
          }

          indent--;

          fprintf(fp, "}\n");

          fprintf(fp, "\n");
          fprintf(fp, "\n");
     }

     return 0;
}



int bindx_write_py(FILE **fp, const bindx_data *d, const char *name)
{
     bindx_write_c_header_top(fp[0]);
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#include <Python.h>\n");
     fprintf(fp[0], "#include <structmember.h>\n");
     fprintf(fp[0], "#ifndef Py_USING_UNICODE\n");
     fprintf(fp[0], "#define Py_USING_UNICODE\n");
     fprintf(fp[0], "#endif\n");
/*
     fprintf(fp[0], "#if NPY_FEATURE_VERSION < 0x00000007\n");
     fprintf(fp[0], "#define NPY_ARRAY_IN_ARRAY NPY_IN_ARRAY\n");
     fprintf(fp[0], "#define NPY_ARRAY_OUT_ARRAY NPY_OUT_ARRAY\n");
     fprintf(fp[0], "#define NPY_ARRAY_INOUT_ARRAY NPY_INOUT_ARRAY\n");
     fprintf(fp[0], "#else\n");
     fprintf(fp[0], "#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION\n");
     fprintf(fp[0], "#endif\n");
*/
     fprintf(fp[0], "#include <numpy/arrayobject.h>\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#include <gutil.h>\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#include <%s_interface.h>\n", d->prefix);
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "typedef struct {\n");
     fprintf(fp[0], "     PyObject_HEAD\n");
     fprintf(fp[0], "     %s_data %s;\n", d->prefix, d->prefix);
     fprintf(fp[0], "} %s_data_py;\n", d->prefix);
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "static PyObject *%sError;\n", d->PREFIX);
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "static PyObject *%s_new(PyTypeObject *type, PyObject *args, PyObject *kwds)\n", d->prefix);
     fprintf(fp[0], "{\n");
     fprintf(fp[0], "     %s_data_py *self;\n", d->prefix);
     fprintf(fp[0], "     self = (%s_data_py *) type->tp_alloc(type, 0);\n", d->prefix);
     fprintf(fp[0], "     if (self != NULL) { }\n");
     fprintf(fp[0], "     return (PyObject *) self;\n");
     fprintf(fp[0], "}\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     write_utilities(fp[0], d);

     write_subprograms(fp[0], d, SUBPROGRAM_TYPE_INIT,    &d->subs_init,    name);
     write_subprograms(fp[0], d, SUBPROGRAM_TYPE_FREE,    &d->subs_free,    name);
     write_subprograms(fp[0], d, SUBPROGRAM_TYPE_GENERAL, &d->subs_general, name);

     write_methods(fp[0], d, &d->subs_general,  name);
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "static PyMemberDef %s_members[] = {\n", d->prefix);
     fprintf(fp[0], "     {NULL}\n");
     fprintf(fp[0], "};\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "static PyTypeObject %s_type = {\n", d->prefix);
     fprintf(fp[0], "#if PY_MAJOR_VERSION < 3\n");
     fprintf(fp[0], "     PyObject_HEAD_INIT(NULL)\n");
     fprintf(fp[0], "     0,\n");						/* ob_size */
     fprintf(fp[0], "#else\n");
     fprintf(fp[0], "     PyVarObject_HEAD_INIT(NULL, 0)\n");
     fprintf(fp[0], "#endif\n");
     fprintf(fp[0], "     \"%s.%s\",\n", d->prefix, d->prefix);			/* tp_name */
     fprintf(fp[0], "     sizeof(%s_data_py),\n", d->prefix);			/* tp_basicsize */
     fprintf(fp[0], "     0,\n");						/* tp_itemsize */
     fprintf(fp[0], "     (destructor) %s_dealloc,\n", d->prefix);		/* tp_dealloc */
     fprintf(fp[0], "     0,\n");						/* tp_print */
     fprintf(fp[0], "     0,\n");						/* tp_getattr */
     fprintf(fp[0], "     0,\n");						/* tp_setattr */
     fprintf(fp[0], "     0,\n");						/* tp_compare */
     fprintf(fp[0], "     0,\n");						/* tp_repr */
     fprintf(fp[0], "     0,\n");						/* tp_as_number */
     fprintf(fp[0], "     0,\n");						/* tp_as_sequence */
     fprintf(fp[0], "     0,\n");						/* tp_as_mapping */
     fprintf(fp[0], "     0,\n");						/* tp_hash */
     fprintf(fp[0], "     0,\n");						/* tp_call */
     fprintf(fp[0], "     0,\n");						/* tp_str */
     fprintf(fp[0], "     0,\n");						/* tp_getattro */
     fprintf(fp[0], "     0,\n");						/* tp_setattro */
     fprintf(fp[0], "     0,\n");						/* tp_as_buffer */
     fprintf(fp[0], "     Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,\n");	/* tp_flags */
     fprintf(fp[0], "     \"%s object\",\n", d->PREFIX);			/* tp_doc */
     fprintf(fp[0], "     0,\n");						/* tp_traverse */
     fprintf(fp[0], "     0,\n");						/* tp_clear */
     fprintf(fp[0], "     0,\n");						/* tp_richcompare */
     fprintf(fp[0], "     0,\n");						/* tp_weaklistoffset */
     fprintf(fp[0], "     0,\n");						/* tp_iter */
     fprintf(fp[0], "     0,\n");						/* tp_iternext */
     fprintf(fp[0], "     %s_methods,\n", d->prefix);				/* tp_methods */
     fprintf(fp[0], "     %s_members,\n", d->prefix);				/* tp_members */
     fprintf(fp[0], "     0,\n");						/* tp_getset */
     fprintf(fp[0], "     0,\n");						/* tp_base */
     fprintf(fp[0], "     0,\n");						/* tp_dict */
     fprintf(fp[0], "     0,\n");						/* tp_descr_get */
     fprintf(fp[0], "     0,\n");						/* tp_descr_set */
     fprintf(fp[0], "     0,\n");						/* tp_dictoffset */
     fprintf(fp[0], "     (initproc) %s_init,\n", d->prefix);			/* tp_init */
     fprintf(fp[0], "     0,\n");						/* tp_alloc */
     fprintf(fp[0], "     (newfunc) %s_new\n", d->prefix);			/* tp_new */
     fprintf(fp[0], "};\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "static PyMethodDef module_methods[] = {\n");
     fprintf(fp[0], "     {NULL}\n");
     fprintf(fp[0], "};\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#ifndef PyMODINIT_FUNC\n");
     fprintf(fp[0], "#define PyMODINIT_FUNC void\n");
     fprintf(fp[0], "#endif\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#if PY_MAJOR_VERSION >= 3\n");
     fprintf(fp[0], "    static struct PyModuleDef module_def = {\n");
     fprintf(fp[0], "        PyModuleDef_HEAD_INIT,\n");
     fprintf(fp[0], "        \"%s\",\n", d->prefix);
     fprintf(fp[0], "        \"Module for accessing %s\",\n", d->PREFIX);
     fprintf(fp[0], "        -1,\n");
     fprintf(fp[0], "        module_methods,\n");
     fprintf(fp[0], "        NULL,\n");
     fprintf(fp[0], "        NULL,\n");
     fprintf(fp[0], "        NULL,\n");
     fprintf(fp[0], "        NULL\n");
     fprintf(fp[0], "    };\n");
     fprintf(fp[0], "#endif\n");
     fprintf(fp[0], "\n");
     fprintf(fp[0], "\n");

     fprintf(fp[0], "#if PY_MAJOR_VERSION < 3\n");
     fprintf(fp[0], "PyMODINIT_FUNC init%s(void)\n", d->prefix);
     fprintf(fp[0], "{\n");
     fprintf(fp[0], "     PyObject *module;\n");
     fprintf(fp[0], "     if (PyType_Ready(&%s_type) < 0)\n", d->prefix);
     fprintf(fp[0], "          return;\n");
     fprintf(fp[0], "     module = Py_InitModule3(\"%s\", module_methods, \"Module for accessing %s\");\n", d->prefix, d->PREFIX);
     fprintf(fp[0], "     if (module == NULL)\n");
     fprintf(fp[0], "          return;\n");
     fprintf(fp[0], "     Py_INCREF(&%s_type);\n", d->prefix);
     fprintf(fp[0], "     PyModule_AddObject(module, \"%s\", (PyObject *) &%s_type);\n", d->prefix, d->prefix);
     fprintf(fp[0], "     %sError = PyErr_NewException(\"%s.error\", NULL, NULL);\n", d->PREFIX, d->prefix);
     fprintf(fp[0], "     Py_INCREF(%sError);\n", d->PREFIX);
     fprintf(fp[0], "     PyModule_AddObject(module, \"error\", %sError);\n", d->PREFIX);
     fprintf(fp[0], "     import_array();\n");
     fprintf(fp[0], "}\n");
     fprintf(fp[0], "#else\n");
     fprintf(fp[0], "PyMODINIT_FUNC PyInit_%s(void)\n", d->prefix);
     fprintf(fp[0], "{\n");
     fprintf(fp[0], "     PyObject *module;\n");
     fprintf(fp[0], "     if (PyType_Ready(&%s_type) < 0)\n", d->prefix);
     fprintf(fp[0], "          return NULL;\n");
     fprintf(fp[0], "     module = PyModule_Create(&module_def);\n");
     fprintf(fp[0], "     if (module == NULL)\n");
     fprintf(fp[0], "          return NULL;\n");
     fprintf(fp[0], "     Py_INCREF(&%s_type);\n", d->prefix);
     fprintf(fp[0], "     PyModule_AddObject(module, \"%s\", (PyObject *) &%s_type);\n", d->prefix, d->prefix);
     fprintf(fp[0], "     %sError = PyErr_NewException(\"%s.error\", NULL, NULL);\n", d->PREFIX, d->prefix);
     fprintf(fp[0], "     Py_INCREF(%sError);\n", d->PREFIX);
     fprintf(fp[0], "     PyModule_AddObject(module, \"error\", %sError);\n", d->PREFIX);
     fprintf(fp[0], "     import_array();\n");
     fprintf(fp[0], "     return module;\n");
     fprintf(fp[0], "}\n");
     fprintf(fp[0], "#endif\n");

     return 0;
}
