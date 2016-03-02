#!/bin/bash

let i=1
set IFS="\n"
find . -type f -name '*.htm*' -print0 | while IFS= read -r -d '' org; do
	org="${org/./}"
	org="${org/\//}"
	dest="${org//\//_}"
	dest="${dest// /_}"
	cp "$org" ./$dest
	echo $dest
	let i=i+1
done
