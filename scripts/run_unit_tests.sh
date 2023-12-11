#!/usr/bin/env bash
ARTIFACTS="artifacts"
NAME="unittest"
OUTDIR="$ARTIFACTS/$NAME"
RESULT_FILE="$ARTIFACTS/results.txt"
LOG="$OUTDIR/$NAME.log"

mkdir -p $OUTDIR

BUILD="build"
COMMAND="ctest"

echo "${COMMAND} --verbose --test-dir ${BUILD}"
${COMMAND} --verbose --test-dir ${BUILD}
RET=$?

cat $LOG

cp -r build/artifacts/gcov/* $OUTDIR 

echo "${NAME^^}=$RET" >> $RESULT_FILE

exit 0