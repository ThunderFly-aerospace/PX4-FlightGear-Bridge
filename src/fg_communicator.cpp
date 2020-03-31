/**
 * @file fg_communicator.cpp
 *
 * @author ThunderFly s.r.o., Vít Hanousek <info@thunderfly.cz>
 * @url https://github.com/ThunderFly-aerospace
 *
 * FlightGear communication socket.
 */


#include "fg_communicator.h"


FGCommunicator::FGCommunicator(VehicleState *v)
{
	this->vehicle = v;
	sendBuff = new double[v->controlsCount];
}

FGCommunicator::~FGCommunicator()
{
	delete [] sendBuff;
}

int FGCommunicator::Init()
{

	fgSockOut = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset((char *) &fg_addr_out, 0, sizeof(fg_addr_out));
	memset((char *) &my_addr_out, 0, sizeof(my_addr_out));
	my_addr_out.sin_family = AF_INET;
	my_addr_out.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	my_addr_out.sin_port = htons(4444);

	fgSockIn = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset((char *) &fg_addr_in, 0, sizeof(my_addr_out));
	fg_addr_in.sin_family = AF_INET;
	fg_addr_in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	fg_addr_in.sin_port = htons(4445);

	//bind socket to port
	if (bind(fgSockOut, (struct sockaddr *) &my_addr_out, sizeof(my_addr_out)) == -1) {
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
	close(fgSockOut);
	close(fgSockIn);
	return 0;
}

int FGCommunicator::Send()
{
	for (int c = 0; c < vehicle->controlsCount; c++) {
		sendBuff[c] = vehicle->FGControls[c];
		swap64(&(sendBuff[c]));
	}

	int size = vehicle->controlsCount * sizeof(double);

	if (sendto(fgSockIn, (void *)(sendBuff), size, 0,
		   (struct sockaddr *) &fg_addr_in, sizeof(fg_addr_in)) != size) {
		printf("Error send packet");
		return -1;
	}

	return 0;
}

int FGCommunicator::Recieve(bool blocking)
{

	struct fgOutputData outputPacket;
	struct pollfd fds[1] = {};
	fds[0].fd = fgSockOut;
	fds[0].events = POLLIN;

	int p = poll(&fds[0], 1, (blocking ? -1 : 2));

	if (p < 0) {
		fprintf(stderr, "Pool error\n");
	}

	if (p == 0) {
		//fprintf(stderr,"No FG data\n");
	} else {
		if (fds[0].revents & POLLIN) {

			unsigned int len = sizeof(fg_addr_out);

			if (recvfrom(fgSockOut, (void *)&outputPacket, sizeof(outputPacket), 0, (struct sockaddr *) &fg_addr_out, &len) == -1) {
				printf("Error recieve packet");
				return -1;

			} else {
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

				swap64(&outputPacket.pressure_alt_ft);
				swap64(&outputPacket.temperature_degc);
				swap64(&outputPacket.pressure_inhg);
				swap64(&outputPacket.measured_total_pressure_inhg);

				//fprintf(stderr,"FG data recieved\n");

				vehicle->setFGData(outputPacket);

				return 1;
			}
		}
	}

	return 0;
}
