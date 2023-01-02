#!/usr/bin/env bash
ARTIFACTS="artifacts"
NAME="linter"
OUTDIR="$ARTIFACTS/$NAME"
RESULT_FILE="$ARTIFACTS/results.txt"
LOG="$OUTDIR/$NAME.log"

mkdir -p $OUTDIR

COMMAND="clang-tidy"
BUILD_DB="build"

SOURCE="source/*.c"
INCLUDE="include/*.h"
TEST="test/*.c"

echo "$COMMAND -p $BUILD_DB $SOURCE $INCLUDE $TEST"
$COMMAND -p $BUILD_DB $SOURCE $INCLUDE $TEST &> $LOG
RET=$?

cat $LOG

echo "${NAME^^}=$RET" >> $RESULT_FILE

exit 0