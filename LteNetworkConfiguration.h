/*
 * LteNetworkConfiguration.h
 *
 *  Created on: 01 Jun 2019
 *      Author: gift
 */

#ifndef SCRATCH_LTENETWORKCONFIGURATION_H_
#define SCRATCH_LTENETWORKCONFIGURATION_H_

#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/core-module.h"

namespace ns3 {
class LteNetworkConfiguration {
public:
	LteNetworkConfiguration();
	void setUpLteHelperWithEpc();
	void setUpEpc();
	void setUpPgw();
	void setUpSingleRemoteHost();
	void setUpTheInternet();
	enum DeviceTypes {
		Enb, UE
	};
	NetDeviceContainer generateLteEnbDevices(NodeContainer*,
			LteNetworkConfiguration::DeviceTypes);
	Ptr<LteHelper> getLteHelper(){ return lteHelper;}
	void connectEnbsWithX2Interface(NodeContainer);
	Ptr<Node> getPGW(){return pgw;}
	Ptr<Node> getRemoteHost(){return remoteHost;}
	void connectUeToNearestEnb(NetDeviceContainer*,NetDeviceContainer*);
	void installIpStackUe(NodeContainer*, NetDeviceContainer*);
private:
	Ptr<LteHelper> lteHelper;
	Ptr<PointToPointEpcHelper> epcHelper;
	Ptr<Node> pgw;
	NodeContainer remoteHostContainer;
	InternetStackHelper internet;
	Ptr<Node> remoteHost;
	Ipv4Address remoteHostAddr;
	Ipv4StaticRoutingHelper ipv4RoutingHelper;
	PointToPointHelper p2ph;
	NetDeviceContainer internetDevices;
	Ipv4AddressHelper ipv4h;
	Ipv4InterfaceContainer internetIpIfaces;
	Ipv4InterfaceContainer ueIpIfaces;
	Ptr<Ipv4StaticRouting> remoteHostStaticRouting;
	MobilityHelper lteMobility;
};

LteNetworkConfiguration::LteNetworkConfiguration() {
	//must follow this order
	setUpEpc();
	setUpLteHelperWithEpc();
	setUpPgw();
	setUpSingleRemoteHost();
	setUpTheInternet();
	lteMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
	Ptr<ListPositionAllocator> ltePositionAlloc = CreateObject<
			ListPositionAllocator>();
	ltePositionAlloc->Add(Vector(0, -20, 0));
	ltePositionAlloc->Add(Vector(0, -10, 0));
	lteMobility.SetPositionAllocator(ltePositionAlloc);
	lteMobility.Install(remoteHost);
	lteMobility.Install(pgw);
}

void LteNetworkConfiguration::setUpLteHelperWithEpc() {
	lteHelper = CreateObject<LteHelper>();
	lteHelper->SetEpcHelper(epcHelper);
}

void LteNetworkConfiguration::setUpEpc() {
	epcHelper = CreateObject<PointToPointEpcHelper>();
}

void LteNetworkConfiguration::setUpPgw() {
	pgw = epcHelper->GetPgwNode();
}

void LteNetworkConfiguration::setUpSingleRemoteHost() {
	remoteHostContainer.Create(1);
	remoteHost = remoteHostContainer.Get(0);
	internet.Install(remoteHostContainer);
}

void LteNetworkConfiguration::setUpTheInternet() {
	p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
	p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
	p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
	internetDevices = p2ph.Install(pgw, remoteHost);

	ipv4h.SetBase("1.0.0.0", "255.0.0.0");
	internetIpIfaces = ipv4h.Assign(internetDevices);
	remoteHostAddr = internetIpIfaces.GetAddress(1);

	//routing of the internet host(towards the Lte Network)
	remoteHostStaticRouting =
			ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
	// interface 0 is localhost, 1 is the p2p device
	remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"),
			Ipv4Mask("255.0.0.0"), 1);
}

NetDeviceContainer LteNetworkConfiguration::generateLteEnbDevices(
		NodeContainer* nodes, LteNetworkConfiguration::DeviceTypes type) {
	switch (type) {
	case DeviceTypes::Enb:
		return lteHelper->InstallEnbDevice(*nodes);
	case DeviceTypes::UE:
		return lteHelper->InstallUeDevice(*nodes);
	default:
		NetDeviceContainer c;
		return c;
	}
	nodes = NULL;
}

void LteNetworkConfiguration::connectEnbsWithX2Interface(NodeContainer nodes){
	lteHelper->AddX2Interface(nodes);
}

void LteNetworkConfiguration::connectUeToNearestEnb(NetDeviceContainer* Ues,NetDeviceContainer* Enbs){
	lteHelper->AttachToClosestEnb(*Ues,*Enbs);
	Ues = Enbs = NULL;
}

void LteNetworkConfiguration::installIpStackUe(NodeContainer* Ues, NetDeviceContainer* ueLteDevs){
	 internet.Install (*Ues);
	 ueIpIfaces = epcHelper->AssignUeIpv4Address (*ueLteDevs);

	 Ues = NULL;
	 ueLteDevs = NULL;
}
}

#endif /* SCRATCH_LTENETWORKCONFIGURATION_H_ */
