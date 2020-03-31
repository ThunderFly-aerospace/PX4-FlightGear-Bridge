# FlightGear Bridge

###How to run the development version:

1) Install FlightGear. Flightgear should be able to run with ```fgfs```.
2) Install required FG model ([Rascal_110](http://wiki.flightgear.org/Rascal_110)) -- (auto-download coming soon)
3) Set write permissions to the Protocol folder of FG (in ubuntu /usr/share/games/flightgear/Protocols)
4) If flightgear data folder is different than ```/usr/share/games/flightgear/```, set that folder to ```FGDATA``` enviroment variable.
5) Open ```QgroundControl```
6) In PX4Firmware folder run: ```make px4_sitl flightgear_plane``` for plane.
