#!/bin/bash
# watch.sh uses inotify to trigger re-makes.
# I guess this basically is a basic version of nodemon
SRCFILE=${1:-"samples/types/enum.coral"}
debounce() {
    LAST=0
    while read line; do
	TIME=$(date +"%s%2N")
	if (($TIME - $LAST > 200)); then echo $line; fi
	LAST=$TIME
    done
}
(echo "0 0"; inotifywait -q -m -r src samples tests Makefile watch.sh -e MODIFY --exclude "#") \
| stdbuf -o0 grep -v ISDIR | debounce \
| while read dir event file; do
    echo "-- [$file] -----($SRCFILE)-------------------------------"
    echo "$dir -> $event -> $file [$SRCFILE]"
    if [[ $SRCFILE == "test" ]]; then
	make -s test
	continue
    elif [[ "$file" == "watch.sh" ]]; then
	exec /bin/bash watch.sh $SRCFILE
    elif [[ "$file" == *.coral ]]; then
	SRCFILE="$dir$file"
    fi
    echo "-- [$file] -----($SRCFILE)-------------------------------"
    make -s scope
    # make -s bin/coral && bin/coral parse ${SRCFILE} && bin/coral jit ${SRCFILE}
    # make -s bin/infer && bin/infer
done
