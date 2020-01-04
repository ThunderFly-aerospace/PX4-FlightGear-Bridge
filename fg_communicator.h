#ifndef FG_COMMUNICATOR_H
#define FG_COMMUNICATOR_H

#include "vehicle_state.h"


#include <sys/types.h>         
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


union temp64 {
                int64_t ll;
                int32_t l[2];
};


class FGCommunicator
{

private:
	VehicleState * vehicle;
    struct sockaddr_in  fg_addr,my_addr;
    int fgSock;


	void swap64(void *p);
public: 
	FGCommunicator(VehicleState * v);
	int Init();
	int Clean();

	int Send();
	int Recieve();
	
};



#endif
