#include "vehicle_state.h"

#include "geo_mag_declination.h"

#include <iostream>
#include <iomanip>

using namespace std;

VehicleState::VehicleState(int cCount, const int *cMap, const double *cP)
{

    this->controlsCount=cCount;
    this->controlsMap=cMap;
    this->controlsP=cP;

    this->FGControls=new double[controlsCount];

	standard_normal_distribution_ = std::normal_distribution<double>(0.0f, 1.0f);

	acc_nois=0.0001;
	gyro_nois=0.001;
	mag_nois=0.001;
	baro_alt_nois=0.01;
    temp_nois=0.01;
    abs_pressure_nois=0.05;
    diff_pressure_nois=0.01;
}

VehicleState::~VehicleState()
{
    delete [] FGControls;
}

void VehicleState::setPXControls(const mavlink_hil_actuator_controls_t& controls)
{
    bool armed = (controls.mode & MAV_MODE_FLAG_SAFETY_ARMED);
    for(int c=0;c<controlsCount;c++)
    {
        if(armed)
            FGControls[c]=controlsP[c]*(double)controls.controls[controlsMap[c]];
        else
            FGControls[c]=0;
    }
}


void VehicleState::setFGData(const fgOutputData& fgData)
{
    double freq=1.0/(fgData.elapsed_sec-lastTime);
    lastTime=fgData.elapsed_sec;

    if(freq<20)
    {
        std::cout << "Freq: "<< freq <<std::endl;
    }

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

/*void VehicleState::setStateQuatMsg(const fgOutputData& fgData)
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

}*/

void VehicleState::setSensorMsg(const fgOutputData& fgData)
{
	    sensor_msg.time_usec = fgData.elapsed_sec*1e6;
 
        //akcelerometr vypadá hodnotově také v pořádku
        sensor_msg.xacc = ftToM(fgData.accelX_fps)+acc_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.yacc = ftToM(fgData.accelY_fps)+acc_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.zacc = ftToM(fgData.accelZ_fps)+acc_nois*standard_normal_distribution_(random_generator_);

        //if(sensor_msg.xacc<-100 || sensor_msg.yacc<-100 || sensor_msg.zacc<-100)
         //   std::cout <<sensor_msg.xacc << " " << sensor_msg.yacc << " " << sensor_msg.zacc <<std::endl;

        Vector3d gyro=getGyro(fgData);
        sensor_msg.xgyro = gyro[0]+gyro_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.ygyro = gyro[1]+gyro_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.zgyro = gyro[2]+gyro_nois*standard_normal_distribution_(random_generator_);

        //magnetometr davva stejne vysledky jako Gazebo 
		Vector3d mag_l=getMagneticField(fgData);
        sensor_msg.xmag = mag_l[0]+mag_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.ymag = mag_l[1]+mag_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.zmag = mag_l[2]+mag_nois*standard_normal_distribution_(random_generator_);

        sensor_msg.temperature = fgData.temperature_degc +temp_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.abs_pressure = fgData.pressure_inhg*3386.39/100.0+abs_pressure_nois*standard_normal_distribution_(random_generator_);
        sensor_msg.pressure_alt = ftToM(fgData.pressure_alt_ft)+baro_alt_nois*standard_normal_distribution_(random_generator_);

        sensor_msg.diff_pressure = (fgData.measured_total_pressure_inhg-fgData.pressure_inhg)*3386.39/100.0+diff_pressure_nois*standard_normal_distribution_(random_generator_) ;
        sensor_msg.fields_updated = 4095;


}

Vector3d VehicleState::getGyro(const fgOutputData& fgData)
{

      //  std::cout << fgData.rateRoll_degps << " " << fgData.ratePitch_degps << " " << fgData.rateYaw_degps << std::endl;


        Quaterniond roll(Vector3d(1,0,0), degToRad(fgData.roll_deg));
        Quaterniond pitch(Vector3d(0,1,0), degToRad(fgData.pitch_deg));
        Quaterniond heading(Vector3d(0,0,1),degToRad(fgData.heading_deg));
        Quaterniond bodyRot=heading*pitch*roll;        

//        Quaterniond rollRate(Vector3d(1,0,0), degToRad(fgData.rateRoll_degps));
//        Quaterniond pitchRate(Vector3d(0,1,0), degToRad(fgData.ratePitch_degps));
//        Quaterniond headingRate(Vector3d(0,0,1),degToRad(fgData.rateYaw_degps));
//        Quaterniond omega=headingRate*pitchRate*rollRate;

        Vector3d rollRateP(degToRad(fgData.rateRoll_degps),0,0);

        Vector3d pitchRate(0,degToRad(fgData.ratePitch_degps),0);
        Vector3d pitchRateP=bodyRot.RotateVectorReverse(heading.RotateVector(pitchRate));

        Vector3d headingRate(0,0,degToRad(fgData.rateYaw_degps));
        Vector3d headingRateP=bodyRot.RotateVectorReverse(headingRate);

//        double angle;
//        Vector3d vomega;
//        omega.ToAxis(vomega,angle);
//        vomega=vomega.Normalize();
//        vomega=angle*vomega;

        Vector3d ret=rollRateP+pitchRateP+headingRateP;
//        ret[0]=-ret[0];
//        ret[1]=-ret[1];
//        ret[2]=-ret[2];
/*        ret[0]=degToRad(fgData.rateRoll_degps);
        ret[1]=degToRad(fgData.ratePitch_degps)/std::cos(degToRad(fgData.roll_deg));
        ret[2]=0;*/
        
        //std::cout << ret[0] << " " << ret[1] << " " << ret[2] << std::endl;

        return ret;
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
		float Z = H * tanf(inclination_rad);
		float X = H * cosf(declination_rad);
		float Y = H * sinf(declination_rad);

		Vector3d mag_g(X,Y,Z);

        Quaterniond roll(Vector3d(1,0,0), degToRad(fgData.roll_deg));
        Quaterniond pitch(Vector3d(0,1,0), degToRad(fgData.pitch_deg));
        Quaterniond heading(Vector3d(0,0,1),degToRad(fgData.heading_deg));
        Quaterniond bodyRot=heading*pitch*roll;

       // Quaterniond bodyRot2(degToRad(fgData.roll_deg),degToRad(fgData.pitch_deg),degToRad(fgData.heading_deg));

        Vector3d mag1=bodyRot.RotateVectorReverse(mag_g);
        //Vector3d mag2=bodyRot2.RotateVectorReverse(mag_g);

       // cerr << mag1[0] << " " << mag1[1] << " " << mag1[2] <<endl <<mag1[0] << " " << mag1[1] << " " << mag1[2] << endl << endl;

		return mag1;
}

double VehicleState::ftpssTomG(double fpss)
{
    return fpss*1000/ 32.2; //wtf ?
}

double VehicleState::ftToM(double ft)
{
	return 0.3048*ft;
}

double VehicleState::degToRad(double deg)
{
	return deg*3.141592654/180;
}
