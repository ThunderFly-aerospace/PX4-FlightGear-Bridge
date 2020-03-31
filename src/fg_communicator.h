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
#include <poll.h>


union temp64 {
	int64_t ll;
	int32_t l[2];
};


class FGCommunicator
{

private:
	VehicleState *vehicle;
	struct sockaddr_in  fg_addr_out, my_addr_out;
	int fgSockOut;
	int fgSockIn;
	struct sockaddr_in  fg_addr_in;

	double *sendBuff;


	void swap64(void *p);
public:
	FGCommunicator(VehicleState *v);
	~FGCommunicator();
	int Init();
	int Clean();

	int Send();
	int Recieve(bool blocking);

};



#endif
