/*
 * UE.h
 *
 *  Created on: 02 Jun 2019
 *      Author: gift
 */

#ifndef SCRATCH_UE_H_
#define SCRATCH_UE_H_

#include "../build/ns3/mobility-helper.h"
#include "../build/ns3/node-container.h"
#include "../build/ns3/object.h"
#include "../build/ns3/position-allocator.h"
#include "../build/ns3/ptr.h"
#include "../build/ns3/vector.h"
#include "ns3/lte-module.h"
#include "ns3/netanim-module.h"
#include <string>

namespace ns3 {
class UE {
public:
	UE(int, int, int, int);
	UE(int, int, int);
	void setNetAnimProperties(AnimationInterface*, int);
	NodeContainer* getUes() {
		return &UENodes;
	}
	struct historyPos {
		Vector p1;
		Vector p2;
	};
	std::map<uint32_t, UE::historyPos>* getUEPositionHistory() {
		return &uePositionHistory;
	}
	double getLoggingDistance(){
		return loggingDistance;
	}
private:
	int numOfUEs;
	NodeContainer UENodes;
	MobilityHelper UeMobilityHelper;
	MobilityHelper UeMobilityHelper60Kmh;
	MobilityHelper UeMobilityHelper120Kmh;
	int xCenter;
	int yCenter;
	int radius;
	double loggingDistance;
	std::map<uint32_t, UE::historyPos> uePositionHistory;
};

UE::UE(int numberOfUes, int yPosition, int speedDifference) :
		xCenter(0), yCenter(0), radius(0), loggingDistance(30) {
	numOfUEs = numberOfUes;
	UENodes.Create(numberOfUes);

	Ptr<ListPositionAllocator> positionAlloc = CreateObject<
			ListPositionAllocator>();
	for (int i = 0; i < numberOfUes; i++) {
		positionAlloc->Add(Vector(-50, yPosition, 0));
	}

	UeMobilityHelper.SetPositionAllocator(positionAlloc);
	UeMobilityHelper.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
	UeMobilityHelper.Install(UENodes);
	int minimumSpeed = 10; //m/s
	for (uint n = 0; n < UENodes.GetN(); n++) {
		Ptr<ConstantVelocityMobilityModel> mob  =
				UENodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
		mob->SetVelocity(Vector(minimumSpeed + n * speedDifference, 0, 0));
	}
}

UE::UE(int numberOfUes, int xBound, int yBound, int _radius) {
	numOfUEs = numberOfUes;
	UENodes.Create(numberOfUes);

	UeMobilityHelper.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
			"X", StringValue(std::to_string(xBound)), "Y",
			StringValue(std::to_string(yBound)), "Rho",
			StringValue(
					"ns3::UniformRandomVariable[Min=5|Max="
							+ std::to_string(_radius) + "]"));

	UeMobilityHelper60Kmh.SetPositionAllocator(
			"ns3::RandomDiscPositionAllocator", "X",
			StringValue(std::to_string(xBound)), "Y",
			StringValue(std::to_string(yBound)), "Rho",
			StringValue(
					"ns3::UniformRandomVariable[Min=5|Max="
							+ std::to_string(_radius) + "]"));

	UeMobilityHelper120Kmh.SetPositionAllocator(
			"ns3::RandomDiscPositionAllocator", "X",
			StringValue(std::to_string(xBound)), "Y",
			StringValue(std::to_string(yBound)), "Rho",
			StringValue(
					"ns3::UniformRandomVariable[Min=5|Max="
							+ std::to_string(_radius) + "]"));

	xCenter = xBound;
	yCenter = yBound;
	radius = +_radius;

	int xmin = xCenter - radius;
	int xmax = xCenter + radius;
	int ymin = yCenter - radius;
	int ymax = yCenter + radius;
	UeMobilityHelper.SetMobilityModel("ns3::RandomWalk2dMobilityModel", "Mode",
			StringValue("Time"), "Time", StringValue("5s"), "Speed",
			StringValue("ns3::ConstantRandomVariable[Constant=2.0]"), "Bounds",
			RectangleValue(Rectangle(xmin, xmax, ymin, ymax)));

	UeMobilityHelper60Kmh.SetMobilityModel(
			"ns3::RandomDirection2dMobilityModel", "Bounds",
			RectangleValue(Rectangle(xmin, xmax, ymin, ymax)), "Speed",
			StringValue("ns3::ConstantRandomVariable[Constant=17]"), "Pause",
			StringValue("ns3::ConstantRandomVariable[Constant=0.2]"));

	UeMobilityHelper120Kmh.SetMobilityModel(
			"ns3::RandomDirection2dMobilityModel", "Bounds",
			RectangleValue(Rectangle(xmin, xmax, ymin, ymax)), "Speed",
			StringValue("ns3::ConstantRandomVariable[Constant=36]"), "Pause",
			StringValue("ns3::ConstantRandomVariable[Constant=0.2]"));

	int numOfRandomUEs = 0.6 * numberOfUes;
	int numOf60KMpHUEs = 0.55 * numberOfUes; //17 m/s
	//int numOf120KMpHUEs = 0.05 * numberOfUes; //36 m/s

	for (int i = 0; i < numberOfUes; i++) {
		//
		if (i + 1 <= numOfRandomUEs) {
			UeMobilityHelper.Install(UENodes.Get(i));
		} else if (i + 1 <= numOfRandomUEs + numOf60KMpHUEs) {
			UeMobilityHelper60Kmh.Install(UENodes.Get(i));
		} else if (i + 1 <= numberOfUes) {
			UeMobilityHelper120Kmh.Install(UENodes.Get(i));
		}
	}
}

void UE::setNetAnimProperties(AnimationInterface* anim, int imageId) {
	for (int i = 0; i < numOfUEs; i++) {
		int nodeId = UENodes.Get(i)->GetId();
		anim->UpdateNodeImage(nodeId, imageId);
		anim->UpdateNodeSize(nodeId, 10, 10);
		anim->UpdateNodeDescription(UENodes.Get(i), "UE");
	}
	anim = 0;
}
}

#endif /* SCRATCH_UE_H_ */
