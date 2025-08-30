# FlightGear to PX4 Autopilot Bridge

![Build Tests](https://github.com/PX4/PX4-FlightGear-Bridge/workflows/Build%20Tests/badge.svg)

The FlightGear alternative to the current PX4's mainstream simulator Gazebo. FlightGear expands the simulation possibilities through advanced weather simulations. 

![FlightGear SITL connected with PX4 and QGroundControl](art/screenshot.png)

This stand-alone application adds the possibility of the use of the FlightGear simulator. The system was tested on the [Rascal airplane](https://github.com/ThunderFly-aerospace/FlightGear-Rascal) and [TF-G1](https://github.com/ThunderFly-aerospace/FlightGear-TF-G1) autogyro simulation models.

It connects to FG (over UDP generic protocol) and transforms the data to TCP MAVlink packets for the PX4 stack.

### How to use FlightGear with PX4

#### Install

1) Install FlightGear. In Ubuntu, you can install the latest stable FG from the [PPA repository](https://launchpad.net/~saiarcot895/+archive/ubuntu/flightgear) by following commands: ```sudo add-apt-repository -y -u ppa:saiarcot895/flightgear``` and ```sudo DEBIAN_FRONTEND=noninteractive apt-get -y --quiet install flightgear```
* Alternatively use the [TF modified FlightGear](https://github.com/ThunderFly-aerospace/FlightGear/wiki) for advanced features of the models. 
3) Set write permissions to the `Protocols` folder in the FlightGear installation directory. On Ubuntu run ```sudo chmod a+w /usr/share/games/flightgear/Protocol ```

#### Startup

General usage consists following steps

1) Open [QgroundControl](http://qgroundcontrol.com/)
2) In [PX4Firmware](https://github.com/PX4/PX4-Autopilot) folder run: ```make px4_sitl_nolockstep flightgear_model-name```
3) Wait until FlightGear fully loads.


##### Rascal airplane

1) Open [QgroundControl](http://qgroundcontrol.com/)
2) In PX4Firmware folder run: ```make px4_sitl_nolockstep flightgear_rascal``` for plane with combustion engine. Alternatively, you could use  ```make px4_sitl_nolockstep flightgear_rascal-electric``` for the plane with an electric engine model. 
3) Wait until FlightGear fully loads.

##### TF-G1 autogyro

1) Open [QgroundControl](http://qgroundcontrol.com/)
2) In PX4Firmware folder run: ```make px4_sitl_nolockstep flightgear_tf-g1``` for [TF-G1 autogyro](https://github.com/ThunderFly-aerospace/TF-G1).
3) Wait until FlightGear fully loads.

##### TF-G2 autogyro

1) Open [QgroundControl](http://qgroundcontrol.com/)
2) In PX4Firmware folder run: ```make px4_sitl_nolockstep flightgear_tf-g2``` for [TF-G2 autogyro](https://github.com/ThunderFly-aerospace/TF-G2).
3) Wait until FlightGear fully loads.

##### TF-R1 rover

1) Open [QgroundControl](http://qgroundcontrol.com/)
2) In PX4Firmware folder run: ```make px4_sitl_nolockstep flightgear_tf-r1``` for [TF-R1 ground rover](https://github.com/ThunderFly-aerospace/TF-R1).
3) Wait until FlightGear fully loads.



#### Known issues

1) If you have FPS lower than 20, the bridge will not work correctly. Check your FPS. In FlightGear, display frame rate by enabling it in View->View Options->Show frame rate.
2) You can probably use a wild set of FG versions - we tested installation with FG 2019.1.1 and 2020.3.8. But in the past, we achieved basic functionality on Debian 9 running FG 2016.1.1 from the distribution repository.
3) Multiple models packaged with the bridge have an electric engine that requires up-to-date FlightGear.
5) PX4 internal starting script runs ```fgfs``` with a set of parameters to reduce graphic load. 
6) The starting script searches for the FG-Data folder. Run ```fgfs --version``` to check the paths. If your output of this command does not contain the FG_ROOT line, the script will not work. Check where are FG binaries are in your system, by executing the command ``` which fgfs```. Then the Advanced Options section of this readme could help you.

### Advanced Options

You can tune your FG installation/settings by the following environment variables:

1) ```FG_BINARY``` - absolute path to FG binary to run. (It can be an AppImage)
2) ```FG_MODELS_DIR``` - absolute path to the folder containing the manually downloaded aircraft models that should be used for simulation.
3) ```FG_ARGS_EX``` - any additional FG parameters

FlightGear Bridge and starting script now support multiple instances of PX4. FG\_run script takes the second argument, which is PX4 ID (and automatically adjusts the port numbers according to the given number), and bridge binary takes this ID as the first argument before the output of get\_FGbridge\_params.py

If you want to use a currently unsupported FlightGear aircraft with PX4, you need:
1) Add the FlightGear aircraft to ```models``` subdirectory, or to another MODEL PATH searched by FG
2) In ```models``` subdirectory create ```.json``` file. 
3) Add model into PX4 make system in file ```platforms/posix/cmake/sitl_target.cmake``` with same name as json file in previous step.
4) You probably will need a new PX4 startup script for your vehicle. (vehicle name is the same name as the name of .json file)

If you want to test another plane model, you can switch the Rascal model to another type by editing the file ```models/rascal.json```.  For example substitute ```Rascal110-YASim``` by ```Rascal110-JSBSim```, or chosen aircraft name.

### Limitations

The PX4 is connected to FlightGear through "[generic protocol](http://wiki.flightgear.org/Generic_protocol)", which is served synchronously to the simulator graphics engine frame rate. So the PX4 gets the sensor data in frequency, depending on graphics resources and the current scene. The source code implements the artificial upsampling of sensor data to ~100Hz to avoid stale sensor detection triggers in PX4. Random noise is added to the sensor data.

The possibly better approach is to connect FlightGear using an [HLA](http://wiki.flightgear.org/High-Level_Architecture) interface.

### Credits

 FlightGear bridge was initially developed at [ThunderFly s.r.o.](https://www.thunderfly.cz/) by @slimonslimon <support@thunderfly.cz>
