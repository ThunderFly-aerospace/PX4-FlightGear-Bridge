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
 * @file vehicle_state.h
 *
 * @author ThunderFly s.r.o., Vít Hanousek <info@thunderfly.cz>
 * @url https://github.com/ThunderFly-aerospace
 *
 * FG to PX4 messages and units transcript.
*/

#ifndef VEHICLE_STATE_H
#define VEHICLE_STATE_H

#include <random>

#include <common/mavlink.h>
#include <Eigen/Geometry>

using namespace Eigen;

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

	//baro
	double          pressure_alt_ft;
	double          temperature_degc;
	double          pressure_inhg;
	double          measured_total_pressure_inhg;

	//rpm
	double          rpm;

} __attribute__((packed));


class VehicleState
{

//typedef Vector3d Vector3<double>

public:

	mavlink_hil_gps_t hil_gps_msg;
	double rpm;
	double *FGControls;
	int controlsCount;

	std::default_random_engine random_generator_;
	std::normal_distribution<double> standard_normal_distribution_;

	VehicleState(int cCount, const int *cMap, const double *cP);
	~VehicleState();
	void setFGData(const fgOutputData &fgData);
	void setPXControls(const mavlink_hil_actuator_controls_t &controls);
    mavlink_hil_sensor_t getSensorMsg(int offset_us);

private:
	double ftToM(double ft);
	double degToRad(double deg);
	double ftpssTomG(double fpss);

	void setSensor(const fgOutputData &fgData);
	void setGPSMsg(const fgOutputData &fgData);
	//void setStateQuatMsg(const fgOutputData &fgData);

	Vector3d getBarometrTempAltPres(const fgOutputData &fgData);
	Vector3d getMagneticField(const fgOutputData &fgData);
	Vector3d getGyro(const fgOutputData &fgData);
	double getDiffPressure(const fgOutputData &fgData, double localTemp);

	double lastTime;

	const int *controlsMap;
	const double *controlsP;

	double acc_nois;
	double gyro_nois;
	double mag_nois;
	double baro_alt_nois;
	double temp_nois;
	double abs_pressure_nois;
	double diff_pressure_nois;

    //sensor state
    double elapsed_sec;
    Vector3d acc;
    Vector3d gyro;
    Vector3d mag_l;
    double temperature;
    double abs_pressure;
    double pressure_alt;
    double diff_pressure;


    Quaterniond lastFGBodyRot;
    double lastFGTime;


};

#endif
