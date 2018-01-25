#!/bin/bash -f
GFSH_PATH=""
which gfsh 2> /dev/null

if [ $? -gt 0 ]; then
    GFSH_PATH=`which gfsh 2> /dev/null`
fi


if [ "$GFSH_PATH" == "" ]; then
    if [ "$GEODE_HOME" == "" ]; then
        echo "Could not find gfsh. Please set the GEODE_HOME path."
        echo "e.g. export GEODE_HOME=<path to Geode>"
    else
        GFSH_PATH=$GEODE_HOME/bin/gfsh
    fi
fi

$GFSH_PATH  -e "start locator --name=locator" -e "start server --name=server"  -e "create region --name=orders --type=PARTITION"
w