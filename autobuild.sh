#!/bin/sh -x 

cd `dirname $0`

HPCNAME=`./bin/hpcname -n`
TEST_LOG="ipm-autobuild-$HPCNAME.log"
TEST_EMAIL="deskinner@lbl.gov"

rm -rf $TEST_LOG
echo "autobuild: HPCNAME="$HPCNAME | tee -a $TEST_LOG
echo TEST_BEGIN `date`  | tee -a $TEST_LOG
env | tee -a $TEST_LOG 
./configure | tee -a $TEST_LOG
make  | tee -a $TEST_LOG
RV=$?
echo "MAKE_RV="$RV
echo TEST_END `date`  | tee -a $TEST_LOG


if [ "$HPCNAME" != "lego" ] ; then 
 mail -s $TEST_LOG $TEST_EMAIL < $TEST_LOG ;
fi

exit $RV
