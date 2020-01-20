#!/bin/bash


#FGFS=fgfs
FGFS=/usr/local/flightgear/bin/fgfs

#PROTOCOL=/usr/share/games/flightgear/Protocol
PROTOCOL=/usr/local/flightgear/share/flightgear/Protocol


cp px4bridge.xml $PROTOCOL
$FGFS \
    --aircraft=Rascal110-JSBSim \
    --timeofday=noon \
    --disable-sound \
    --generic=socket,out,10,127.0.0.1,4444,udp,px4bridge \
    --telnet=4443 \
    --disable-real-weather-fetch 

#./a.out

#    --generic=socket,out,10,127.0.0.1,4444,udp,px4bridge \
#    --generic=file,out,1,fg.log,px4bridge \
