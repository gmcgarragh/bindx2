/******************************************************************************%
**
**    Copyright (C) 1998-2017 Greg McGarragh <gregm@atmos.colostate.edu>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#include "gutil.h"


#define EXT_BUF 256


void str_replace_char(char *s, char c1, char c2) {

     while ((s = strchr(s+1, c1)))
          *s++ = c2;
}



int gstrtob(const char *s, int *x) {

     int i;

     for (i = 0; s[i]; ++i) {
          if (!isspace((int) s[i])) {
               break;
          }
     }

     switch(s[i]) {
          case '0':
               *x = 0;
               break;
          case '1':
               *x = 1;
               break;
          case 'f':
               *x = 0;
               break;
          case 'F':
               *x = 0;
               break;
          case 't':
               *x = 1;
               break;
          case 'T':
               *x = 1;
               break;
          default:
               *x = 0;
               return -1;
     }

     if (s[i+1]) {
          if (s[i] == 'f' || s[i] == 'F') {
                if (s[i+5]) {
                      *x = 0;
                      return -1;
                }

                switch(s[i+1]) {
                     case 'a':
                          break;
                     case 'A':
                          break;
                     default:
                          *x = 0;
                          return -1;
                }
                switch(s[i+2]) {
                     case 'l':
                          break;
                     case 'L':
                          break;
                     default:
                          *x = 0;
                          return -1;
                }
                switch(s[i+3]) {
                     case 's':
                          break;
                     case 'S':
                          break;
                     default:
                          *x = 0;
                          return -1;
                }
                switch(s[i+4]) {
                     case 'e':
                          break;
                     case 'E':
                          break;
                     default:
                          *x = 0;
                          return -1;
                }
          }
          else if (s[i] == 't' || s[i] == 'T') {
                if (s[i+4]) {
                      *x = 0;
                      return -1;
                }

                switch(s[i+1]) {
                     case 'r':
                          break;
                     case 'R':
                          break;
                     default:
                          *x = 0;
                          return -1;
                }
                switch(s[i+2]) {
                     case 'u':
                          break;
                     case 'U':
                          break;
                     default:
                          *x = 0;
                          return -1;
                }
                switch(s[i+3]) {
                     case 'e':
                          break;
                     case 'E':
                          break;
                     default:
                          *x = 0;
                          return -1;
                }
          }
          else {
               *x = 0;
               return -1;
          }
     }

     return 0;
}



const char *gbtostr(int x) {

     if (x)
          return "true";
     else
          return "false";
}



#define gstrtol_frame(postfix_, function_, type_)	\
int XCAT(gstrto, postfix_)(const char *nprt, type_ *x) {\
							\
     char *endptr;					\
							\
     errno = 0;						\
							\
     if (nprt == NULL)					\
          return -1;					\
							\
     if (*nprt == '\0')					\
          return -1;					\
							\
     *x = (type_) function_(nprt, &endptr, 10);		\
							\
     if (errno == ERANGE)				\
          return -1;					\
							\
     if (*endptr != '\0')				\
          return -1;					\
							\
     return 0;						\
}

gstrtol_frame(us,  strtol,  ushort)
gstrtol_frame(s,   strtol,  short)
gstrtol_frame(ui,  strtol,  uint)
gstrtol_frame(i,   strtol,  int)
gstrtol_frame(ul,  strtol,  ulong)
gstrtol_frame(l,   strtol,  long)
#if PLATFORM == WIN32_MSVC
gstrtol_frame(ull, _strtoi64, uint64_t)
gstrtol_frame(ll,  _strtoi64, int64_t)
#else
gstrtol_frame(ull, strtoll, uint64_t)
gstrtol_frame(ll,  strtoll, int64_t)
#endif


#define gstrtod_frame(postfix_, function_, type_)	\
int XCAT(gstrto, postfix_)(const char *nprt, type_ *x) {\
							\
     char *endptr;					\
							\
     errno = 0;						\
							\
     if (nprt == NULL)					\
          return -1;					\
							\
     if (*nprt == '\0')					\
          return -1;					\
							\
     *x = (type_) function_(nprt, &endptr);		\
							\
     if (errno == ERANGE)				\
          return -1;					\
							\
     if (*endptr != '\0')				\
          return -1;					\
							\
     return 0;						\
}

gstrtod_frame(f,   strtod,  float)
gstrtod_frame(d,   strtod,  double)



char *gtrtok_r(char *s, const char *delim, char **lasts) {

     char *token;

     char *delim_p;
     char *lasts_p;

     if (s)
          *lasts = s;

     if (*lasts == NULL)
           return  NULL;

     lasts_p = *lasts;

     for ( ; *lasts_p; ++lasts_p) {
          if (*lasts_p == '\'') {
               delim = "\'";
               ++lasts_p;
               break;
          }
          if (*lasts_p == '\"') {
               delim = "\"";
               ++lasts_p;
               break;
          }

          for (delim_p = (char *) delim; *delim_p; ++delim_p) {
               if (*lasts_p == *delim_p)
                    break;
          }

          if (*lasts_p != *delim_p)
               break;
     }

     if (*lasts_p == '\0') {
          *lasts = NULL;
           return  NULL;
     }

     token = lasts_p;

     for ( ; *lasts_p; ++lasts_p) {
          for (delim_p = (char *) delim; *delim_p; ++delim_p) {
               if (*lasts_p == *delim_p) {
                    goto L1;
               }
          }
     }

L1:
     if (*lasts_p) {
          *lasts_p  = '\0';
          *lasts = lasts_p;
          (*lasts)++;
     }
     else
          *lasts = NULL;

     return token;
}



char *get_token(int n, char *delim, char **lasts) {

     char *token = NULL;

     int i;

     for (i = 0; i < n; ++i) {
          token = gtrtok_r(NULL, delim, lasts);
          if (token == NULL)
               return NULL;
     }

     return token;
}



char *get_token_init(char *s, int n, char *delim, char **lasts) {

     char *token;

     int i;

     *lasts = NULL;

     token = gtrtok_r(s, delim, lasts);

     for (i = 1; i < n; ++i) {
          token = gtrtok_r(NULL, delim, lasts);
          if (token == NULL)
               return NULL;
     }

     return token;
}



int count_tokens(char *line, char *delim) {

     int i, n, count;

     int inside;

     n = strlen(line);

     count = 0;
     inside = 1;
     for (i = 0; i < n; ++i) {
          if (strchr(delim, line[i])) {
               inside = 1;
               continue;
          }
          if (inside == 1) {
               inside = 0;
               ++count;
          }
     }

     return count;
}



int tokenize(char *line, char **tokens, char *delim) {

     char *curtok;
     char *lasts;

     int i = 0;

     lasts = NULL;

     if ((tokens[i++] = gtrtok_r(line, delim, &lasts)) == NULL)
          return 0;

     while((curtok = gtrtok_r(NULL, delim, &lasts)) != NULL)
          tokens[i++] = curtok;

     return i;
}



char *remove_pad(char *s, int side) {

     int i;

     if (side == 0 || side == 2) {
          for (i = 0; s[i] == ' '; ++i) ;
          s = s + i;
     }
     if (side == 1 || side == 2) {
          for (i = strlen(s) - 1; s[i] == ' '; --i) ;
          s[i+1] = '\0';
     }

     return s;
}



char *int_to_string(int i, char *s) {

     sprintf(s, "%d", i);

     return s;
}



char *uint_to_string(uint ui, char *s) {

     sprintf(s, "%u", ui);

     return s;
}



char *long_to_string(long l, char *s) {

     sprintf(s, "%ld", l);

     return s;
}



char *ulong_to_string(ulong ul, char *s) {

     sprintf(s, "%lu", ul);

     return s;
}


char *float_to_string(float f, char *s) {

     sprintf(s, "%f", f);

     return s;
}


char *double_to_string(double d, char *s) {

     sprintf(s, "%f", d);

     return s;
}



int strcmp2(const void *v1, const void *v2) {

     char  *s1;
     char **s2;

     s1 = (char  *) v1;
     s2 = (char **) v2;

     return strcmp(s1, *s2);
}



int strcmp3(const void *v1, const void *v2) {

     char **s1;
     char **s2;

     s1 = (char **) v1;
     s2 = (char **) v2;

     return strcmp(*s1, *s2);
}



/* "min string length" strcmp() */

int mincmp(const void *v1, const void *v2) {

     char *s1;
     char *s2;

     s1 = (char *) v1;
     s2 = (char *) v2;

     for ( ; *s1 == *s2; ++s1, ++s2) {
          if (*s1 == '\0')
               return 0;
     }

     return !*s1 || !*s2 ? 0 : *s1 - *s2;
}



int minncmp(const void *v1, const void *v2, int n) {

     char *s1;
     char *s2;

     int i;

     s1 = (char *) v1;
     s2 = (char *) v2;

     for (i = 0; s1[i] == s2[i]; ) {
          if (s1[i] == '\0' || ++i == n)
               return 0;
     }

     return !s1[i] || !s2[i] ? 0 : s1[i] - s2[i];
}



/* "not case sensitive" strcmp() */

int ncscmp(const void *v1, const void *v2) {

     char c1;
     char c2;

     char *s1;
     char *s2;

     s1 = (char *) v1;
     s2 = (char *) v2;

     for ( ; (c1 = (char) tolower(*s1)) == (c2 = (char) tolower(*s2)); ++s1, ++s2) {
          if (c1 == '\0')
               return 0;
     }

     return c1 - c2;
}



int ncsncmp(const void *v1, const void *v2, int n) {

     char c1;
     char c2;

     char *s1;
     char *s2;

     int i;

     s1 = (char *) v1;
     s2 = (char *) v2;

     for (i = 0; (c1 = (char) tolower(s1[i])) == (c2 = (char) tolower(s2[i])); ) {
          if (c1 == '\0' || ++i == n)
               return 0;
     }

     return c1 - c2;
}



void strtolower(const char *s1, char *s2) {

     while(*s1)
          *(s2++) = (char) tolower(*(s1++));

     *s2 = '\0';
}



void strtoupper(const char *s1, char *s2) {

     while(*s1)
          *(s2++) = (char) toupper(*(s1++));

     *s2 = '\0';
}
