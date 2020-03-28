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
    --generic=socket,out,1000,127.0.0.1,4444,udp,px4bridge \
    --telnet=4443 \
    --disable-terrasync \
	--disable-splash-screen \
	--model-hz=120 \
	--disable-random-objects \
	--prop:/sim/rendering/texture-compression=off \
	--prop:/sim/rendering/quality-level=0 \
	--prop:/sim/rendering/shaders/quality-level=0 \
	--disable-ai-traffic \
	--prop:/sim/ai/enabled=0 \
	--prop:/sim/rendering/random-vegetation=0 \
	--prop:/sim/rendering/random-buildings=0 \
	--disable-specular-highlight \
	--disable-ai-models \
	--disable-clouds \
	--disable-clouds3d \
	--fog-fastest \
	--visibility=5000 \
	--disable-distance-attenuation \
	--disable-real-weather-fetch \
	--prop:/sim/rendering/particles=0 \
	--prop:/sim/rendering/multi-sample-buffers=1 \
	--prop:/sim/rendering/multi-samples=2 \
	--prop:/sim/rendering/draw-mask/clouds=false \
	--prop:/sim/rendering/draw-mask/aircraft=false \
	--prop:/sim/rendering/draw-mask/models=false \
	--prop:/sim/rendering/draw-mask/terrain=true \
	--disable-random-vegetation \
	--disable-random-buildings \
	--disable-rembrandt \
	--disable-horizon-effect
	



#./a.out

#    --generic=socket,out,10,127.0.0.1,4444,udp,px4bridge \
#    --generic=file,out,1,fg.log,px4bridge \
