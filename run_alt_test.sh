#!/bin/bash


FGFS=fgfs
#FGFS=/usr/local/flightgear/bin/fgfs

PROTOCOL=/usr/share/games/flightgear/Protocol
#PROTICOL=/usr/local/flightgear/share/flightgear/Protocol


cp px4bridge.xml $PROTOCOL
$FGFS \
    --aircraft=Rascal110-JSBSim \
    --timeofday=noon \
    --disable-sound \
    --telnet=4443 \
    --generic=socket,out,1,127.0.0.1,4444,udp,px4bridge

#./a.out
