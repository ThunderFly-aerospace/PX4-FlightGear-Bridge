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
    hil_gps_msg.eph =  100.0;
    hil_gps_msg.epv = 100.0;
    hil_gps_msg.vel = 65535;
    hil_gps_msg.vn = ftToM(fgData.speed_north_fps)*100;
    hil_gps_msg.ve = ftToM(fgData.speed_east_fps)*100;
    hil_gps_msg.vd = ftToM(fgData.speed_down_fps)*100;
    hil_gps_msg.cog = 65535;
    hil_gps_msg.satellites_visible = 10;

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

		Vector3d TAP=getBarometrTempAltPres(fgData);
        sensor_msg.temperature = TAP[0];
        sensor_msg.abs_pressure = TAP[2];
        sensor_msg.pressure_alt = TAP[1];

        sensor_msg.diff_pressure = getDiffPressure(fgData,TAP[0]);
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

Vector3d VehicleState::getBarometrTempAltPres(const fgOutputData& fgData)
{
    // calculate abs_pressure using an ISA model for the tropsphere (valid up to 11km above MSL)
    const float lapse_rate = 0.0065f; // reduction in temperature with altitude (Kelvin/m)
    const float temperature_msl = 288.0f; // temperature at MSL (Kelvin)
    float alt_msl = ftToM(fgData.altitude_ft)+baro_alt_nois*standard_normal_distribution_(random_generator_);;
    float temperature_local = temperature_msl - lapse_rate * alt_msl;
    float pressure_ratio = powf((temperature_msl/temperature_local), 5.256f);
    const float pressure_msl = 101325.0f; // pressure at MSL
    float absolute_pressure = pressure_msl / pressure_ratio;

    // convert to hPa
    absolute_pressure *= 0.01f;
	Vector3d ret;
    ret[2]=absolute_pressure;

    // calculate density using an ISA model for the tropsphere (valid up to 11km above MSL)
    //const float density_ratio = powf((temperature_msl/temperature_local) , 4.256f);
    //float rho = 1.225f / density_ratio;

    // calculate pressure altitude including effect of pressure noise
    ret[1]=alt_msl;

    // calculate temperature in Celsius
    ret[0]=temperature_local - 273.0f;

	return ret;
}

double VehicleState::getDiffPressure(const fgOutputData& fgData, double localTemp)
{
	const double temperature_msl = 288.0; // temperature at MSL (Kelvin)
    double temperature_local = localTemp + 273.0;
    const double density_ratio = pow((temperature_msl/temperature_local) , 4.256);
    double rho = 1.225 / density_ratio;

    // Let's use a rough guess of 0.05 hPa as the standard devitiation which roughly yields
    // about +/- 1 m/s noise.
    //const float diff_pressure_stddev = 0.05f;
    //const float diff_pressure_noise = standard_normal_distribution_(random_generator_) * diff_pressure_stddev;

    // calculate differential pressure in hPa
    // if vehicle is a tailsitter the airspeed axis is different (z points from nose to tail)
	double vel_ms=fgData.airspeed_kt*1.852/3.6;
    return 0.005*rho*vel_ms*vel_ms;
    

}


double VehicleState::ftToM(double ft)
{
	return 0.3048*ft;
}

double VehicleState::degToRad(double deg)
{
	return deg*3.141592654/180;
}
