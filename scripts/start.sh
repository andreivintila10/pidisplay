#!/bin/bash

if [[ $# -eq 1 ]]
then
	if [[ "$1" == "-k" ]]
	then
		echo "Killing previous instance of LedMatrix"
		sudo /usr/bin/sudo /var/www/html/pidisplay/scripts/endPs.sh
	fi
fi


if ! pgrep -x "LedMatrix" > /dev/null
then
	/usr/bin/sudo /var/www/html/pidisplay/c/LedMatrix &
fi
