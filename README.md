# FlightGear Bridge

![Build Tests](https://github.com/PX4/PX4-FlightGear-Bridge/workflows/Build%20Tests/badge.svg)

The FlightGear alternative to the current PX4's mainstream simulator Gazebo.

![FlightGear SITL connected with PX4 and QGroundControl](art/screenshot.png)

This stand-alone application adding the possibility of the use of the FlightGear simulator. The system was tested on the [Rascal airplane](https://github.com/ThunderFly-aerospace/FlightGear-Rascal) and [TF-G1](https://github.com/ThunderFly-aerospace/FlightGear-TF-G1) autogyro simulation models.

It connects to FG (over UDP generic protocol) and transforms the data to TCP MAVlink packets for the PX4 stack.

### How to run the development version:

#### Install:
1) Install FlightGear. In Ubuntu You can use install last stable FG from the [PPA repository](https://launchpad.net/~saiarcot895/+archive/ubuntu/flightgear) by following commands: ```sudo add-apt-repository -y -u ppa:saiarcot895/flightgear``` and ```sudo DEBIAN_FRONTEND=noninteractive apt-get -y --quiet install flightgear```
3) Set write permissions to the `Protocols` folder in the FlightGear instalation directory. On Ubuntu run ```sudo chmod a+w /usr/share/games/flightgear/Protocol ```

#### Run:
1) Open [QgroundControl](http://qgroundcontrol.com/)
2) In PX4Firmware folder run: ```make px4_sitl_nolockstep flightgear_rascal``` for plane.
3) Wait until FlightGear fully loads.

#### Known issues:
1) If you have FPS lower than 20 the bridge will not work correctly. Check your FPS. In FlightGear display frame rate by enabling it in View->View Options->Show frame rate.
2) You can probably use a wild set of FG versions - we tested installation with FG 2019.1.1 and the "next" (source-code branch) of FG (nightly builds). But in the past, we achieved basic functionality on Debian 9, FG 2016.1.1 from the distribution repository, with some Racal (JSBSim) model download as a zip from the FGaddon repository.
3) Multiple models packaged with the bridge have an electric engine that needs up-to-date patched FlightGear from [nightly builds PPA repository](https://launchpad.net/~saiarcot895/+archive/ubuntu/flightgear-edge).
5) Internal starting scripts run ```fgfs``` has a set of parameters to reduce graphic load. But before that setup, the starting script searches for the FG-Data folder by running ```fgfs --version```. If your output of this command does not contain the FG_ROOT line, the script will not work. Check how is your FG binaries in the system, by ```which fgfs```. Then the Advanced Options section of this readme can help you.
6) If you want another plane model, you can switch the Rascal model to another editing file ```models/rascal.json```.  For example substitute ```Rascal110-YASim``` by ```Rascal110-JSBSim```, or another aircraft name.

### Advanced Options

You can tune your FG installation/settings by the following environment variables:

1) FG\_BINARY - absolute path to FG binary to run. (It can be an AppImage)
2) FG\_MODELS\_DIR - absolute path to the folder containing the manually-downloaded aircraft models which should be used for simulation.
3) FG\_ARGS\_EX - any additional FG parameters

FlightGear Bridge and starting script now support multiple instances of PX4. FG\_run script takes the second argument, which is PX4 ID (and automatically adjust the port numbers according to given number) and bridge binary takes this ID as the first argument before the output of get\_FGbridge\_params.py

If you want to test another aircraft with PX4, you need:
1) Add the FlightGear aircraft to ```models``` subdirectory, or to other MODEL PATH searched by FG
2) In ```models``` subdirectory create ```.json``` file. 
3) Add model into PX4 make system in file ```platforms/posix/cmake/sitl_target.cmake``` with same name as json file in previous step.
4) You probably will need a PX4 startup script for your vehicle. (vehicle name is the same name as the name of .json file)

### Limitations

The PX4 is connected to FlightGear thought "[generic protocol](http://wiki.flightgear.org/Generic_protocol)", which is served synchronously to the simulator graphics engine frame rate. So the PX4 gets the sensor data in frequency, depending on graphics resources and the current scene. The source-code implements artificial upsampling of sensor data to ~100Hz in the order to avoid stale sensor detection. Random noise is added to the sensor data.

The possible better approach is to obtain the FlightGear using an [HLA](http://wiki.flightgear.org/High-Level_Architecture) interface.

### Credits

 FlightGear bridge was initially developed at [ThunderFly s.r.o.](https://www.thunderfly.cz/) by Vít Hanousek <info@thunderfly.cz>
