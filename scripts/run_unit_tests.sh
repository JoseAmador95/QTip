#!/usr/bin/env bash
ARTIFACTS="artifacts"
NAME="unittest"
OUTDIR="$ARTIFACTS/$NAME"
RESULT_FILE="$ARTIFACTS/results.txt"
LOG="$OUTDIR/$NAME.log"

mkdir -p $OUTDIR

COMMAND="ceedling"

TEST_SUITE="test:all"

CLEAN_BUILD="clobber"
ARGS_GCOV="gcov:all utils:gcov"
ARGS_SAN="options:sanitizer"

echo "$COMMAND $CLEAN_BUILD $ARGS_SAN $ARGS_GCOV $TEST_SUITE"
$COMMAND $CLEAN_BUILD $ARGS_SAN $ARGS_GCOV $TEST_SUITE &> $LOG
RET=$?

cat $LOG

cp -r build/artifacts/gcov/* $OUTDIR 

echo "${NAME^^}=$RET" >> $RESULT_FILE

exit 0