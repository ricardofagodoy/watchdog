##
# watchdog daemon
#
# @file
# @version 0.1

CFLAGS= -Wall -pedantic -std=gnu99

all: watchdog

watchdog:
	gcc $(CFLAGS) `pkg-config --cflags --libs libnotify` main.c -o build/watchdogd

# end
