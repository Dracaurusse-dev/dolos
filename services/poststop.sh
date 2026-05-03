#!/bin/sh


find /tmp/dolosd.d -type f -print0 | while IFS= read -r -d '' file; do
	kill $(cat $file)
done


rm /tmp/dolosd.d/*
