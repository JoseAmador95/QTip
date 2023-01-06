#!/usr/bin/env bash
ARTIFACTS="artifacts"
NAME="build"
OUTDIR="$ARTIFACTS/$NAME"
RESULT_FILE="$ARTIFACTS/results.txt"
LOG="$OUTDIR/$NAME.log"

COMPILER="$1"
mkdir -p $OUTDIR

COMMAND="cmake"
SRC="$(pwd)"
BUILD="build"
TARGET="qtip"

echo "$COMMAND -S $SRC -B $BUILD -DCMAKE_C_COMPILER=$COMPILER"
$COMMAND -S $SRC -B $BUILD -DCMAKE_C_COMPILER=$COMPILER &>> $LOG

echo "$COMMAND --build $BUILD --target $TARGET"
$COMMAND --build $BUILD --target $TARGET &>> $LOG
RET=$?

cat $LOG

echo "${NAME^^}=$RET" >> $RESULT_FILE

exit 0