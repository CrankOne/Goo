#!/bin/bash
#
# This script provides a set of operations to build the CMake project.
#
# SYNOPSIS
#   $ runbuild.sh --loglevel 1 --srcdir=../../p348g4/ cmakeopts: \
#           -DCMAKE_MODULE_PATH=/home/crank/Projects/goo.install/share/cmake/Modules/
#
# DESCRIPTION
#   Despite build process of CMake project ideally requires a quite
# neat and laconic commands:
#   $ cmake
#   $ make
# we often need a fine control over the build process by providing
# additional options. In case of walkthrough test build --- a lot of
# options, so the designation of this script is to route those options
# between three stages: cmake'ing, build and run of
# system/unit tests (that is currently unavailable --- TODO).
#
# TODO
#   - usage documentation
#   - support for system/unit tests

SECONDS=0

mkdir_u=mkdir
cmake_u=/usr/bin/cmake
make_u=/usr/bin/make
cp_u=/bin/cp

LOGLEVEL=full  # possible values: full/inline/quiet
BUILDDIRBASE=/tmp
SOURCEDIR=''
DSTRNAME=sources
BUILDCFGNAMETAG=custom
MAKEOPTS=()
CMAKEOPTS=()
ARGV=()
FTCODE=-1
HEX_FTCODE=0
DRY_RUN=false

# Utility function --- checks, if key exists in associative array:
# $1 key
# $2 must be 'at'
# $3 associative array
exists(){
  if [ "$2" != at ]; then
    echo "Incorrect usage."
    echo "Correct usage: exists {key} in {array}"
    return
  fi   
  eval '[ ${'$3'[$1]+one} ]'  
}
#
contains_element() {
  local e
  for e in "${@:2}"; do [[ "$e" == "$1" ]] && return 0; done
  return 1
}
#
function return_script_error {
    exit 1
}
#
function evacuate_logs {
    if [ "$FTCODE" -ne -1 ] ; then
        $cp_u "$BUILDDIR/build.log"  "$BUILDDIR/../$HEX_FTCODE.build.log"
        $cp_u "$BUILDDIR/errors.log" "$BUILDDIR/../$HEX_FTCODE.errors.log"
    fi
}
#
function on_sigint {
    echo "(child script) Trapped SIGINT!"
    exit 10
}
#
function message() {
    if [ "$LOGLEVEL" == "full" ] ; then
        echo -e "\033[1;32m[MSG]\033[0m $1";
    fi
}
#
function set_loglevel() {
    if [ "$val" == "0" ] ; then
        LOGLEVEL=quiet;
    elif  [ "$val" == "1" ] ; then
        LOGLEVEL=inline;
    elif  [ "$val" == "2" ] ; then
        LOGLEVEL=full;
    fi
    message "Loglevel is set: '--${OPTARG}', value: '${val}'";
}
#
function set_target_dir() {
    if [ ! -d "$1" ] ; then
        if [ ! -z "$1" ] ; then
            echo "Target source directory $1 does not exist."
        else
            echo 'One must specify taget source directory via `--srcdir <PATH>` option.'
        fi
    else
        SOURCEDIR=`realpath $1`
        message "Setting source directory:'$SOURCEDIR'"
    fi
}
#
function set_build_dir() {
    if [ ! -d "$1" ] ; then
        if [ ! -z "$1" ] ; then
            echo "Build directory $1 does not exist."
        else
            echo 'One must specify build directory via `--blddir <PATH>` option.'
        fi
    else
        BUILDDIRBASE=`realpath $1`
        message "Setting build directory:'$BUILDDIRBASE'"
    fi
}

trap return_script_error ERR
trap on_sigint INT

#
# ENTRY POINT
###############################################################################
#  - separate cmd-line arguments: own, cmake, make
args=("$@")
cmdArgsMode=0
for arg in ${args[*]} ; do
    if [[ $arg =~ ^cmakeopts:$ ]] ; then
        cmdArgsMode=1
        continue
    elif [[ $arg =~ ^makeopts:$ ]] ; then
        cmdArgsMode=2
        continue
    fi

    if [ $cmdArgsMode -eq 0 ] ; then
        # script options
        #echo ": $arg"
        ARGV+=($arg)
    elif [ $cmdArgsMode -eq 1 ] ; then
        # CMake options
        CMAKEOPTS+=($arg)
    else
        # GNU make options
        MAKEOPTS+=($arg)
    fi
done

#  - parse own cmd-line arguments:
optspec="df:i:C:-:"
while getopts "$optspec" optchar ${ARGV[@]}; do
    case "${optchar}" in
        -)
            case "${OPTARG}" in
                srcdir=*)
                    val=${OPTARG#*=}
                    opt=${OPTARG%=$val}
                    set_target_dir $val
                    ;;
                srcdir)
                    val="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    set_target_dir $val
                    ;;
                blddir=*)
                    val=${OPTARG#*=}
                    opt=${OPTARG%=$val}
                    BUILDDIRBASE=$val
                    ;;
                blddir)
                    val="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    BUILDDIRBASE=$val
                    ;;
                loglevel)
                    val="${!OPTIND}"; OPTIND=$(( $OPTIND + 1 ))
                    set_loglevel $val
                    ;;
                loglevel=*)
                    val=${OPTARG#*=}
                    opt=${OPTARG%=$val}
                    set_loglevel $val
                    ;;
                cmakeopts)
                    echo "CMAKE: ${OPTARG#*=}"
                    ;;
                *)
                    if [ "$OPTERR" = 1 ] && [ "${optspec:0:1}" != ":" ]; then
                        echo -e "\033[1;31m[EE]\033[0m Unknown option --${OPTARG}" >&2
                    fi
                    ;;
            esac;;
        d)
            DRY_RUN=true
            ;;
        i)
            val="$OPTARG"; OPTIND=$(( $OPTIND + 1 ))
            set_target_dir $val
            ;;
        C)
            val="$OPTARG"; OPTIND=$(( $OPTIND + 1 ))
            if [ -z $val ]; then
                    echo -e "\033[1;31m[EE]\033[0m Incorrect empty build-config parameter. Keep '$BUILDCFGNAMETAG'." >&2
                    exit 1
                else
                    BUILDCFGNAMETAG=$val;
                    message "Config nametag is set to '$BUILDCFGNAMETAG'."
            fi
            ;;
        f)
            #echo "#### $FTCODE <<<<<<<<<<<<<<<<<";
            FTCODE="$OPTARG"; #OPTIND=$(( $OPTIND + 1 ))
            HEX_FTCODE=$(echo "obase=16; $FTCODE" | bc)
            #echo ">>>>>>>>>>>>>>>>> $FTCODE <<<<<<<<<<<<<<<<<";
            ;;
        \?)
            echo "\
usage: $0 [-v] [--loglevel[=]<value>] [--builddir=<path=/tmp>] [--cmakeopts=<cmakeopts>]" >&2
            exit 1
            ;;
        *)
            if [ "$OPTERR" != 1 ] || [ "${optspec:0:1}" = ":" ]; then
                echo -e "\033[1;31m[EE]\033[0m Non-option argument: '-${OPTARG}'" >&2
                exit 1
            fi
            ;;
    esac
done

set_build_dir $BUILDDIRBASE


DSTRNAME=`basename $SOURCEDIR`
message "Tag (naming infix) for distributive:'$DSTRNAME'"
BUILDDIR=$BUILDDIRBASE/$DSTRNAME.build/$BUILDCFGNAMETAG/
message "Concrete build directory:'$BUILDDIR'"
$mkdir_u -p $BUILDDIR
message "Build directory created:'$BUILDDIR'"

#  - now, build:
buildlog=$BUILDDIR/build.log
builderr=$BUILDDIR/errors.log
cd $BUILDDIR
rm -rf ./*
#  -- CMake
if [ "inline" == $LOGLEVEL ] ; then
    echo -ne "\033[scmake ..."
    echo -e "Performing:\n\t\033[1;32m$\033[0m $cmake_u ${CMAKEOPTS[@]}" $SOURCEDIR >> $buildlog
    echo -e "Performing:\n\t\033[1;32m$\033[0m $cmake_u ${CMAKEOPTS[@]}" $SOURCEDIR >> $builderr
    if [ "$DRY_RUN" = true ] ; then
        exit 9  # Dry run exit
    fi
    trap " evacuate_logs ; exit 2 " ERR
    $cmake_u ${CMAKEOPTS[@]} $SOURCEDIR 1>>$buildlog 2>>$builderr
    if [ $? -eq  0 ] ; then
        printf "\b\b\b\b:ok"
    else
        evacuate_logs
        printf "\b\b\b\b:failure ($SECONDS sec)"
        exit 2
    fi
    trap return_script_error ERR
elif [ "full" == $LOGLEVEL ] ; then
    echo -e "Performing:\n\t\033[1;32m$\033[0m $cmake_u $SOURCEDIR ${CMAKEOPTS[@]}"
    $cmake_u $SOURCEDIR ${CMAKEOPTS[@]}
    if [ $? -ne 0 ] ; then
        evacuate_logs
        echo -e "\033[1;31m[EE]\033[0m CMake error ($SECONDS sec)"
        exit 2
    fi
fi
#  -- make
if [ "inline" == $LOGLEVEL ] ; then
    printf ", compiling ..."
    trap " evacuate_logs ; exit 3 " ERR
    $make_u ${MAKEOPTS[@]} 2>>$builderr | gawk -vbuildlog="$buildlog" 'match($0, /\[\s(.*)].*/, a) {printf "\b\b\b\033[1;36m" a[1] ; print "$0\033[0m" >> buildlog; fflush(stdout)}'
    if [ ${PIPESTATUS[0]} -eq  0 ] ; then
        printf "\b\b\b\b:ok"
    else
        evacuate_logs
        printf "\b\b\b\b:failure ($SECONDS sec)"
        exit 3
    fi
    trap return_script_error ERR
elif [ "full" == $LOGLEVEL ] ; then
    echo -e "Performing:\n\t\033[1;32m$\033[0m $make_u ${MAKEOPTS[@]}"
    $make_u ${MAKEOPTS[@]}
    if [ $? -ne 0 ] ; then
        evacuate_logs
        echo -e "\033[1;31m[EE]\033[0m Make error ($SECONDS sec)"
        exit 3
    fi
fi

# TODO here be system/unit tests

# TODO cleanup

if [ "inline" == $LOGLEVEL ] ; then
    printf "\033[uSucceed ($SECONDS sec)"
elif [ "full" == $LOGLEVEL ] ; then
    printf "Succeed ($SECONDS sec)"
fi

