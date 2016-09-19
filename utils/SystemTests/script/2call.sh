#!/bin/sh

# This script should be invoked by goo's execl()-wrapper.
# Basically, following cases are considered by this script
# where returning value is a number of case:
# 1. (trivial) Invokation of script without any arguments or
#   environment variables specified.
# 2. Invokation of script with argument "--arg1" set to "one"
#   without environment variable.
# 3. Invokation of script without arguments and with "TEST_ENVVAR1"
#   set to "one".
# 4. Invokation of this script with arguments:
#   --arg1=two
#   --arg2=three
#   and environment variables:
#   TEST_ENVVAR1=two
#   TEST_ENVVAR2=three
# If arguments or environmental variables will be set to unexpected values or,
# either not set at all the -10 will be returned.

if [ $# -eq 0 ]; then
    echo "> No args given. Assuming case 1 or 3."
    if [ "$TEST_ENVVAR1" != "one" ]; then
        echo "> Env var \$TEST_ENVVAR is not set to one. Checking for case 1."
        if [ -z "$TEST_ENVVAR1" ] && [ -z "$TEST_ENVVAR2" ]; then
            echo "> Both envvars not set. Case 1 confirmed. Returning 1."
            exit 1
        else
            >&2 echo "! Wrong env vars set: [1]=\"$TEST_ENVVAR1\", [2]=\"$TEST_ENVVAR2\""
            exit -10
        fi
    else
        echo "> env var [1]=\"$TEST_ENVVAR1\", assuming case 3."
        if [ -z "$TEST_ENVVAR2" ] ; then
            echo "> Case 3 confirmed."
            exit 3
        fi
    fi
elif [ $# -eq 2 ] ; then
    echo "> two cmd-line args provided; assuming case 2."
    if [ "$1" == "--arg1" ] &&
       [ "$2" == "one" ] &&
       [ -z "$TEST_ENVVAR1" ] &&
       [ -z "$TEST_ENVVAR2" ] ; then
       echo "> Case 2 confirmed"
       exit 2
    else
        >&2 echo "! Wrong invokation: \"$ $0 $1 $2\" w env vars [1]=\"$TEST_ENVVAR1\", [2]=\"$TEST_ENVVAR2\""
    fi
elif [ $# -eq 4 ] ; then
    echo "> Got four cmd-line args, assuming case 4."
    if [ "$1" == "--arg1" ] &&
       [ "$2" == "two" ] &&
       [ "$3" == "--arg2" ] &&
       [ "$4" == "three" ] &&
       [ "$TEST_ENVVAR1" == "two" ] &&
       [ "$TEST_ENVVAR2" == "three" ] ; then
       echo "> Case 4 confirmed."
       exit 4
    else
        >&2 echo "! Wrong invokation: \"$ $0 $1 $2\" w env vars [1]=\"$TEST_ENVVAR1\", [2]=\"$TEST_ENVVAR2\""
        exit -10
    fi
fi

