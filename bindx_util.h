/*******************************************************************************
**
**    Copyright (C) 2011-2018 Greg McGarragh <gregm@atmos.colostate.edu>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#ifndef BINDX_UTIL_H
#define BINDX_UTIL_H

#include <gutil.h>

#ifdef __cplusplus
extern "C" {
#endif


#define internal_compiler_error(text) do {			\
    fprintf(stderr, "Internal compiler error: %s:%d, %s\n",	\
            __FILE__, __LINE__, text);				\
    exit(1);							\
} while (0)


#include "prototypes/bindx_util_p.h"


#ifdef __cplusplus
}
#endif

#endif /* BINDX_UTIL_H */
