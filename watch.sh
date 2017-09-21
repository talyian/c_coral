#!/bin/bash
SRCFILE=${1:-"samples/functions.basic.coral"}
. ~/.bash.d/rates.sh
(echo "0 0 watch"; inotifywait -q -m -r src samples Makefile watch.sh -e MODIFY --exclude "#") \
| stdbuf -o0 grep -v ISDIR | debounce \
| while read dir event file; do
    echo "$dir -> $event -> $file [$SRCFILE]"
    if [[ "$dir" == "watch.sh" ]]; then
	exec /bin/bash watch.sh $SRCFILE
    elif [[ "$dir" == "samples/" && "$file" ]]; then
	SRCFILE="$dir/$file"
	echo "-- [$file] -----($SRCFILE)-------------------------------------------------------"
	make && bin/coral ir $SRCFILE | lli-5.0
    else
	echo "-- [$file] ------------------------------------------------------------"
	make && bin/coral ir $SRCFILE | lli-5.0
    fi
done
