/****************************************************************************
 *
 *   Copyright (c) 2020 ThunderFly s.r.o.. All rights reserved.
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
 * @file px4_communicator.cpp
 *
 * @author ThunderFly s.r.o., Vít Hanousek <info@thunderfly.cz>
 * @url https://github.com/ThunderFly-aerospace
 *
 * PX4 communication socket.
 */

#include "px4_communicator.h"
#include <iostream>


PX4Communicator::PX4Communicator(VehicleState * v)
{
	this->vehicle=v;
}

int PX4Communicator::Init(int portOffset)
{

    memset((char *) &simulator_mavlink_addr, 0, sizeof(px4_mavlink_addr));
    memset((char *) &px4_mavlink_addr, 0, sizeof(px4_mavlink_addr));
    simulator_mavlink_addr.sin_family = AF_INET;
    simulator_mavlink_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    simulator_mavlink_addr.sin_port = htons(portBase+portOffset);

    if ((listenMavlinkSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr<<"PX4 Communicator: Creating TCP socket failed: " << strerror(errno) << std::endl;
    }

    //do not accumulate messages by waiting for ACK
    int yes = 1;
    int result = setsockopt(listenMavlinkSock, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
    if (result != 0)
    {
        std::cerr<<"PX4 Communicator: setsockopt failed: " << strerror(errno) << std::endl;
    }

    //try to close as fast as posible 
    struct linger nolinger;
    nolinger.l_onoff = 1;
    nolinger.l_linger = 0;
    result = setsockopt(listenMavlinkSock, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
    if (result != 0)
    {
        std::cerr<<"PX4 Communicator: setsockopt failed: " << strerror(errno) << std::endl;
    }

    // The socket reuse is necessary for reconnecting to the same address
    // if the socket does not close but gets stuck in TIME_WAIT. This can happen
    // if the server is suddenly closed, for example, if the robot is deleted in gazebo.
    int socket_reuse = 1;
    result = setsockopt(listenMavlinkSock, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(socket_reuse));
    if (result != 0) 
    {
         std::cerr<<"PX4 Communicator: setsockopt failed: " << strerror(errno) << std::endl;
    }

    // Same as above but for a given port
    result = setsockopt(listenMavlinkSock, SOL_SOCKET, SO_REUSEPORT, &socket_reuse, sizeof(socket_reuse));
    if (result != 0) 
    {
        std::cerr<<"PX4 Communicator: setsockopt failed: " << strerror(errno) << std::endl;
    }


    if (bind(listenMavlinkSock, (struct sockaddr *)&simulator_mavlink_addr, sizeof(simulator_mavlink_addr)) < 0)
    {
        std::cerr<<"PX4 Communicator: bind failed:  " << strerror(errno) << std::endl;
    }

    errno = 0;
    result=listen(listenMavlinkSock, 5);
    if (result < 0)
    {
        std::cerr<<"PX4 Communicator: listen failed: " << strerror(errno) << std::endl;
    }

    sleep(5);

    unsigned int px4_addr_len=sizeof(px4_mavlink_addr);
    while(true)
    {
        px4MavlinkSock = accept(listenMavlinkSock, (struct sockaddr *)&px4_mavlink_addr, &px4_addr_len);
        if (px4MavlinkSock<0)
        {
            std::cerr<<"PX4 Communicator: accept failed: " << strerror(errno) << std::endl;
        }
        else
        {
            std::cerr<<"PX4 Communicator: PX4 Connected."<< std::endl;
            break;
        }
    }

	return result;
}

/*void PX4Communicator::CheckClientReconect()
{
    struct pollfd fds[1] = {};
    fds[0].fd = listenMavlinkSock;
    fds[0].events = POLLIN;

    int p=poll(&fds[0], 1, 1);
    if(p<0)
        fprintf(stderr,"Pool for new client error\n");

    if(p==0)
    {
        //fprintf(stderr,"No new Client\n");
    }
    else
    {
        fprintf(stderr,"New Client Connected to Bridge\n");
        close(px4MavlinkSock);
        unsigned int px4_addr_len=sizeof(px4_mavlink_addr);;
        px4MavlinkSock = accept(listenMavlinkSock, (struct sockaddr *)&px4_mavlink_addr, &px4_addr_len);
    }
}*/


int PX4Communicator::Clean()
{
    close(px4MavlinkSock);
    close(listenMavlinkSock);
    return 0;
}

int PX4Communicator::Send(int offset_us)
{

    mavlink_message_t msg;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int packetlen;

    mavlink_hil_sensor_t sensor_msg =vehicle->getSensorMsg(offset_us);

    mavlink_msg_hil_sensor_encode_chan(1, 200, MAVLINK_COMM_0, &msg, &sensor_msg);
    packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
    if(send(px4MavlinkSock, buffer, packetlen, 0)!=packetlen)
    {
        std::cerr << "PX4 Communicator: Sent to PX4 failed: "<< strerror(errno) <<std::endl;
        return -1;
    }

    mavlink_hil_gps_t hil_gps_msg=vehicle->hil_gps_msg;
    hil_gps_msg.time_usec+=offset_us;

    mavlink_msg_hil_gps_encode_chan(1, 200, MAVLINK_COMM_0, &msg, &hil_gps_msg);
    packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
    if(send(px4MavlinkSock, buffer, packetlen, 0)!=packetlen)
    {
        std::cerr << "PX4 Communicator: Sent to PX4 failed: " << strerror(errno) <<std::endl;
        return -1;
    }


    mavlink_raw_rpm_t rpmmessage;
    rpmmessage.index=0;
    rpmmessage.frequency=vehicle->rpm;
    mavlink_msg_raw_rpm_encode_chan(1, 200, MAVLINK_COMM_0, &msg, &rpmmessage);
    packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
    if(send(px4MavlinkSock, buffer, packetlen, 0)!=packetlen)
    {
        std::cerr << "PX4 Communicator: Sent to PX4 failed: " << strerror(errno) <<std::endl;
        return -1;
    }

    return 0;
}

int PX4Communicator::Recieve(bool blocking)
{

        mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];

        struct pollfd fds[1] = {};
        fds[0].fd = px4MavlinkSock;
        fds[0].events = POLLIN;

        int p=poll(&fds[0], 1, (blocking?-1:2));
        if(p<0)
            std::cerr<<"PX4 Communicator: PX4 Pool error\n" << std::endl;

        if(p==0)
        {
            //std::cerr<<"PX4 Communicator:No PX data" <<std::endl;
        }
        else
        {
            if(fds[0].revents & POLLIN)
            {
                unsigned int slen=sizeof(px4_mavlink_addr);
                unsigned int len = recvfrom(px4MavlinkSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&px4_mavlink_addr, &slen);
                if (len > 0)
                {
                    mavlink_status_t status;
                    for (unsigned i = 0; i < len; ++i)
                    {
                      if (mavlink_parse_char(MAVLINK_COMM_0, buffer[i], &msg, &status))
                      {
                            if(msg.msgid==MAVLINK_MSG_ID_HIL_ACTUATOR_CONTROLS)
                            {
                                    mavlink_hil_actuator_controls_t controls;
                                    mavlink_msg_hil_actuator_controls_decode(&msg, &controls);
                                    vehicle->setPXControls(controls);
                                    return 1;
                            }
                      }
                    }
                }
            }
        }

    return 0;
}

