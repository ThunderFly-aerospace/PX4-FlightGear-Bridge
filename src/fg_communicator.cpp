/****************************************************************************
 *
 *   Copyright (c) 2020-2022 ThunderFly s.r.o.. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file fg_communicator.cpp
 *
 * @author ThunderFly s.r.o., Vít Hanousek <info@thunderfly.cz>
 * @url https://github.com/ThunderFly-aerospace
 *
 * FlightGear socket communication.
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

int FGCommunicator::Init(int portOffset)
{

	fgSockOut = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	memset((char *) &fg_addr_out, 0, sizeof(fg_addr_out));
	memset((char *) &my_addr_out, 0, sizeof(my_addr_out));
	my_addr_out.sin_family = AF_INET;
	my_addr_out.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	my_addr_out.sin_port = htons(FGOutPortBase+portOffset);

	fgSockIn = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset((char *) &fg_addr_in, 0, sizeof(my_addr_out));
	fg_addr_in.sin_family = AF_INET;
	fg_addr_in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	fg_addr_in.sin_port = htons(FGInPortBase+portOffset);

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
		fprintf(stderr, "FG: Pool error\n");
        return 0;
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

				swap64(&outputPacket.rpm);

				//fprintf(stderr,"FG data recieved\n");
                if(outputPacket.elapsed_sec>5)
    				vehicle->setFGData(outputPacket);
                else
                    return 0;

				return 1;
			}
		}
	}

	return 0;
}
