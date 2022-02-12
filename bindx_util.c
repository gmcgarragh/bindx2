/*******************************************************************************
**
**    Copyright (C) 2011-2018 Greg McGarragh <greg.mcgarragh@colostate.edu>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#include <gutil.h>

#include "bindx_parse.h"
#include "bindx_util.h"


/*******************************************************************************
 *
 ******************************************************************************/
const char *bindx_version()
{
     return "0.01";
}



/*******************************************************************************
 *
 ******************************************************************************/
char *bindx_indent_str(int n)
{
     switch (n) {
          case 0:
               return "";
          case 1:
               return "     ";
          case 2:
               return "          ";
          case 3:
               return "               ";
          case 4:
               return "                    ";
          case 5:
               return "                         ";
          case 6:
               return "                              ";
          case 7:
               return "                                   ";
          case 8:
               return "                                        ";
          case 9:
               return "                                             ";
          default:
               INTERNAL_ERROR("Invalid indent length: %d", n);
     }

     return "";
}


char *bxis(int n) { return bindx_indent_str(n); }



/*******************************************************************************
 *
 ******************************************************************************/
void ifprintf(FILE *fp, int n_indent, const char *fmt, ...)
{
     va_list ap;

     fprintf(fp, "%s", bindx_indent_str(n_indent));

     va_start(ap, fmt);

     vfprintf(fp, fmt, ap);

     va_end(ap);
}


/*******************************************************************************
 *
 ******************************************************************************/
static char *write_string_trunc(FILE *fp, char *s, int n, char *prefix)
{
     int i;

     for (i = n; s[i] != ' ' && s[i] != '\t'; --i) ;

     for (     ; s[i] == ' ' || s[i] == '\t'; --i) ;

     s[i + 1] = '\0';

     fprintf(fp, "%s%s\n", prefix, s);

     for (i += 2; s[i] == ' ' || s[i] == '\t'; ++i) ;

     return s + i;
}



static void write_string_wrap(FILE *fp, char *s, int n, int n_wrap, char *prefix)
{
     char *s2;

     char *prefix2;

     int n_wrap2;

     int n_prefix = strlen(prefix);

     s2 = s;

     prefix2 = "";

     n_wrap2 = n_wrap;

     while (1) {
          if (n <= n_wrap) {
               fprintf(fp, "%s%s\n", prefix2, s2);
               return;
          }

          s2 = write_string_trunc(fp, s2, n_wrap2, prefix2);

          prefix2 = prefix;

          n_wrap2 = n_wrap - n_prefix;

          n -= s2 - s;
     }
}



void bindx_fprintf_wrap(FILE *fp, int n_wrap, char *prefix, const char *fmt, va_list ap)
{
     int j;
     int n;

     static int i = 0;

     static char buffer[1024];

     n = vsprintf(buffer + i, fmt, ap);

     for (j = 0; j < n; ++j) {
          if (buffer[i + j] == '\n') {
                buffer[i + j] = '\0';

                write_string_wrap(fp, buffer, i + j, n_wrap, prefix);

                memmove(buffer, buffer + i + j, n - j);

                i  = 0;

                n -= j + 1;
          }
     }

     i += n;
}
