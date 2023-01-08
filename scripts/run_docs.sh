#!/usr/bin/env bash
ARTIFACTS="artifacts"
NAME="docs"
OUTDIR="$ARTIFACTS/$NAME"
RESULT_FILE="$ARTIFACTS/results.txt"
LOG="$OUTDIR/$NAME.log"
DOCS="build/doxygen"

mkdir -p $OUTDIR $DOCS

COMMAND="doxygen"

CONFIG_FILE="docs/Doxyfile"

$COMMAND $CONFIG_FILE &> $LOG
RET=$?

echo "$COMMAND $CONFIG_FILE"

cat $LOG

echo "${NAME^^}=$RET" >> $RESULT_FILE

cp -r $DOCS/* $OUTDIR

exit 0