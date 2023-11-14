#!/bin/bash

#!/bin/bash

mount-input() {
    cat <<EOF
mount
EOF
}

mount-output() {
    cat <<EOF
disk mounted.
EOF
}

mount-mount-input() {
    cat <<EOF
mount
mount
EOF
}

mount-mount-output() {
    cat <<EOF
disk mounted.
mount failed!
EOF
}

mount-format-input() {
    cat <<EOF
mount
format
EOF
}

mount-format-output() {
    cat <<EOF
disk mounted.
format failed!
EOF
}

test-mount () {
    TEST=$1

    #echo -n "Testing $TEST on data/image.5 ... "
    prog_out=$($TEST-input| ./bin/sfssh data/image.5 5 2> /dev/null)
    prog_out=$(printf "$prog_out" | grep -v "block reads" | grep -v "block writes")
    if diff -u <(printf "$prog_out\n") <($TEST-output) > test.log; then
    	echo "[RESULT]Success"
    else
    	echo "[RESULT]Failure"
	cat test.log
    fi
    rm -f test.log
}

test-mount mount
test-mount mount-mount
test-mount mount-format

SCRATCH=$(mktemp -d)
trap "rm -fr $SCRATCH" INT QUIT TERM EXIT

bad-mount-input() {
    cat <<EOF
mount
EOF
}

bad-mount-output() {
    cat <<EOF
mount failed!
EOF
}

echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x10 0x34 0xf1 0xf0) >  $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x05 0x00 0x00 0x00) >> $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x01 0x00 0x00 0x00) >> $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x80 0x00 0x00 0x00) >> $SCRATCH/image.5
#echo -n "Testing bad-mount on $SCRATCH/image.5 ... "
prog_out=$(bad-mount-input| ./bin/sfssh $SCRATCH/image.5 5 2> /dev/null)
prog_out=$(printf "$prog_out" | grep -v "block reads" | grep -v "block writes")
if diff -u <(printf "$prog_out\n") <(bad-mount-output) > $SCRATCH/test.log; then
    echo "[RESULT]Success"
else
    echo "[RESULT]Failure"
    cat $SCRATCH/test.log
fi

echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x10 0x34 0xf1 0xf0) >  $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x05 0x00 0x00 0x00) >> $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x01 0x00 0x00 0x00) >> $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x80 0x00 0x00 0x00) >> $SCRATCH/image.5
#echo -n "Testing bad-mount on $SCRATCH/image.5 ... "
prog_out=$(bad-mount-input| ./bin/sfssh $SCRATCH/image.5 5 2> /dev/null)
prog_out=$(printf "$prog_out" | grep -v "block reads" | grep -v "block writes")
if diff -u <(printf "$prog_out\n") <(bad-mount-output) > $SCRATCH/test.log; then
    echo "[RESULT]Success"
else
    echo "[RESULT]Failure"
    cat $SCRATCH/test.log
fi

echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x10 0x34 0xf0 0xf0) >  $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x00 0x00 0x00 0x00) >> $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x01 0x00 0x00 0x00) >> $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x80 0x00 0x00 0x00) >> $SCRATCH/image.5
#echo -n "Testing bad-mount on $SCRATCH/image.5 ... "
prog_out=$(bad-mount-input| ./bin/sfssh $SCRATCH/image.5 5 2> /dev/null)
prog_out=$(printf "$prog_out" | grep -v "block reads" | grep -v "block writes")
if diff -u <(printf "$prog_out\n") <(bad-mount-output) > $SCRATCH/test.log; then
    echo "[RESULT]Success"
else
    echo "[RESULT]Failure"
    cat $SCRATCH/test.log
fi

echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x10 0x34 0xf0 0xf0) >  $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x05 0x00 0x00 0x00) >> $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x02 0x00 0x00 0x00) >> $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x80 0x00 0x00 0x00) >> $SCRATCH/image.5
#echo -n "Testing bad-mount on $SCRATCH/image.5 ... "
prog_out=$(bad-mount-input| ./bin/sfssh $SCRATCH/image.5 5 2> /dev/null)
prog_out=$(printf "$prog_out" | grep -v "block reads" | grep -v "block writes")
if diff -u <(printf "$prog_out\n") <(bad-mount-output) > $SCRATCH/test.log; then
    echo "[RESULT]Success"
else
    echo "[RESULT]Failure"
    cat $SCRATCH/test.log
fi

echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x10 0x34 0xf0 0xf0) >  $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x05 0x00 0x00 0x00) >> $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x01 0x00 0x00 0x00) >> $SCRATCH/image.5
echo -n -e $(printf '\\x%x\\x%x\\x%x\\x%x' 0x70 0x00 0x00 0x00) >> $SCRATCH/image.5
#echo -n "Testing bad-mount on $SCRATCH/image.5 ... "
prog_out=$(bad-mount-input| ./bin/sfssh $SCRATCH/image.5 5 2> /dev/null)
prog_out=$(printf "$prog_out" | grep -v "block reads" | grep -v "block writes")
if diff -u <(printf "$prog_out\n") <(bad-mount-output) > $SCRATCH/test.log; then
    echo "[RESULT]Success"
else
    echo "[RESULT]Failure"
    cat $SCRATCH/test.log
fi
