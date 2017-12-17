/******************************************************************************%
**
**    Copyright (C) 2011-2017 Greg McGarragh <gregm@atmos.colostate.edu>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#include <gutil.h>
#include <glist.h>

#include "bindx_c.h"
#include "bindx_cpp.h"
#include "bindx_f90.h"
#include "bindx_f77.h"
#include "bindx_idl.h"
#include "bindx_py.h"
#include "bindx_parse_int.h"
#include "bindx_yylex_int.h"
#include "bindx_util.h"


#ifdef NAME
#undef NAME
#endif
#define NAME    "bindx"


#define MAX_IN_FILES	16
#define MAX_OUT_FILES	16


typedef struct {
     int def;
     int c;
     int cpp;
     int f77;
     int f90;
     int idl;
     int py;
     int help;
     int version;
} options_data;


int bindx_write_x(const bindx_data *bindx_int, const char *name, int n_out_files,
                  char **out_files, int (*write)(FILE **, const bindx_data *,
                  const char *));
void usage();


int main(int argc, char *argv[]) {

     char *name_cpp;
     char *name_f77;
     char *name_f90;
     char *name_idl;
     char *name_py;

     char *in_files_def[MAX_IN_FILES];

     char *out_files_def[MAX_OUT_FILES];

     char *out_files_cpp[MAX_OUT_FILES];
     char *out_files_f77[MAX_OUT_FILES];
     char *out_files_f90[MAX_OUT_FILES];
     char *out_files_idl[MAX_OUT_FILES];
     char *out_files_py[MAX_OUT_FILES];

     int i;
     int n;

     int n_in_files_def;

     int n_out_files_cpp;
     int n_out_files_f77;
     int n_out_files_f90;
     int n_out_files_idl;
     int n_out_files_py;

     FILE *fp;

     bindx_data bindx_int;

     locus_data locus;

     options_data options;


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     options.def     = 0;
     options.c       = 0;
     options.cpp     = 0;
     options.f77     = 0;
     options.f90     = 0;
     options.idl     = 0;
     options.py      = 0;
     options.help    = 0;
     options.version = 0;

     n_in_files_def  = 0;

     n = 0;
     for (i = 1; i < argc; ++i) {
          if (argv[i][0] == '-') {
               if (strcmp(argv[i], "-int_def_in") == 0) {
                    check_arg_count(i, argc, 1, argv[i]);
                    in_files_def[n_in_files_def++] = argv[++i];
               }
               else if (strcmp(argv[i], "-def_out") == 0) {
                    check_arg_count(i, argc, 1, argv[i]);
                    options.def = 1;
                    out_files_def [0] = argv[++i];
               }
               else if (strcmp(argv[i], "-cpp") == 0) {
                    check_arg_count(i, argc, 3, argv[i]);
                    options.cpp = 1;
                    name_cpp = argv[++i];
                    n_out_files_cpp = 2;
                    out_files_cpp[0] = argv[++i];
                    out_files_cpp[1] = argv[++i];
               }
               else if (strcmp(argv[i], "-f77") == 0) {
                    check_arg_count(i, argc, 4, argv[i]);
                    options.f77 = 1;
                    name_f77 = argv[++i];
                    n_out_files_f77 = 3;
                    out_files_f77[0] = argv[++i];
                    out_files_f77[1] = argv[++i];
                    out_files_f77[2] = argv[++i];
               }
               else if (strcmp(argv[i], "-f90") == 0) {
                    check_arg_count(i, argc, 3, argv[i]);
                    options.f90 = 1;
                    name_f90 = argv[++i];
                    n_out_files_f90 = 2;
                    out_files_f90[0] = argv[++i];
                    out_files_f90[1] = argv[++i];
               }
               else if (strcmp(argv[i], "-idl") == 0) {
                    check_arg_count(i, argc, 3, argv[i]);
                    options.idl = 1;
                    name_idl = argv[++i];
                    n_out_files_idl = 2;
                    out_files_idl[0] = argv[++i];
                    out_files_idl[1] = argv[++i];
               }
                else if (strcmp(argv[i], "-py") == 0) {
                    check_arg_count(i, argc, 2, argv[i]);
                    options.py = 1;
                    name_py = argv[++i];
                    n_out_files_py = 1;
                    out_files_py[0] = argv[++i];
               }
              else if (strcmp(argv[i], "-help") == 0) {
                    usage();
                    exit(0);
               }
               else if (strcmp(argv[i], "-version") == 0) {
                    printf("%s\n", bindx_version());
                    exit(0);
               }
               else {
                    printf("Invalid option: %s, use -help for more information\n", argv[i]);
                    exit(1);
               }
          }
          else {
               if (n == 0)
                    ;
               else {
                    printf("Too many arguments, use -help for more information\n");
                    exit(1);
               }

               ++n;
          }
     }

     if (n < 0) {
          printf("Not enough arguments, use -help for more information\n");
          exit(1);
     }


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     bindx_init(&bindx_int);

     for (i = 0; i < n_in_files_def; ++i) {
          if (strcmp(in_files_def[i], "-") == 0) {
               yy_in = stdin;

               locus.file = "stdin";
          }
          else {
               if ((yy_in = fopen(in_files_def[i], "r")) == NULL) {
                    fprintf(stderr, "ERROR: Problem opening file for reading: %s ... %s\n",
                            in_files_def[i], strerror(errno));
                    exit(1);
               }

               locus.file = in_files_def[i];
          }

          locus.line = 1;

          bindx_parse(&bindx_int, &locus);

          if (yy_in != stdin)
               fclose(yy_in);
     }


     bindx_finialize(&bindx_int);


     if (options.def) {
          if ((fp = fopen(out_files_def[0], "w")) == NULL) {
               fprintf(stderr, "ERROR: Problem opening file for writing: %s ... %s\n",
                       out_files_def[i], strerror(errno));
               exit(1);
          }

          bindx_write(fp, &bindx_int);

          fclose(fp);
     }


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     if (options.cpp) {
          if (bindx_write_x(&bindx_int, name_cpp, n_out_files_cpp, out_files_cpp,
              bindx_write_cpp)) {
              fprintf(stderr, "ERROR: bindx_write_x()\n");
              return -1;
          }
     }

     if (options.f77) {
          if (bindx_write_x(&bindx_int, name_f77, n_out_files_f77, out_files_f77,
              bindx_write_f77)) {
              fprintf(stderr, "ERROR: bindx_write_x()\n");
              return -1;
          }
     }

     if (options.f90) {
          if (bindx_write_x(&bindx_int, name_f90, n_out_files_f90, out_files_f90,
              bindx_write_f90)) {
              fprintf(stderr, "ERROR: bindx_write_x()\n");
              return -1;
          }
     }

     if (options.idl) {
          if (bindx_write_x(&bindx_int, name_idl, n_out_files_idl, out_files_idl,
              bindx_write_idl)) {
              fprintf(stderr, "ERROR: bindx_write_x()\n");
              return -1;
          }
     }

     if (options.py) {
          if (bindx_write_x(&bindx_int, name_py, n_out_files_py, out_files_py,
              bindx_write_py)) {
              fprintf(stderr, "ERROR: bindx_write_x()\n");
              return -1;
          }
     }


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     bindx_free(&bindx_int);


     exit(0);
}



int bindx_write_x(const bindx_data *bindx_int, const char *name, int n_out_files,
                  char **out_files, int (*write)(FILE **, const bindx_data *,
                  const char *))
{
     int i;

     FILE *fp[MAX_IN_FILES];

     for (i = 0; i < n_out_files; ++i) {
          if ((fp[i] = fopen(out_files[i], "w")) == NULL) {
               fprintf(stderr, "ERROR: Problem opening file for writing: %s\n",
                       out_files[i]);
               fprintf(stderr, "  ... %s\n", strerror(errno));
               return -1;
          }
     }

     write(fp, bindx_int, name);

     for (i = 0; i < n_out_files; ++i)
          fclose(fp[i]);

     return 0;
}



void usage()
{

}
