/******************************************************************************%
**
**    Copyright (C) 2011-2017 Greg McGarragh <gregm@atmos.colostate.edu>
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
void parse_error(locus_data *locus, const char *format, ...)
{
     char format2[1024];

     va_list ap;

     snprintf(format2, 1024, "ERROR: %s:%d:%d, %s\n", locus->file, locus->line,
              locus->character, format);

     va_start(ap, format);

     vfprintf(stderr, format2, ap);

     va_end(ap);

     exit(1);
}



/*******************************************************************************
 *
 ******************************************************************************/
void internal_error(const char *file, int line, const char *format, ...)
{
     char format2[1024];

     va_list ap;

     snprintf(format2, 1024, "INTERNAL ERROR: %s:%d, %s\n", file, line, format);

     va_start(ap, format);

     vfprintf(stderr, format2, ap);

     va_end(ap);

     exit(1);
}
