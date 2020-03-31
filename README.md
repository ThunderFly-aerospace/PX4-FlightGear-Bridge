# FlightGear Bridge

### How to run the development version:

1) Install FlightGear and . You should be able to run Flightgear by ```fgfs``` from command line.
2) Install required aircraft models ([Rascal_110](http://wiki.flightgear.org/Rascal_110), [ThunderFly TF-G1](https://github.com/ThunderFly-aerospace/FlightGear-TF-G1)) 
3) Set write permissions to the Protocol folder of FG (in ubuntu /usr/share/games/flightgear/Protocols)
4) If flightgear data folder is different than ```/usr/share/games/flightgear/```, set that folder to ```FGDATA``` enviroment variable.
5) Open [QgroundControl](http://qgroundcontrol.com/)
6) In PX4Firmware folder run: ```make px4_sitl flightgear_plane``` for plane or ```make px4_sitl flightgear_TF-G1``` for autogyro
