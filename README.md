# FlightGear Bridge

The FlightGear alternative to the current PX4's mainstream simulator Gazebo.

![FlightGear SITL connected with PX4 and QGroundControl](art/screenshot.png)

This stand-alone application adding the possibility of the use of the FlightGear simulator. The system was tested on the [Rascal airplane](https://github.com/ThunderFly-aerospace/FlightGear-Rascal) and [TF-G1](https://github.com/ThunderFly-aerospace/FlightGear-TF-G1) autogyro simulation models.

It connects to FG (over UDP generic protocol) and transforms the data to TCP MAVlink packets for the PX4 stack.

### How to run the development version:

#### Install:
1) Install FlightGear.
2) Install Models. Run ```fgfs --launcher``` on tab Add-ons add new a hangar http://location_of_thunderflyhangar. The install aircraft models from this hangar.
3) Set write permissions to the `Protocols` folder in the FlightGear instalation directory. On Ubuntu run ```sudo chmod a+w /usr/share/games/flightgear/Protocols ```

#### Run:
1) Open [QgroundControl](http://qgroundcontrol.com/)
2) In PX4Firmware folder run: ```make px4_sitl_nolockstep flightgear_rascal``` for plane.
3) Wait until FlightGear fully loads.

#### Known issues:
1) If you have FPS lower than 20 the bridge will not work correctly. Check your FPS. In FlightGear display frame rate by enabling it in View->View Options->Show frame rate.
2) Multiple models in the hangar have an electric engine that needs up-to-date FlightGear from [nightly builds PPA repository](https://launchpad.net/~saiarcot895/+archive/ubuntu/flightgear-edge).
3) In the case, you have set-up mission take off in the opposite direction than aircraft current heading, then the aircraft [will fly directly away forever](https://github.com/ThunderFly-aerospace/PX4-FlightGear-Bridge/issues/10).
4) Internal starting scripts run ```fgfs``` with a set of parameters to reduce graphics and connect FG to bridge. But before that, the starting script searches for the FG-Data folder by running ```fgfs --version```. If your output of this command does not contain the FG_ROOT line, the script will not work. Check how is your FG binaries in the system, by ```which fgfs```.

### Advanced Options

You can tune your FG installation/settings by the following environment variables:

1) FG\_BINARY - absolute path to FG binary to run. (It can be an AppImage)
2) FG\_MODELS\_DIR - absolute path to the folder containing the manually-downloaded aircraft models which should be used for simulation.
3) FG\_ARGS\_EX - any additional FG parameters

FlightGear Bridge and starting script now support multiple instances of PX4. FG\_run script takes the second argument, which is PX4 ID (and automatically adjust the port numbers according to given number) and bridge binary takes this ID as the first argument before the output of get\_FGbridge\_params.py

### Limitations

The PX4 is connected to FlightGear thought "[generic protocol](http://wiki.flightgear.org/Generic_protocol)", which is served synchronously to the simulator graphics engine frame rate. So the PX4 gets the sensor data in frequency, depending on graphics resources and the current scene. The source-code implements artificial upsampling of sensor data to ~100Hz in the order to avoid stale sensor detection. Random noise is added to the sensor data.

The possible better approach is to obtain the FlightGear using an [HLA](http://wiki.flightgear.org/High-Level_Architecture) interface.

### Credits

 FlightGear bridge was initially developed at [ThunderFly s.r.o.](https://www.thunderfly.cz/) by Vít Hanousek <info@thunderfly.cz>
