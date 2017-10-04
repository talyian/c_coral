#!/bin/bash
SRCFILE=${1:-"samples/basic/hello_world.coral"}
debounce() {
    LAST=0
    while read line; do
	TIME=$(date +"%s%2N")
	if [[ $(($TIME - $LAST > 200)) ]]; then echo $line; fi
	LAST=$TIME
    done
}
(echo "0 0 watch"; inotifywait -q -m -r src samples Makefile watch.sh -e MODIFY --exclude "#") \
| stdbuf -o0 grep -v ISDIR | debounce \
| while read dir event file; do
    echo "$dir -> $event -> $file [$SRCFILE]"
    if [[ "$dir" == "watch.sh" ]]; then
	exec /bin/bash watch.sh $SRCFILE
    elif [[ "$dir" == "samples/"* && "$file" ]]; then
	SRCFILE="$dir$file"
    fi
    echo "-- [$file] -----($SRCFILE)---------------------------------------------------"
    make -s
done
