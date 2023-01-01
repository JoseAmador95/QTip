#!/usr/bin/env bash
ARTIFACTS="artifacts"
NAME="ccn"
OUTDIR="$ARTIFACTS/$NAME"
RESULT_FILE="$ARTIFACTS/results.txt"
LOG="$OUTDIR/$NAME.log"

mkdir -p $OUTDIR

COMMAND="lizard"

SOURCE="source/*.c"

$COMMAND $SOURCE &> $LOG
RET=$?

echo "$COMMAND $SOURCE"
$COMMAND $SOURCE --output_file $OUTDIR/$NAME.html

cat $LOG

echo "${NAME^^}=$RET" >> $RESULT_FILE

exit 0