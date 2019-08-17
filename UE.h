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
#include <ns3/constant-velocity-mobility-model.h>
#include <string>

namespace ns3
{
double calculateDistance(Vector p1, Vector p2)
{
	double dist = (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);

	return std::sqrt(dist);
}

class UE
{
public:
	struct historyPos
	{
		Vector p1;
		Vector p2;
	};
	//uses rnti and cellid in that order as key
	static std::map< uint64_t, UE::historyPos> uePositionHistory;
	UE(int, int, int, int, int);
	UE(int,int,int,int);//for testing
	void setNetAnimProperties(AnimationInterface *, int);
	NodeContainer *getUes()
	{
		return &UENodes;
	}

	void updateUePositionHistory();

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
	int simulationTime;
};
std::map<uint64_t, UE::historyPos> UE::uePositionHistory = {};

void UE::updateUePositionHistory()
{
	double currentTime = Simulator::Now().GetSeconds();
	for (uint n = 0; n < UENodes.GetN(); n++)
	{
		Ptr<Node> node = UENodes.Get(n)->GetObject<Node>();
		Ptr<LteUeNetDevice> ueNetDev = 0;
		for(uint32_t devs = 0; devs < UENodes.Get(n)->GetNDevices(); devs++){
			ueNetDev = UENodes.Get(n)->GetDevice(devs)->GetObject<LteUeNetDevice>();
			if(ueNetDev != 0){
				break;
			}
		}
		Ptr<MobilityModel> mob = UENodes.Get(n)->GetObject<MobilityModel>();
		auto it = uePositionHistory.find(ueNetDev->GetImsi());

		if (it == uePositionHistory.end())
		{
			UE::historyPos historyP1P2;
			historyP1P2.p1 = mob->GetPosition();
			historyP1P2.p2 = mob->GetPosition();
			uePositionHistory.insert(std::make_pair(ueNetDev->GetImsi(), historyP1P2));
		}
		else
		{
			if (calculateDistance(mob->GetPosition(), it->second.p2) >= loggingDistance)
			{
				it->second.p1 = it->second.p2;
				it->second.p2 = mob->GetPosition();
			}
		}
	}
	double t = currentTime + 0.1;
	Simulator::Schedule(Seconds(t >= simulationTime ? simulationTime : t),
						&UE::updateUePositionHistory, this);
}

UE::UE(int numberOfUes, int xBound, int yBound, int _simulationTime):
xCenter(0), yCenter(0), radius(0), loggingDistance(30) {
	numOfUEs = numberOfUes;
	UENodes.Create(numberOfUes);

	Ptr<ListPositionAllocator> positionAlloc = CreateObject<
			ListPositionAllocator>();
	for (int i = 0; i < numberOfUes; i++) {
		positionAlloc->Add(Vector(xBound, yBound, 0));
	}

	UeMobilityHelper.SetPositionAllocator(positionAlloc);
	UeMobilityHelper.SetMobilityModel("ns3::ConstantVelocityMobilityModel");
	UeMobilityHelper.Install(UENodes);
	
	for (uint n = 0; n < UENodes.GetN(); n++) {
		Ptr<ConstantVelocityMobilityModel> mob  =
				UENodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
		//mob->SetVelocity(Vector(10, 0, 0));
		if(mob != 0)
		mob->SetVelocity(Vector(55,0,0));
	}

	Simulator::Schedule(Seconds(0),
					&UE::updateUePositionHistory, this);
}

UE::UE(int numberOfUes, int xBound, int yBound, int _radius, int _simulationTime) : loggingDistance(10)
{
	numOfUEs = numberOfUes;
	simulationTime = _simulationTime;
	UENodes.Create(numberOfUes);

	UeMobilityHelper.SetPositionAllocator("ns3::RandomDiscPositionAllocator",
										  "X", StringValue(std::to_string(xBound)), "Y",
										  StringValue(std::to_string(yBound)), "Rho",
										  StringValue(
											  "ns3::UniformRandomVariable[Min=5|Max=" + std::to_string(_radius) + "]"));

	UeMobilityHelper60Kmh.SetPositionAllocator(
		"ns3::RandomDiscPositionAllocator", "X",
		StringValue(std::to_string(xBound)), "Y",
		StringValue(std::to_string(yBound)), "Rho",
		StringValue(
			"ns3::UniformRandomVariable[Min=5|Max=" + std::to_string(_radius) + "]"));

	UeMobilityHelper120Kmh.SetPositionAllocator(
		"ns3::RandomDiscPositionAllocator", "X",
		StringValue(std::to_string(xBound)), "Y",
		StringValue(std::to_string(yBound)), "Rho",
		StringValue(
			"ns3::UniformRandomVariable[Min=5|Max=" + std::to_string(_radius) + "]"));

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

	for (int i = 0; i < numberOfUes; i++)
	{
		//
		if (i + 1 <= numOfRandomUEs)
		{
			UeMobilityHelper.Install(UENodes.Get(i));
		}
		else if (i + 1 <= numOfRandomUEs + numOf60KMpHUEs)
		{
			UeMobilityHelper60Kmh.Install(UENodes.Get(i));
		}
		else if (i + 1 <= numberOfUes)
		{
			UeMobilityHelper120Kmh.Install(UENodes.Get(i));
		}
	}
	//updateUePositionHistory();
	Simulator::Schedule(Seconds(0),
						&UE::updateUePositionHistory, this);
}

void UE::setNetAnimProperties(AnimationInterface *anim, int imageId)
{
	for (int i = 0; i < numOfUEs; i++)
	{
		int nodeId = UENodes.Get(i)->GetId();
		anim->UpdateNodeImage(nodeId, imageId);
		anim->UpdateNodeSize(nodeId, 20, 20);
		anim->UpdateNodeDescription(UENodes.Get(i), "UE");
	}
	anim = 0;
}
} // namespace ns3

#endif /* SCRATCH_UE_H_ */
