#!/bin/bash

SOURCE_DIR=.
COUNT=`java -jar /apache-rat-0.12/apache-rat-0.12.jar -E ${SOURCE_DIR}/.ratignore -d ${SOURCE_DIR} | grep '== File:' | sed 's/== File://' | wc -l`

exit $COUNT

