#include <iostream>


#include "px4_communicator.h"
#include "fg_communicator.h"
#include "vehicle_state.h"

using namespace std;

int main(int argc, char ** argv)
{
    cerr << "I'm Mavlink to FlightGear Bridge" <<endl;;
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
		fg.Recieve();
		px4.Send();
		//px4.Recieve(false);
		//fg.Send();
	}

	cerr << "Bridge Exiting" <<endl;
	fg.Clean();
	px4.Clean();

}
