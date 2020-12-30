#!/bin/bash

ps -ef | grep -x "LedMatrix" | grep -v grep | awk '{print $2}' | xargs kill
