#ifndef VEHICLE_STATE_H
#define VEHICLE_STATE_H

#include <random>

#include <mavlink/v2.0/common/mavlink.h>
#include <ignition/math/Vector3.hh>
#include <ignition/math/Quaternion.hh>

using namespace ignition::math;

struct fgOutputData {

		//Time
		double          elapsed_sec;

        // GPS
        double          latitude_deg;
        double          longitude_deg;
        double          altitude_ft;

        //orientation
        double          roll_deg;
        double          pitch_deg;
        double          heading_deg;

        double          speed_north_fps;
        double          speed_east_fps;
		double          speed_down_fps;
		double          airspeed_kt;

        // IMU
        double          accelX_fps;
        double          accelY_fps;
        double          accelZ_fps;
        double          rateRoll_degps;
        double          ratePitch_degps;
        double          rateYaw_degps;

} __attribute__((packed));




class VehicleState
{

public:

    mavlink_hil_sensor_t sensor_msg;
    mavlink_hil_gps_t hil_gps_msg;
    mavlink_hil_state_quaternion_t hil_state_quat;
    std::default_random_engine random_generator_;
    std::normal_distribution<double> standard_normal_distribution_;

	VehicleState(); 
	void setFGData(const fgOutputData& fgData);

    private:
        double ftToM(double ft);
        double degToRad(double deg);

		void setSensorMsg(const fgOutputData& fgData);
		void setGPSMsg(const fgOutputData& fgData);
		void setStateQuatMsg(const fgOutputData& fgData);

		Vector3d getBarometrTempAltPres(const fgOutputData& fgData);
		Vector3d getMagneticField(const fgOutputData& fgData);
		double getDiffPressure(const fgOutputData& fgData, double localTemp);


		double acc_nois;
		double gyro_nois;
		double mag_nois;
		double baro_alt_nois;
   
};

#endif
