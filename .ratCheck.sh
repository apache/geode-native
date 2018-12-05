#!/bin/bash

SOURCE_DIR=.
COUNT=`java -jar /apache-rat-0.12/apache-rat-0.12.jar -e ${SOURCE_DIR}/.ratignore -d ${SOURCE_DIR} | grep '== File:' | sed 's/== File://' | wc -l`

if [ $COUNT -gt 0 ]
    then
        echo "Rat check failed, $COUNT files are missing Apache license headers"
fi

exit $COUNT

