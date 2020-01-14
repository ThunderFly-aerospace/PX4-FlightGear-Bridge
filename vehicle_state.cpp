#include "vehicle_state.h"

#include "geo_mag_declination.h"

#include <iostream>

using namespace std;

VehicleState::VehicleState()
{
	standard_normal_distribution_ = std::normal_distribution<double>(0.0f, 1.0f);

	acc_nois=0.01;
	gyro_nois=0.001;
	mag_nois=0.001;
	baro_alt_nois=0.1;
}

void VehicleState::setFGData(const fgOutputData& fgData)
{
	setSensorMsg(fgData);
    //setStateQuatMsg(fgData);
    setGPSMsg(fgData);
}

void VehicleState::setGPSMsg(const fgOutputData& fgData)
{
    hil_gps_msg.time_usec = fgData.elapsed_sec*1e6;
    hil_gps_msg.fix_type = 3;
    hil_gps_msg.lat = fgData.latitude_deg * 1e7;
    hil_gps_msg.lon = fgData.longitude_deg * 1e7;
    hil_gps_msg.alt = ftToM(fgData.altitude_ft) * 1000;
    hil_gps_msg.eph =  100;
    hil_gps_msg.epv = 100;    
    hil_gps_msg.vn = ftToM(fgData.speed_north_fps)*100;
    hil_gps_msg.ve = ftToM(fgData.speed_east_fps)*100;
    hil_gps_msg.vd = ftToM(fgData.speed_down_fps)*100;
    hil_gps_msg.vel = std::sqrt(hil_gps_msg.vn*hil_gps_msg.vn+hil_gps_msg.ve*hil_gps_msg.ve);
    double cog=-std::atan2(hil_gps_msg.vn,hil_gps_msg.ve)*180/3.141592654+90;
    if(cog<0)
        cog+=360;
    hil_gps_msg.cog =cog*100; 
    hil_gps_msg.satellites_visible = 10;

   // std::cerr<< fgData.speed_north_fps <<"   " << fgData.speed_east_fps << std::endl;

}

void VehicleState::setStateQuatMsg(const fgOutputData& fgData)
{
      hil_state_quat.time_usec = fgData.elapsed_sec*1e6;

      Quaterniond bodyRot(degToRad(fgData.roll_deg),degToRad(fgData.pitch_deg),degToRad(fgData.heading_deg));

      hil_state_quat.attitude_quaternion[0] = bodyRot.W();
      hil_state_quat.attitude_quaternion[1] = bodyRot.X();
      hil_state_quat.attitude_quaternion[2] = bodyRot.Y();
      hil_state_quat.attitude_quaternion[3] = bodyRot.Z();

      hil_state_quat.rollspeed = degToRad(fgData.rateRoll_degps);
      hil_state_quat.pitchspeed = degToRad(fgData.ratePitch_degps);
      hil_state_quat.yawspeed =  degToRad(fgData.rateYaw_degps);

      hil_state_quat.lat = fgData.latitude_deg * 1e7;
      hil_state_quat.lon = fgData.longitude_deg * 1e7;
      hil_state_quat.alt = ftToM(fgData.altitude_ft) * 100;

      //TODO:
      hil_state_quat.vx = 0;
      hil_state_quat.vy = 0;
      hil_state_quat.vz = 0;

      //TODO:
      // assumed indicated airspeed due to flow aligned with pitot (body x)
      hil_state_quat.ind_airspeed = 0;
      hil_state_quat.true_airspeed = 0;  //no wind simulated

      hil_state_quat.xacc = ftToM(fgData.accelX_fps)*1000;
      hil_state_quat.yacc = ftToM(fgData.accelY_fps)*1000;
      hil_state_quat.zacc = ftToM(fgData.accelZ_fps)*1000;

}

void VehicleState::setSensorMsg(const fgOutputData& fgData)
{
	    sensor_msg.time_usec = fgData.elapsed_sec*1e6;
 
        //akcelerometr vypadá hodnotově také v pořádku
        sensor_msg.xacc = ftToM(fgData.accelX_fps)+acc_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.yacc = ftToM(fgData.accelY_fps)+acc_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.zacc = ftToM(fgData.accelZ_fps)+acc_nois*standard_normal_distribution_(random_generator_);
  
        Vector3d gyro=getGyro(fgData);
        sensor_msg.xgyro = gyro[0]+gyro_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.ygyro = gyro[1]+gyro_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.zgyro = gyro[2]+gyro_nois*standard_normal_distribution_(random_generator_);

        //magnetometr davva stejne vysledky jako Gazebo 
		Vector3d mag_l=getMagneticField(fgData);
        sensor_msg.xmag = mag_l[0]+mag_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.ymag = mag_l[1]+mag_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.zmag = mag_l[2]+mag_nois*standard_normal_distribution_(random_generator_);

        sensor_msg.temperature = (float)fgData.temperature_degc;
        sensor_msg.abs_pressure = fgData.pressure_inhg*3386.39/100.0;
        sensor_msg.pressure_alt = ftToM(fgData.pressure_alt_ft);

        std::cout << fgData.temperature_degc << "  " << fgData.pressure_inhg*3386.39/100.0 << "   " << fgData.pressure_alt_ft <<std::endl;

        sensor_msg.diff_pressure = (fgData.measured_total_pressure_inhg-fgData.pressure_inhg)*3386.39/100.0 ;
        sensor_msg.fields_updated = 4095;


}

Vector3d VehicleState::getGyro(const fgOutputData& fgData)
{
        Quaterniond bodyRot(degToRad(fgData.roll_deg),degToRad(fgData.pitch_deg),degToRad(fgData.heading_deg));
        Vector3d omega(degToRad(fgData.rateRoll_degps),degToRad(fgData.ratePitch_degps),degToRad(fgData.rateYaw_degps));

        return bodyRot.RotateVectorReverse(omega*Vector3d(1,-1,1));
}

Vector3d VehicleState::getMagneticField(const fgOutputData& fgData)
{
		// Magnetic strength (10^5xnanoTesla)
		float strength_ga = 0.01f * get_mag_strength(fgData.latitude_deg, fgData.longitude_deg);

		// Magnetic declination and inclination (radians)
		float declination_rad = get_mag_declination(fgData.latitude_deg, fgData.longitude_deg) * 3.14159265f / 180;
		float inclination_rad = get_mag_inclination(fgData.latitude_deg, fgData.longitude_deg) * 3.14159265f / 180;

		// Magnetic filed components are calculated by http://geomag.nrcan.gc.ca/mag_fld/comp-en.php
		float H = strength_ga * cosf(inclination_rad);
		float Z = H * tanf(inclination_rad) ;
		float X = H * cosf(declination_rad);
		float Y = H * sinf(declination_rad);

		Vector3d mag_g(X,Y,Z);

        Quaterniond roll(Vector3d(1,0,0), degToRad(fgData.roll_deg));
        Quaterniond pitch(Vector3d(0,1,0), degToRad(fgData.pitch_deg));
        Quaterniond heading(Vector3d(0,0,1),degToRad(fgData.heading_deg));
        Quaterniond bodyRot=heading*roll*pitch;

       // Quaterniond bodyRot2(degToRad(fgData.roll_deg),degToRad(fgData.pitch_deg),degToRad(fgData.heading_deg));

        Vector3d mag1=bodyRot.RotateVectorReverse(mag_g);
        //Vector3d mag2=bodyRot2.RotateVectorReverse(mag_g);

       // cerr << mag1[0] << " " << mag1[1] << " " << mag1[2] <<endl <<mag1[0] << " " << mag1[1] << " " << mag1[2] << endl << endl;

		return mag1;
}


double VehicleState::ftToM(double ft)
{
	return 0.3048*ft;
}

double VehicleState::degToRad(double deg)
{
	return deg*3.141592654/180;
}
