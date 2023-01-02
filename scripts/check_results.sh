#!/usr/bin/env bash
ARTIFACTS="artifacts"
RESULTS="results.txt"
FILE=$ARTIFACTS/$RESULTS

if ! [[ -f $FILE ]]; then
    echo "Results file not found" > /dev/stderr
    exit 1
fi

source $FILE

echo "Return Code for each stage (0 is PASS, non-zero is FAIL)"
echo "Stage: Build Return:$BUILD"
echo "Stage: LINTER Return:$LINTER"
echo "Stage: CCN Return:$CCN"
echo "Stage: Unit Test Return:$UNITTEST"

if [ "$BUILD" -eq "0" ] && [ "$LINTER" -eq "0" ] && [ "$CCN" -eq "0" ] && [ "$UNITTEST" -eq "0" ];
then
    echo "Build PASSED"
    RET=0
else
    echo "Build FAILED"
    RET=1
fi

exit $RET