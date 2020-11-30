#!/bin/ksh

if [ -n "$1" ]; then
	command="$1"
fi

if [ "$command" = "flash" ]; then
	gmake -f /usr/local/share/makeEspArduino/makeEspArduino.mk UPLOAD_PORT=/dev/cuaU0 flash
else
	gmake -f /usr/local/share/makeEspArduino/makeEspArduino.mk
fi
