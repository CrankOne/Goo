# ifndef H_GOO_ANSI_ESCAPE_SEQUENCES_H
# define H_GOO_ANSI_ESCAPE_SEQUENCES_H

/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

# include "goo_config.h"
/*
 * Shell colors
 */

# ifdef ANSI_ESCSEQ_PRINT

# define ESC_CLRBOLD    "\033[0;1m"
# define ESC_CLRITALIC  "\033[0;3m"
# define ESC_CLRUNDRLN  "\033[0;4m"

# define ESC_BLDRED     "\033[1;31m"
# define ESC_BLDGREEN   "\033[1;32m"
# define ESC_BLDYELLOW  "\033[1;33m"
# define ESC_BLDBLUE    "\033[1;34m"
# define ESC_BLDVIOLET  "\033[1;35m"
# define ESC_BLDCYAN    "\033[1;36m"
# define ESC_BLDWHITE   "\033[1;37m"

# define ESC_CLRRED     "\033[0;31m"
# define ESC_CLRGREEN   "\033[0;32m"
# define ESC_CLRYELLOW  "\033[0;33m"
# define ESC_CLRBLUE    "\033[0;34m"
# define ESC_CLRVIOLET  "\033[0;35m"
# define ESC_CLRCYAN    "\033[0;36m"
# define ESC_CLRWHITE   "\033[0;37m"

# define ESC_UNLRED     "\033[4;31m"
# define ESC_UNLGREEN   "\033[4;32m"
# define ESC_UNLYELLOW  "\033[4;33m"
# define ESC_UNLBLUE    "\033[4;34m"
# define ESC_UNLVIOLET  "\033[4;35m"
# define ESC_UNLCYAN    "\033[4;36m"
# define ESC_UNLWHITE   "\033[4;37m"

# define ESC_CLRCLEAR   "\033[0m"

# else /* ANSI_ESCSEQ_PRINT */

# define ESC_CLRBOLD    ""
# define ESC_CLRITALIC  ""
# define ESC_CLRUNDRLN  ""

# define ESC_BLDRED     ""
# define ESC_BLDGREEN   ""
# define ESC_BLDYELLOW  ""
# define ESC_BLDBLUE    ""
# define ESC_BLDVIOLET  ""
# define ESC_BLDCYAN    ""
# define ESC_BLDWHITE   ""

# define ESC_CLRRED     ""
# define ESC_CLRGREEN   ""
# define ESC_CLRYELLOW  ""
# define ESC_CLRBLUE    ""
# define ESC_CLRVIOLET  ""
# define ESC_CLRCYAN    ""
# define ESC_CLRWHITE   ""

# define ESC_UNLRED     ""
# define ESC_UNLGREEN   ""
# define ESC_UNLYELLOW  ""
# define ESC_UNLBLUE    ""
# define ESC_UNLVIOLET  ""
# define ESC_UNLCYAN    ""
# define ESC_UNLWHITE   ""

# define ESC_CLRCLEAR   ""

# endif  /* ANSI_ESCSEQ_PRINT */
# endif  /* H_GOO_ANSI_ESCAPE_SEQUENCES_H */
