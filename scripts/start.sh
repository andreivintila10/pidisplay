#!/bin/bash

if ! pgrep -x "LedMatrix" > /dev/null
then
	/usr/bin/sudo /var/www/html/pidisplay/c/LedMatrix &
fi
