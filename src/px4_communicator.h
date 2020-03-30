#ifndef PX4_COMMUNICATOR_H
#define PX4_COMMUNICATOR_H

#include "vehicle_state.h"


#include <stdio.h>
#include <mavlink/v2.0/common/mavlink.h>

#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <errno.h>

#define TIMEOUTS 5
#define TIMEOUTUS 0

class PX4Communicator
{

private:
	VehicleState * vehicle;


    struct sockaddr_in  px4_mavlink_addr;
    struct sockaddr_in  simulator_mavlink_addr;
    int listenMavlinkSock;
    int px4MavlinkSock;

public:
	PX4Communicator(VehicleState * v);
	int Init();
	int Clean();

	int Send();
	int Recieve(bool blocking);


	int Test();
};


#endif
