#!/bin/bash
# (c) Kurt Pfeifle <kpfeifle-eG1X/KtG1iM@xxxxxxxxxxxxxxxx>
# <pfeifle-RoXCvvDuEio@xxxxxxxxxxxxxxxx>
# Script is GPL and free to use and modify
#
# Proxy filter for CUPS to harvest intermediate
# files from each step in the filtering chain
#
#
# log a lot of stuff which may be interesting for us
# so we learn how CUPS works.... ;-)
LOG=/tmp/cupsproxyfilter.log
echo "=======================================================" >> $LOG
date >> $LOG
echo "=======================================================" >> $LOG
echo "I am running under this user ID:" >> $LOG
id >> $LOG
echo "=======================================================" >> $LOG
env >> $LOG
echo C $# >> $LOG
echo 0 $0 >> $LOG
echo 1 $1 >> $LOG
echo 2 $2 >> $LOG
echo 3 $3 >> $LOG
echo 4 $4 >> $LOG
echo 5 $5 >> $LOG
echo 6 $6 >> $LOG

# create a directory to store the output of each single filter
# under a uniq name for later debugging purpose
[ -d /tmp/cups-filter-outputs ] || mkdir /tmp/cups-filter-outputs

# in case of wrong number of arguments: print usage hint
if test "$#" -eq "0"; then
echo "ERROR: `basename $0` job-id user title copies options [file]"
1>&2
exit 0
fi

if test "$#" -lt "5"; then
echo "ERROR: Number of arguments ($#) is wrong" 1>&2
echo "USAGE: `basename $0` job-id user title copies options [file]"
1>&2
exit 1
fi

if test "$#" -gt "6"; then
echo "ERROR: Number of arguments ($#) is wrong" 1>&2
echo "USAGE: `basename $0` job-id user title copies options [file]"
1>&2
exit 1
fi

if test "$#" -eq "5"; then
INPUT="-"
else
INPUT="$6"
fi

echo $0.bin "$@" >> $LOG
echo "$#" >> $LOG
echo `id` >> $LOG
exec $0.bin "$@" | tee /tmp/cups-filter-outputs/JobID-$1-$3-`basename
$0`-output