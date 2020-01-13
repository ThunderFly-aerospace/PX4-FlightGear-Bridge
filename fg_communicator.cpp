#include "fg_communicator.h"


FGCommunicator::FGCommunicator(VehicleState * v)
{
	this->vehicle=v;
}


int FGCommunicator::Init()
{

    fgSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    memset((char *) &fg_addr, 0, sizeof(fg_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    my_addr.sin_port = htons(4444);

	//bind socket to port
	if( bind(fgSock , (struct sockaddr*) &my_addr, sizeof(my_addr) ) == -1)
	{
		printf("Cannot bind socket");
        return -1;
	}

	return 0;
}


void FGCommunicator::swap64(void *p)
{
        union temp64  *f, t;

        f = (union temp64 *)p;

        t.l[0] = htonl(f->l[1]);
        t.l[1] = htonl(f->l[0]);

        f->ll = t.ll;
}

int FGCommunicator::Clean()
{
	close(fgSock);
	return 0;
}

int FGCommunicator::Send()
{
	return 0;
}

int FGCommunicator::Recieve()
{

	struct fgOutputData outputPacket;

    unsigned int len=sizeof(fg_addr);
    if(recvfrom(fgSock, (void *)&outputPacket, sizeof(outputPacket), 0, (struct sockaddr*) &fg_addr, &len) == -1)
    {
    	printf("Error recieve packet");
		return -1;
    }
    else
    {
        swap64(&outputPacket.elapsed_sec);

        swap64(&outputPacket.latitude_deg);
        swap64(&outputPacket.longitude_deg);
        swap64(&outputPacket.altitude_ft);

        swap64(&outputPacket.roll_deg);
        swap64(&outputPacket.pitch_deg);
        swap64(&outputPacket.heading_deg);

        swap64(&outputPacket.speed_north_fps);
        swap64(&outputPacket.speed_east_fps);
		swap64(&outputPacket.speed_down_fps);
		swap64(&outputPacket.airspeed_kt);

        swap64(&outputPacket.accelX_fps);
        swap64(&outputPacket.accelY_fps);
        swap64(&outputPacket.accelZ_fps);
        swap64(&outputPacket.rateRoll_degps);
        swap64(&outputPacket.ratePitch_degps);
        swap64(&outputPacket.rateYaw_degps);

        //fprintf(stderr,"FG data recieved\n");

		vehicle->setFGData(outputPacket);
    }

    return 0;
}



