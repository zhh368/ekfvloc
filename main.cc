/*
 * main.cpp
 *
 *  Created on: 17-dic-2009
 *      Author: mayte
 */

#include <libplayerc++/playerc++.h>
#include "localize.hh"
#include "types.hh"

using namespace PlayerCc;

int main(int argc, char *argv[]){

	if (argc != 2) {
		fprintf(stderr, "Uso: fichmapa\n");
		exit(1);
	}

	PlayerClient	*pc;
	Position2dProxy	*odom;
	LaserProxy		*laser;

	pc		= new PlayerClient("localhost",6666);
	odom	= new Position2dProxy(pc,0);
	laser	= new LaserProxy(pc,0);

	pc->SetDataMode(PLAYER_DATAMODE_PULL);
	pc->SetReplaceRule(true,PLAYER_MSGTYPE_DATA,-1,-1);

    pc->Read();

    laser->RequestGeom();

    // Initialize localization:
    Localize loc(8.0,
                 laser->GetPose().px, laser->GetPose().py, laser->GetPose().pyaw,
                 0.004, 0.045,
                 0.4, 0.2, 0.2);

    loc.SetRobotPose(odom->GetXPos(), odom->GetYPos(), odom->GetYaw());
    loc.LoadMap(argv[1]);

	bool fin = false;
	int iteration = 0;

	while (not fin){
		iteration++;

		pc->Read();

		DoublesVector rho, phi;
	    for (uint32_t i=0; i < laser->GetCount(); i++) {
	    	rho.push_back(laser->GetRange(i));
	    	phi.push_back(laser->GetBearing(i));
	    }

		printf("\n\nOdometry: %f, %f, %f (%d)\n",
		       odom->GetXPos(), odom->GetYPos(), odom->GetYaw(), iteration);

		// Estimate new localization
		loc.Update(odom->GetXPos(), odom->GetYPos(), odom->GetYaw(),
		           rho, phi);

		printf("Localize: %f, %f, %f (%d)\n",
		       loc.pose().x, loc.pose().y, loc.pose().th, iteration);

		cout << "Covariance\n" << loc.GetCovariance();
	}
}

