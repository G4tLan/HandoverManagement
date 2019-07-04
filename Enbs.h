/*
 * Enbs.h
 *
 *  Created on: 01 Jun 2019
 *      Author: gift
 */

#ifndef SCRATCH_ENBS_H_
#define SCRATCH_ENBS_H_

#include "../build/ns3/mobility-helper.h"
#include "../build/ns3/node-container.h"
#include "../build/ns3/object.h"
#include "../build/ns3/position-allocator.h"
#include "../build/ns3/ptr.h"
#include "../build/ns3/vector.h"
#include "ns3/lte-module.h"
#include "ns3/netanim-module.h"

namespace ns3 {
class Enbs {
public:
	enum Position_Types {
		HEX_MATRIX, STRAIGHT_LINE
	};
	Enbs(int numOfEnbs, int distance, Enbs::Position_Types);
	NodeContainer* getEnbs() {
		return &enbNodes;
	}
	void ConnectClosestEnbX2Interface(Ptr<LteHelper>, Enbs::Position_Types);
	void setNetAnimProperties(AnimationInterface*, int);
	int GetNumOfEnbsInRow() {
		return numOfEnbsHorizontally;
	}
	int GetNumOfRows() {
		return numOfEnbRows;
	}
private:
	NodeContainer enbNodes;
	MobilityHelper enbMobility;
	int numOfEnb;
	int numOfEnbsHorizontally;
	int numOfEnbRows;

	Ptr<ListPositionAllocator> generateEnbLocationsHex(int numOfEnbs,
			int distance);
	Ptr<ListPositionAllocator> generateEnbLocationsStraight(int numOfEnbs,
			int distance);

	void ConnectClosestEnbX2InterfaceHex(Ptr<LteHelper>);
	void ConnectClosestEnbX2InterfaceStraight(Ptr<LteHelper>);
};

Enbs::Enbs(int numOfEnbs, int distance, Enbs::Position_Types p) {
	enbNodes.Create(numOfEnbs);
	numOfEnb = numOfEnbs;
	numOfEnbsHorizontally = 5;
	numOfEnbRows = 0;
	enbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	switch (p) {
	case Enbs::Position_Types::HEX_MATRIX:
		enbMobility.SetPositionAllocator(
				generateEnbLocationsHex(numOfEnbs, distance));
		break;
	case Enbs::Position_Types::STRAIGHT_LINE:
	default:
		enbMobility.SetPositionAllocator(
				generateEnbLocationsStraight(numOfEnbs, distance));
	}
	enbMobility.Install(enbNodes);
}

Ptr<ListPositionAllocator> Enbs::generateEnbLocationsStraight(int numOfEnbs,
		int distance) {
	Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<
			ListPositionAllocator>();

	for (int i = 0; i < numOfEnbs; i++) {
		enbPositionAlloc->Add(Vector(i * distance, 50, 0));
	}

	return enbPositionAlloc;
}

Ptr<ListPositionAllocator> Enbs::generateEnbLocationsHex(int numOfEnbs,
		int distance) {
	{
		Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<
				ListPositionAllocator>();
		if (numOfEnbs <= 5 && numOfEnbs > 1) {
			numOfEnbsHorizontally = numOfEnbs / 2;
		}
		numOfEnbRows = (numOfEnbs + 1) / numOfEnbsHorizontally;
		if (numOfEnbs == 1 || numOfEnbs > 5) {
			numOfEnbRows += 1;
		}
		int count = 0;
		for (int y = 0; y < numOfEnbRows; y++) {
			for (int x = 0; x < numOfEnbsHorizontally; x++) {
				count++;
				if (count > numOfEnbs) {
					break;
				}
				if (y % 2 == 0) {
					enbPositionAlloc->Add(
							Vector(2 * distance * x, y * distance, 0));
				} else {
					enbPositionAlloc->Add(
							Vector((2 * distance * x + distance), y * distance,
									0));
				}
			}
		}
		return enbPositionAlloc;
	}
}

void Enbs::ConnectClosestEnbX2InterfaceHex(Ptr<LteHelper> lteHelper) {
	for (int row = 0; row < numOfEnbRows; row++) {
		int start = row * numOfEnbsHorizontally;
		int end = (row + 1) * numOfEnbsHorizontally - 1;
		int n = start;
		for (int enbAtRow = 0; enbAtRow < numOfEnbsHorizontally; enbAtRow++) {
			if (n >= numOfEnb) {
				break;
			}
			//connect to enb on the left
			if (n > start) {
				lteHelper->AddX2Interface(enbNodes.Get(n - 1), enbNodes.Get(n));
			}
			//if first row
			if (row == 0) {
				n++;
				continue;
			}
			enbNodes.GetN();
			//if row is odd
			if (row % 2 == 1) {
				lteHelper->AddX2Interface(
						enbNodes.Get(n - numOfEnbsHorizontally),
						enbNodes.Get(n));
				if (n < end) {
					lteHelper->AddX2Interface(
							enbNodes.Get(n - numOfEnbsHorizontally + 1),
							enbNodes.Get(n));
				}
			} else { //row is even
				lteHelper->AddX2Interface(
						enbNodes.Get(n - numOfEnbsHorizontally),
						enbNodes.Get(n));
				if (n > start) {
					lteHelper->AddX2Interface(
							enbNodes.Get(n - numOfEnbsHorizontally - 1),
							enbNodes.Get(n));
				}
			}
			n++;
		}
	}
}

void Enbs::ConnectClosestEnbX2InterfaceStraight(Ptr<LteHelper> lteHelper) {
	for (int i = 1; i < numOfEnb; i++) {
		lteHelper->AddX2Interface(enbNodes.Get(i - 1), enbNodes.Get(i));
	}
}

void Enbs::ConnectClosestEnbX2Interface(Ptr<LteHelper> lteHelper,
		Enbs::Position_Types p) {
	switch(p){
	case Enbs::Position_Types::HEX_MATRIX:
		ConnectClosestEnbX2InterfaceHex(lteHelper);
		break;
	case Enbs::Position_Types::STRAIGHT_LINE:
		ConnectClosestEnbX2InterfaceStraight(lteHelper);
		break;
	default:
		break;
	}
}

void Enbs::setNetAnimProperties(AnimationInterface* anim, int imageId) {
	for (int i = 0; i < numOfEnb; i++) {
		int nodeId = enbNodes.Get(i)->GetId();
		anim->UpdateNodeImage(nodeId, imageId);
		anim->UpdateNodeSize(nodeId, 15, 15);
		anim->UpdateNodeDescription(enbNodes.Get(i), "Enb");
	}
	anim = 0;
}

}

#endif /* SCRATCH_ENBS_H_ */
