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
 * @file fg_communicator.cpp
 *
 * @author ThunderFly s.r.o., Vít Hanousek <info@thunderfly.cz>
 * @url https://github.com/ThunderFly-aerospace
 *
 * FlightGear and PX connection manager.
 */

#include <iostream>
#include <cstdlib>
#include <cstring>

#include <time.h>
#include <signal.h>

#include "px4_communicator.h"
#include "fg_communicator.h"
#include "vehicle_state.h"


using namespace std;

int stop=0;
void termSignalHandler(int unused)
{
    stop=1;
}

void intSignalHandler(int unused)
{
   std::cerr<<"Bridge: Signal SIGINT recieve" <<std::endl; 
}

void setup_unix_signals()
{
    struct sigaction term;
    term.sa_handler = termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags |= SA_RESTART;

    //shut down by sigTerm
    if (sigaction(SIGTERM, &term, nullptr))
       std::cerr<<"Error when setting SIGTERM handler" <<std::endl;

    //ignore pipe error -rather handle it by if in code
    signal(SIGPIPE, SIG_IGN);


    struct sigaction term2;
    term2.sa_handler = intSignalHandler;
    sigemptyset(&term2.sa_mask);
    term2.sa_flags |= SA_RESTART;

    if (sigaction(SIGINT, &term2, nullptr))
       std::cerr<<"Error when setting SIGINT handler" <<std::endl;


}

int main(int argc, char **argv)
{
	cerr << "I'm Mavlink to FlightGear Bridge" << endl;;
    
	int delay_us = 2000;
	bool havePxData = false;
	bool haveFGData = false;
	bool sendEveryStep = true;

	cerr << "Targed Bridge Freq: " << 1000000.0 / delay_us << ", send data every step: " << sendEveryStep << std::endl;

	//parse parameters
	if (argc < 3) {
		cerr << "Use: bridge PX4ID ControlCount ControlIndex0 ControlP0 ControlIndex1 ControlP1 ..." << endl;
        return -1;
	}

    int px4id = atoi(argv[1]);
	int controlsCount = atoi(argv[2]);

	int *contolsMap = new int[controlsCount];
	double *controlsP = new double[controlsCount];

	for (int i = 0; i < controlsCount; i++) {
		contolsMap[i] = atoi(argv[3 + 2 * i]);
		controlsP[i] = atof(argv[3 + 2 * i + 1]);
	}

	cout << controlsCount << endl;

	for (int i = 0; i < controlsCount; i++) {
		cout << "  " << contolsMap[i] <<  "   " << controlsP[i] << endl;
	}

	VehicleState vehicle(controlsCount, contolsMap, controlsP);
	PX4Communicator px4(&vehicle);
	FGCommunicator fg(&vehicle);

	if (px4.Init(px4id) != 0) {
		cerr << "Unable to Init PX4 Communication" << endl;
		return -1;
	}

	if (fg.Init(px4id) != 0) {
		cerr << "Unable to Init FG Communication" << endl;
		return -1;
	}

    setup_unix_signals();
    stop=0; //set from Signal handler
    int FgNonRecieveIters=0;
	while (stop==0) 
    {

		bool fgRecved = (fg.Recieve(false) == 1);

		if (fgRecved) {
			haveFGData = true;
            FgNonRecieveIters=0;
		}
        else
        {
            FgNonRecieveIters++;
        }

		if (fgRecved || (haveFGData && sendEveryStep)) {
			px4.Send(FgNonRecieveIters*delay_us);
		}
        
        //useless
        //px4.CheckClientReconect();

		bool px4Recved = (px4.Recieve(false) == 1);

		if (px4Recved) {
			havePxData = true;
		}

		if (px4Recved || (havePxData && sendEveryStep)) {
			fg.Send();
		}

		usleep(delay_us);
	}


	cerr << "Bridge Exiting" << endl;
	fg.Clean();
	px4.Clean();

	delete [] contolsMap;
	delete [] controlsP;

}
