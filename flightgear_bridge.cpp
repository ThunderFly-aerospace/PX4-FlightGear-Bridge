#include <iostream>

#include <time.h>

#include "px4_communicator.h"
#include "fg_communicator.h"
#include "vehicle_state.h"


using namespace std;

int main(int argc, char ** argv)
{
    cerr << "I'm Mavlink to FlightGear Bridge" <<endl;;

    int delay_us=5000;
    bool havePxData=false;
    bool haveFGData=false;
    bool sendEveryStep=true;

    cerr << "Targed Bridge Freq: " << 1000000.0/delay_us << ", send data every step: " << sendEveryStep << std::endl;

	VehicleState vehicle;
	PX4Communicator px4(&vehicle);
	FGCommunicator fg(&vehicle);
	
	if(px4.Init()!=0)
	{
		cerr<< "Unable to Init PX4 Communication" << endl;
		return -1;
	}

	if(fg.Init()!=0)
	{
		cerr<< "Unable to Init FG Communication" << endl;
		return -1;
	}

	while(1)
	{
        
        bool fgRecved=(fg.Recieve(false)==1);
		if(fgRecved)
            haveFGData=true;
        if(fgRecved || (haveFGData && sendEveryStep))
            px4.Send();
        
        bool px4Recved=(px4.Recieve(false)==1);
		if(px4Recved)
            havePxData=true;
        if(px4Recved || (havePxData && sendEveryStep))
    		fg.Send();

        usleep(delay_us);
	}

	cerr << "Bridge Exiting" <<endl;
	fg.Clean();
	px4.Clean();

}
