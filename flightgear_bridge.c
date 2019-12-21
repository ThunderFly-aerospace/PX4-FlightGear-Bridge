

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

int main(int argc, char ** argv)
{
    fprintf(stderr,"I'm Mavlink to FlightGear Bridge\n");


    struct sockaddr_in  px4_mavlink_addr;
    struct sockaddr_in  simulator_mavlink_addr;
    int listenMavlinkSock;
    int px4MavlinkSock;
    //int n;
    
    memset((char *) &simulator_mavlink_addr, 0, sizeof(px4_mavlink_addr));
    memset((char *) &px4_mavlink_addr, 0, sizeof(px4_mavlink_addr));
    simulator_mavlink_addr.sin_family = AF_INET;
    simulator_mavlink_addr.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
    simulator_mavlink_addr.sin_port = htons(4560);
    
     if ((listenMavlinkSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr,"Creating TCP socket failed: %s\n", strerror(errno));
      }

      int yes = 1;
      int result = setsockopt(listenMavlinkSock, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
      if (result != 0) {
        fprintf(stderr,"setsockopt failed: %s\n", strerror(errno));
      }

      struct linger nolinger;
      nolinger.l_onoff = 1;
      nolinger.l_linger = 0;

      result = setsockopt(listenMavlinkSock, SOL_SOCKET, SO_LINGER, &nolinger, sizeof(nolinger));
      if (result != 0) {
        fprintf(stderr,"setsockopt failed: %s\n", strerror(errno));
      }

      if (bind(listenMavlinkSock, (struct sockaddr *)&simulator_mavlink_addr, sizeof(simulator_mavlink_addr)) < 0) {
        fprintf(stderr,"bind failed:  %s\n", strerror(errno));
      }

      errno = 0;
      if (listen(listenMavlinkSock, 0) < 0) {
        fprintf(stderr,"listen failed: %s\n", strerror(errno));
      }

      unsigned int px4_addr_len;
      px4MavlinkSock = accept(listenMavlinkSock, (struct sockaddr *)&px4_mavlink_addr, &px4_addr_len);

    sleep(5);

    long time=0;
    long timestep=1000;
    int blocking=0;
    while(1)
    {
        time+=timestep;

        //IMU
        mavlink_hil_sensor_t sensor_msg;
        sensor_msg.time_usec = time;

        sensor_msg.xacc = 0;
        sensor_msg.yacc = 0;
        sensor_msg.zacc = 0;
        sensor_msg.xgyro = 0;
        sensor_msg.ygyro = 0;
        sensor_msg.zgyro = -1;
        sensor_msg.xmag = 0;
        sensor_msg.ymag = 0;
        sensor_msg.zmag = 0;

        sensor_msg.temperature = 20;
        sensor_msg.abs_pressure = 1000;
        sensor_msg.pressure_alt = 10;
        sensor_msg.diff_pressure = 0;
        sensor_msg.fields_updated = 4095;

        mavlink_message_t msg;
        mavlink_msg_hil_sensor_encode_chan(1, 200, MAVLINK_COMM_0, &msg, &sensor_msg);

        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        int packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
        
        //ssize_t len;
        send(px4MavlinkSock, buffer, packetlen, 0);
        
        //GPS
        mavlink_hil_gps_t hil_gps_msg;
        hil_gps_msg.time_usec = time;
        hil_gps_msg.fix_type = 3;
        hil_gps_msg.lat = 0;
        hil_gps_msg.lon = time/1000;
        hil_gps_msg.alt = 10;
        hil_gps_msg.eph = 65535;
        hil_gps_msg.epv = 65535;
        hil_gps_msg.vel = 65535;
        hil_gps_msg.vn = 0;
        hil_gps_msg.ve = 10;
        hil_gps_msg.vd = 0;
        hil_gps_msg.cog = 65535;
        hil_gps_msg.satellites_visible = 255;

        mavlink_msg_hil_gps_encode_chan(1, 200, MAVLINK_COMM_0, &msg, &hil_gps_msg);
        packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
        
        send(px4MavlinkSock, buffer, packetlen, 0);

        //Ground truth
      mavlink_hil_state_quaternion_t hil_state_quat;
    
      hil_state_quat.time_usec = time;

      hil_state_quat.attitude_quaternion[0] = 1;
      hil_state_quat.attitude_quaternion[1] = 0;
      hil_state_quat.attitude_quaternion[2] = 0;
      hil_state_quat.attitude_quaternion[3] = 0;

      hil_state_quat.rollspeed = 0;
      hil_state_quat.pitchspeed = 0;
      hil_state_quat.yawspeed = 0;

      hil_state_quat.lat = 0;
      hil_state_quat.lon = time/1000;
      hil_state_quat.alt = 10;

      hil_state_quat.vx = 0;
      hil_state_quat.vy = 0;
      hil_state_quat.vz = 0;

      // assumed indicated airspeed due to flow aligned with pitot (body x)
      hil_state_quat.ind_airspeed = 0;

      hil_state_quat.true_airspeed = 0;  //no wind simulated


      hil_state_quat.xacc = 0;
      hil_state_quat.yacc = 0;
      hil_state_quat.zacc = 0;

      mavlink_msg_hil_state_quaternion_encode_chan(1, 200, MAVLINK_COMM_0, &msg, &hil_state_quat);
      packetlen = mavlink_msg_to_send_buffer(buffer, &msg);
      send(px4MavlinkSock, buffer, packetlen, 0);


        fprintf(stderr,"Sended. Time %ld\n",time);

        struct pollfd fds[1] = {};
        fds[0].fd = px4MavlinkSock;
        fds[0].events = POLLIN;

        int p=poll(&fds[0], 1, (blocking==1?-1:1000));
        if(p<0)
            fprintf(stderr,"Pool error\n");

        if(p==0)
        {
            fprintf(stderr,"No data\n");
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
                            fprintf(stderr,"Parsed msg\n");
                            if(msg.msgid==MAVLINK_MSG_ID_HIL_ACTUATOR_CONTROLS)
                            {
                                fprintf(stderr,"Got Acuitator\n");
                                blocking=1;
                            }
                      }
                    }
                }
            }  
        }      
  }



}
