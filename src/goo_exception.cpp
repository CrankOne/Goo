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

# include <iostream>
# include <fstream>
# include <iomanip>
# include <sstream>

# include "goo_types.h"
# include "goo_exception.hpp"

# ifdef EM_STACK_UNWINDING
#   include <cxxabi.h>
#   include <execinfo.h>
#   include <unistd.h>
# endif
# include <cstdlib>
# include <cstring>
# include <cctype>
# include <cstdarg>

namespace goo {

bool
Exception::goo_raise( const ErrCode, const em::String & ) { return true; }

// Set default emraise behaviour to 'always throw':
bool (*Exception::user_raise)( const ErrCode,
                               const em::String & ) = Exception::goo_raise;

namespace em {

# ifdef EM_STACK_UNWINDING

void
seek_entry_in_section(  bfd * abfd,
                        asection * section,
                        void * entry_) {
    StackTraceInfoEntry & entry = *reinterpret_cast<StackTraceInfoEntry*>(entry_);
    if( entry.lineno ) { /* line found */ return; }
    if(!(bfd_get_section_flags(abfd, section) & SEC_ALLOC)) {
                         /*no debug info here*/ return; }
    bfd_vma section_vma = bfd_get_section_vma(abfd, section);
    if(entry.soLibAddr < section_vma) {
                        /* the addr lies above the section */ return;}
    bfd_size_type section_size = bfd_section_size(abfd, section);
    if(entry.soLibAddr >= section_vma + section_size) {
                        /* the addr lies below the section */ return; }
    // Calculate the correct offset of our line in the section
    bfd_vma offset = entry.soLibAddr - section_vma - 1;
    // Locate the line by offset
    const char * filename=NULL,
               * functionName=NULL;
    entry.lFound = bfd_find_nearest_line(
                    abfd,   section,    entry.sTable,
                    offset, &filename,  &functionName,
                    &entry.lineno );

    if( !entry.lFound ) {
        entry.failure = "Source lookup failed.";
    }

    entry.srcFilename = filename        ? filename : "";
    entry.function    = functionName    ? functionName : "";

}

static int
sup_positional_info( const String & path,
                     bfd_vma addr,
                     StackTraceInfoEntry & entry ) {
    // see: std::string addr2str(std::string file_name, bfd_vma addr)
    # ifndef NO_BFD_LIB
    bfd * abfd;
    abfd = bfd_openr(path.c_str(), NULL); {
        char ** matching; // TODO: clear?
        if( !abfd ) {
            entry.failure = "Couln't open " + path + "."; return -1;
        } if( bfd_check_format(abfd, bfd_archive) ) {
            entry.failure = "File " + path + " is not a BFD-compliant archive."; return -1;
        } if(!bfd_check_format_matches(abfd, bfd_object, &matching)) {
            entry.failure = "File " + path + " has not a BFD-compliant archive format."; return -1;
        } if((bfd_get_file_flags(abfd) & HAS_SYMS) != 0) {
            unsigned int symbolSize;
            void ** sTablePtr = (void**) &(entry.sTable);
            long nSymbols = bfd_read_minisymbols( abfd,         false,
                                                  sTablePtr,    &symbolSize );

            if( !nSymbols ) {
                // If the bfd_read_minisymbols() already allocated the table, we need
                // to free it first:
                if( entry.sTable != NULL )
                    free( entry.sTable );
                // dynamic
                nSymbols = bfd_read_minisymbols( abfd,          true,
                                                 sTablePtr,     &symbolSize );
            } else if( nSymbols < 0 ) {
                entry.failure = "bfd_read_minisymbols() failed.";
                return -1;
            }
        } else {
            entry.failure = "BFD archive " + path + " has no symbols.";
        }
        bfd_map_over_sections(abfd, seek_entry_in_section, &entry);
    } bfd_close(abfd);
    if( entry.sTable ) { free(entry.sTable); }
    # endif

    return 0;
}

static int
so_lib_callback(struct dl_phdr_info *info,
                               size_t size,
                               void *_data) {
    struct StackTraceInfoEntry * data = (struct StackTraceInfoEntry *) _data;
    for( int i=0; i < info->dlpi_phnum; i++ ) {
        if( info->dlpi_phdr[i].p_type == PT_LOAD ) {
            ElfW(Addr) min_addr = info->dlpi_addr + info->dlpi_phdr[i].p_vaddr;
            ElfW(Addr) max_addr = min_addr + info->dlpi_phdr[i].p_memsz;
            if ((data->addr >= min_addr) && (data->addr < max_addr)) {
                data->elfFilename = info->dlpi_name;
                data->soLibAddr = data->addr - info->dlpi_addr;
                // We found a match, return a non-zero value
                return 1;
            }
        }
    }
    // We didn't find a match, return a zero value
    return 0;
}

int
supplement_stacktrace( List & target, size_t n ) {
    # ifndef NO_BFD_LIB
    bfd_init();
    # endif
    for( auto it = target.begin(); it != target.end(); ++it ) {
        if( dl_iterate_phdr( so_lib_callback, &(*it) ) == 0) {
            it->failure = "dl_iterate_phdr() failed.";
            return -1;
        }
        if( it->elfFilename.length() > 0 ) {
            // This happens for shared libraries.
            // 'it->elfFilename' then contains the full path to the .so library.
            sup_positional_info(it->elfFilename, it->soLibAddr, *it);
        } else {
            // The 'addr_in_file' is from the current executable binary.
            // It can be debugger (gdb, valgrind, etc.), so:
            //pid_t pid = getpid();
            //char bf[32];
            //snprintf( bf, 32, "/proc/%d/exe", pid ); (TODO: not work)
            sup_positional_info( "/proc/self/exe", it->soLibAddr, *it);
        }
    }
    return 0;
}

/**@brief Obtains stacktrace info using binary file descriptor library library.
 * 0 -- ok
 */
int obtain_stacktrace( List & target ) {
    target.clear();
    void * stackPointers[GOO_EMERGENCY_STACK_DEPTH_NENTRIES];
    const size_t size = backtrace(
            stackPointers,
            GOO_EMERGENCY_STACK_DEPTH_NENTRIES );
    for( Size i = 1; i < size; ++i ) {  // Note: hides own entries
        StackTraceInfoEntry entry {
                # if 0 /* unsupported by GCC */
                .addr       = (bfd_vma) stackPointers[i],
                .soLibAddr  = 0, .lFound = 0, .lineno     = 0,
                "",         "",         "",         "",
                # else
                0, 0,
                "",         "",         "",         "",
                # endif
                # ifndef NO_BFD_LIB
                (bfd_vma) stackPointers[i],
                0,
                0,
                # endif
            };
        target.push_front( entry );
    }
    return supplement_stacktrace( target, size );
}

String demangle_class( const char * classname ) {
    if( (!classname) || (*classname == '\0') ) {
        return "<null>";
    } else {
        int status = -4;
        char *d = 0;
        d = abi::__cxa_demangle(classname, 0, 0, &status);
        if( d ) {
            String s(d);
            free(d);
            if( s.size() > 0 ) {
                return s;
            } else {
                return classname;
            }
        } else {
            return classname;
        }
    }
}

String demangle_function( const String & name ) {
    String s;
    if( name.length() == 0 ) {
        s = "??";
    } else {
        int status = 0;
        char *d = 0;
        d = abi::__cxa_demangle(name.c_str(), 0, 0, &status);
        if (d) {
            s = d;
            free(d);
            if (s.size() > 0) {
                if (s[s.size()-1] != ')') {
                    // This means that the C++ demangler failed for some
                    // reason, as each symbol needs to end with ")". As such,
                    // just return the original version:
                    s = name + "()";
                }
            }
        } else {
            s = name + "()";
        }
    }

    return s;
}

std::ostream & operator<<(std::ostream& os, const StackTraceInfoEntry & t) {
    if( !t.failure.empty() ) os << " / "
                                   << ESC_BLDRED << t.failure
                                   << ESC_CLRCLEAR << " / ";
    if( !t.elfFilename.empty() ) os << t.elfFilename;
    else os << "??";
    os << " / ";
    os << "0x" << std::hex << std::setw(8) << std::setfill('0') << t.addr << " "
       << "0x" << std::hex << std::setw(8) << std::setfill('0') << t.soLibAddr << " "
       << std::dec;
    if( !t.srcFilename.empty() ) {
        os << "\n\t\tat "
           << ESC_CLRGREEN << t.srcFilename << ESC_CLRCLEAR << ":"
           << ESC_BLDGREEN << t.lineno << ESC_CLRCLEAR << ",";
    }
    os << "\n\t\tfunction " << ESC_BLDWHITE << demangle_function( t.function ) << ESC_CLRCLEAR
       ;
    return os;
}

# endif  // EM_STACK_UNWINDING

}  // namespace em (emergency)

# define declare_dict_entry(num, nm, dscr) { num, dscr },

struct __ExceptionDescrDictionary {
    ErrCode _c;
    const char * _descr;
};

__ExceptionDescrDictionary __jDict[] = {
    for_all_statuscodes( declare_dict_entry )
    { 0, nullptr }
};

# define declare_static_const(num, nm, dscr) \
const ErrCode Exception::nm = num;
    for_all_errorcodes( declare_static_const )
# undef declare_static_const

Exception::Exception(
                const ErrCode c,
                const em::String & s
                    ) : _code(c),
                        _what(s) {
    # ifdef EM_STACK_UNWINDING
    _get_trace();
    # endif  // EM_STACK_UNWINDING
}

Exception::~Exception() throw() {
}

Exception::Exception( const em::String & s ) : _code(0),
                                               _what(s){
    # ifdef EM_STACK_UNWINDING
    _get_trace();
    # endif  // EM_STACK_UNWINDING
}

# ifdef EM_STACK_UNWINDING 

void
Exception::_get_trace() throw() {
    em::obtain_stacktrace( _stacktrace );
}

# endif  // EM_STACK_UNWINDING 

void
Exception::dump(std::ostream & os) const throw() {
    os  << std::endl << ESC_BLDYELLOW
        << std::setw(15) << std::right << "exception code"
        << ESC_CLRCLEAR ": " ESC_BLDRED << (int) _code << ESC_CLRCLEAR;
    os  << " (" ESC_BLDWHITE
        << get_errcode_description(_code) << ESC_CLRCLEAR")" << std::endl;
    os  << ESC_BLDYELLOW << std::setw(15) << std::right << "details" << ESC_CLRCLEAR": "
        << _what << std::endl;

    # ifdef EM_STACK_UNWINDING
    os << ESC_BLDYELLOW << std::setw(15) << std::right << "stacktrace" << ESC_CLRCLEAR":"
       << " (most recent call last)"
       << std::endl;
    //Size maxStackFrameHeadLen = GOO_EMERGENCY_STACK_DEPTH_NENTRIES;
    Size stackFrameHeadLen = 0;
    for( auto it = _stacktrace.begin();
                   _stacktrace.end() != it; ++it) {
        os << "\t#" << ++stackFrameHeadLen - 1 << " " << *it << std::endl;
        
        //if( ! GOO_EMERGENCY_STACK_DEPTH_NENTRIES && (stackFrameHeadLen > maxStackFrameHeadLen) ) {
        //    os << "\t\t...(and " << _stacktrace.size() - stackFrameHeadLen << " entries more)..." << std::endl;
        //    break;
        //}
    }
    # else
    os << "Stack unwinding is unavailable in current build." << std::endl;
    # endif  // EM_STACK_UNWINDING
}

}  // namespace goo

const char *
get_errcode_description( const ErrCode C ) {
    for( goo::__ExceptionDescrDictionary * c = goo::__jDict;
        nullptr != c->_descr; ++c ) {
        if( C == c->_c ) {
            return c->_descr;
        }
    }
    return "unhandled custom error code";
}

//
// Backtrace
//

extern "C" {

//
// C bindings
//

# define define_error_code_C_alias( code, name, descr ) \
extern const ErrCode goo_e_ ## name = code;
for_all_statuscodes( define_error_code_C_alias )
# undef define_error_code_C_alias

int
goo_C_error( ErrCode code, const char * fmt, ... ) {
    char emBuf[GOO_EMERGENCY_BUFLEN];
    va_list argptr;
    va_start(argptr, fmt);
    snprintf(emBuf, GOO_EMERGENCY_BUFLEN, fmt, argptr);
    va_end(argptr);
    throw goo::Exception( code, emBuf );
}

}  // extern "C"

