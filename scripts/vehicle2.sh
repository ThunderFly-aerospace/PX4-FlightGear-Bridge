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
                   --disable-mouse-pointer \
                   --multiplay=out,10,127.0.0.1,5001 \
                   --multiplay=in,10,127.0.0.1,5000"
export PX4_ID=1
export model="rascal"

#expect that we have builded target px4_sitl_nolockstep with FlightGear bridge
#by px4_sitl_nolockstep flightgear_rascal

src_path=${FIRMWARE_DIR}
build_path=${FIRMWARE_DIR}"/build/px4_sitl_nolockstep/"
sitl_bin=${build_path}"/bin/px4"

#copied form sitl_run.sh
rootfs="$build_path/tmp/rootfs_"$PX4_ID # this is the working directory
mkdir -p "$rootfs"

cd "${src_path}/Tools/flightgear_bridge/"
"${src_path}/Tools/flightgear_bridge/FG_run.py" "models/"${model}".json" $PX4_ID
"${build_path}/build_flightgear_bridge/flightgear_bridge" $PX4_ID `./get_FGbridge_params.py "models/"${model}".json"` &
FG_BRIDGE_PID=`echo $!`

cd ${rootfs}
sitl_command="\"$sitl_bin\" $no_pxh \"$src_path\"/ROMFS/px4fmu_common -s etc/init.d-posix/rcS -t \"$src_path\"/test_data -i $PX4_ID"
echo $sitl_command

export PX4_SIM_MODEL=${model}
eval $sitl_command

kill $FG_BRIDGE_PID
kill -9 `cat /tmp/px4fgfspid_$PX4_ID`
