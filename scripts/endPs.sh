#!/bin/bash

ps -ef | grep LedMatrix | grep -v grep | awk '{print $2}' | xargs kill
