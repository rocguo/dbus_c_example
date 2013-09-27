#!/bin/sh
#DBUS_VERBOSE=1
$HOME/local/dbus/bin/dbus-daemon --session \
    --print-address --nofork
