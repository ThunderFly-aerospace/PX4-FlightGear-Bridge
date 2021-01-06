#!/bin/bash
# get absolute path of models folder
SCRIPT=$(readlink -f $0)
ROOT_DIR=`dirname $SCRIPT`/../
FIRMWARE_DIR=`dirname $SCRIPT`/../../../
MODEL_PATH=${ROOT_DIR}/models

export FG_BINARY=fgfs
export FG_MODELS_DIR=${MODEL_PATH}
export FG_ARGS_EX="--allow-nasal-read=${MODEL_PATH} \
                   --callsign=TF-A2 \
                   --airport=LKSO \
                   --offset-distance=-0.003 \
                   --offset-azimuth=90 \
                   --ignore-autosave \
                   --disable-hold-short \
                   --prop:input/joysticks/js=0 \
                   --prop:/sim/rendering/static-lod/aimp-range-mode-distance=true \
                   --disable-mouse-pointer \
                   --multiplay=out,10,127.0.0.1,5001 \
                   --multiplay=in,10,127.0.0.1,5000"
export PX4_ID=1


cd ${FIRMWARE_DIR}
make px4_sitl_nolockstep flightgear_rascal___multi  # The make target flightgear_rascal___multi is currently available only in flightgear-multi branch of https://github.com/ThunderFly-aerospace/PX4Firmware fork
