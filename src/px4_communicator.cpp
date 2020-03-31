/**
 * @file px4_communicator.cpp
 *
 * @author ThunderFly s.r.o., Vít Hanousek <info@thunderfly.cz>
 * @url https://github.com/ThunderFly-aerospace
 *
 * PX4 communication socket.
 */

#include "px4_communicator.h"


PX4Communicator::PX4Communicator(VehicleState * v)
{
	this->vehicle=v;
}

int PX4Communicator::Init()
{

    memset((char *) &simulator_mavlink_addr, 0, sizeof(px4_mavlink_addr));
    memset((char *) &px4_mavlink_addr, 0, sizeof(px4_mavlink_addr));
    simulator_mavlink_addr.sin_family = AF_INET;
    simulator_mavlink_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    simulator_mavlink_addr.sin_port = htons(4560);

     if ((listenMavlinkSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
     {
        fprintf(stderr,"Creating TCP socket failed: %s\n", strerror(errno));
      }

      int yes = 1;
      int result = setsockopt(listenMavlinkSock, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
      if (result != 0)
      {
        fprintf(stderr,"setsockopt failed: %s\n", strerror(errno));
      }

      struct linger nolinger;
      nolinger.l_onoff = 1;
      nolinger.l_linger = 0;

      result = setsockopt(listenMavlinkSock, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
      if (result != 0)
      {
        fprintf(stderr,"setsockopt failed: %s\n", strerror(errno));
      }

      if (bind(listenMavlinkSock, (struct sockaddr *)&simulator_mavlink_addr, sizeof(simulator_mavlink_addr)) < 0)
      {
        fprintf(stderr,"bind failed:  %s\n", strerror(errno));
      }

      errno = 0;
	  result=listen(listenMavlinkSock, 0);
      if (result < 0)
      {
        fprintf(stderr,"listen failed: %s\n", strerror(errno));
      }

      unsigned int px4_addr_len;
      px4MavlinkSock = accept(listenMavlinkSock, (struct sockaddr *)&px4_mavlink_addr, &px4_addr_len);

      sleep(5);


	return result;
}


int PX4Communicator::Clean()
{
    return 0;
}

int PX4Communicator::Send()
{

    mavlink_message_t msg;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int packetlen;

    mavlink_msg_hil_sensor_encode_chan(1, 200, MAVLINK_COMM_0, &msg, &vehicle->sensor_msg);
    packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
    send(px4MavlinkSock, buffer, packetlen, 0);

	/*mavlink_msg_hil_state_quaternion_encode_chan(1, 200, MAVLINK_COMM_0, &msg, &vehicle->hil_state_quat);
    packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
    send(px4MavlinkSock, buffer, packetlen, 0);*/

    mavlink_msg_hil_gps_encode_chan(1, 200, MAVLINK_COMM_0, &msg, &vehicle->hil_gps_msg);
    packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
    send(px4MavlinkSock, buffer, packetlen, 0);

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
            fprintf(stderr,"Pool error\n");

        if(p==0)
        {
            //fprintf(stderr,"No PX data\n");
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
                            //fprintf(stderr,"Parsed msg\n");
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

int PX4Communicator::Test()
{



	return 0;
}
