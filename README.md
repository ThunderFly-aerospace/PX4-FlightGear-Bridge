# FlightGear Bridge

The FlightGear alternative to the current PX4's mainstream simulator Gazebo.

![screenshot](art/screenshot.png)

This stand-alone application adding the possibility of the use of FlightGear simulator and was tested on the Rascal airplane and TF-G1 autogyro simulation models.

It connects to FG (over UDP generic protocol) and transform the data to PX4 over TCP Mavlink packets.

### How to run the development version:

1) Install FlightGear and . You should be able to run Flightgear by ```fgfs``` from command line.
2) Install required aircraft models ([Rascal_110](http://wiki.flightgear.org/Rascal_110), [ThunderFly TF-G1](https://github.com/ThunderFly-aerospace/FlightGear-TF-G1))
3) Set write permissions to the Protocol folder of FG (in ubuntu /usr/share/games/flightgear/Protocols)
4) Open [QgroundControl](http://qgroundcontrol.com/)
5) In PX4Firmware folder run: ```make px4_sitl_nolockstep flightgear_plane``` for plane or ```make px4_sitl_nolockstep flightgear_tf-g1``` for autogyro

### ADVANCED OPTIONS

You can tune your FG settings by folowing ENV VARs:
1) FG\_BINARY - absolute path to FG binnary to run. (It can be AppImage)
2) FG\_MODELS\_DIR - absolute path to folder with your downloaded models
3) FG\_ARGS\_EX - additional FG parameters 

FlightGear Bridge and starting script now supports multiple instances of PX4. FG\_run script take second argument, which is PX4 ID (and compute port numbers according this numer) and bridge binary takes this ID as firs argument before output of get\_FGbridge\_params.py

### Limitations

The PX4 is connected to FlightGear thought "[generic protocol](http://wiki.flightgear.org/Generic_protocol)", which is served synchronously to the simulator graphics engine frame rate. So the PX4 gets the sensor data in frequency, depending on graphics resources and the current scene. The source-code implements artificial upsampling of sensor data to ~100Hz in the order to avoid stale sensors detection. A random noise is added to the sensor data. 

The possible better approach is to interface the flightgear trought a [HLA](http://wiki.flightgear.org/High-Level_Architecture).

### Credits

 FlightGear bridge was initially developed at ThunderFly s.r.o. by Vít Hanousek <info@thunderfly.cz>
