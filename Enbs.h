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

	struct X2ConnectionPair {
		int node1Index;
		int node2Index;
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
	std::vector<Enbs::X2ConnectionPair> pairs;

	Ptr<ListPositionAllocator> generateEnbLocationsHex(int numOfEnbs,
			int distance);
	Ptr<ListPositionAllocator> generateEnbLocationsStraight(int numOfEnbs,
			int distance);
	Enbs::X2ConnectionPair createPair(int a, int b){
		Enbs::X2ConnectionPair pair;
		pair.node1Index = a;
		pair.node2Index = b;
		return pair;
	}

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

bool checkExists(ListPositionAllocator L, Vector v, int& index) {
	double epsilon = 0.0001;
	Vector existing;
	for (uint32_t i = 0; i < L.GetSize(); i++) {
		Vector existing = L.GetNext();
		if (std::fabs(v.x - existing.x) <= epsilon) {
			if (std::fabs(v.y - existing.y) <= epsilon) {
				index = i;
				return true;
			}
		}
	}
	index = L.GetSize();
	return false;
}

std::vector<Vector> generatePoints(Vector refPoint, int distance) {
	std::vector<Vector> points;
	//0 rad
	points.push_back(Vector(refPoint.x + distance, refPoint.y, 0));
	// pi/3 rad
	points.push_back(
			Vector(refPoint.x + distance / 2,
					refPoint.y + distance * 0.86602540378, 0));
	// 2pi/3 rad
	points.push_back(
			Vector(refPoint.x - distance / 2,
					refPoint.y + distance * 0.86602540378, 0));
	// pi rad
	points.push_back(Vector(refPoint.x - distance, refPoint.y, 0));
	// 4pi/3 rad
	points.push_back(
			Vector(refPoint.x - distance / 2,
					refPoint.y - distance * 0.86602540378, 0));
	// 5pi/3 rad
	points.push_back(
			Vector(refPoint.x + distance / 2,
					refPoint.y - distance * 0.86602540378, 0));

	return points;
}

bool checkPairsExist(std::vector<Enbs::X2ConnectionPair> pairs, Enbs::X2ConnectionPair pair) {

	for(uint32_t  i = 0; i< pairs.size(); i++){
		Enbs::X2ConnectionPair temp = pairs.at(i);

		if((pair.node1Index == temp.node1Index && pair.node2Index == temp.node2Index)
				|| (pair.node1Index == temp.node2Index && pair.node2Index == temp.node1Index)){
			return true;
		}
	}
	return false;
}

Ptr<ListPositionAllocator> Enbs::generateEnbLocationsHex(int numOfEnbs,
		int distance) {
	{
		Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<
				ListPositionAllocator>();

		Vector refPoint = Vector(1000, 1000, 0);
		enbPositionAlloc->Add(refPoint);
		std::vector<Vector> points = generatePoints(refPoint, distance);

		numOfEnbs = numOfEnbs - 1;
		int i = 0, count = 0, refPointIndex = 0, pointIndex = -1, prevPointIndex = -1, startPointIndex = -1;
		 while (count < numOfEnbs) {
			if (!checkExists(*enbPositionAlloc, points.at(i), pointIndex)) {
				std::cout << points.at(i).x << " , " << points.at(i).y << " index " << pointIndex << std::endl;
				enbPositionAlloc->Add(points.at(i));
				/*
				 * parent refers refpoint
				 * child refers to point
				 */
				pairs.push_back(createPair(refPointIndex, pointIndex)); //pair parent with child
				//pair current child with previous child
				if(prevPointIndex > 0){ //exclude first child
					if(!checkPairsExist(pairs, createPair(prevPointIndex, pointIndex))){
						pairs.push_back(createPair(prevPointIndex, pointIndex));
					}
				}
				//last child i = 5
				count++;
			} else {
				if(!checkPairsExist(pairs, createPair(refPointIndex, pointIndex))){
					pairs.push_back(createPair(refPointIndex, pointIndex));
				}
			}
			prevPointIndex = pointIndex;
			if(i == 0) {
				startPointIndex = pointIndex;
			}
			if(i == 5){
				if(!checkPairsExist(pairs, createPair(startPointIndex, pointIndex))){
					pairs.push_back(createPair(startPointIndex, pointIndex));
				}
			}

			i = (i + 1) % 6;
			if (i == 0) {
				refPointIndex++;
				for(int ind = 0; ind <= refPointIndex; ind++){
					refPoint = enbPositionAlloc->GetNext();
				}
				for(int reset = 0; reset < count - refPointIndex; reset++){
					enbPositionAlloc->GetNext();
				}
				points = generatePoints(refPoint, distance);
			}
		};
		return enbPositionAlloc;
	}
}

void Enbs::ConnectClosestEnbX2InterfaceHex(Ptr<LteHelper> lteHelper) {
		int numberOfPairs = pairs.size();
		for(int pair = 0; pair < numberOfPairs; pair++){
			lteHelper->AddX2Interface(enbNodes.Get(pairs.at(pair).node1Index),
					enbNodes.Get(pairs.at(pair).node2Index));
		}
}

void Enbs::ConnectClosestEnbX2InterfaceStraight(Ptr<LteHelper> lteHelper) {
	for (int i = 1; i < numOfEnb; i++) {
		lteHelper->AddX2Interface(enbNodes.Get(i - 1), enbNodes.Get(i));
	}
}

void Enbs::ConnectClosestEnbX2Interface(Ptr<LteHelper> lteHelper,
		Enbs::Position_Types p) {
	switch (p) {
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
