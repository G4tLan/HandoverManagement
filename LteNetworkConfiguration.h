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
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"

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
	void startApps(NodeContainer*, NetDeviceContainer*);
	void setupTraces();
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
	Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (10)));
	Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
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
	lteHelper->SetHandoverAlgorithmType ("ns3::algorithmAdam");
	/*lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold",
			UintegerValue (30));
	lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset",
			UintegerValue (1));*/
	lteHelper->SetSchedulerType("ns3::RrFfMacScheduler");
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

void LteNetworkConfiguration::startApps(NodeContainer* ueNodes, NetDeviceContainer* ueLteDevs){

	// Install and start applications on UEs and remote host
	uint16_t dlPort = 10000;
	uint16_t ulPort = 20000;

	// randomize a bit start times to avoid simulation artifacts
	// (e.g., buffer overflows due to packet transmissions happening
	// exactly at the same time)
	Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
	startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
	startTimeSeconds->SetAttribute ("Max", DoubleValue (0.010));

	uint32_t numberOfUes = ueNodes->GetN();

	for (uint32_t u = 0; u < numberOfUes; ++u)
	{
		Ptr<Node> ue = ueNodes->Get (u);
		// Set the default gateway for the UE
		Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
		ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

		for (uint32_t b = 0; b < 1; ++b)
		{
			++dlPort;
			++ulPort;

			ApplicationContainer clientApps;
			ApplicationContainer serverApps;

			UdpClientHelper dlClientHelper (ueIpIfaces.GetAddress (u), dlPort);
			clientApps.Add (dlClientHelper.Install (remoteHost));
			PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
					InetSocketAddress (Ipv4Address::GetAny (), dlPort));
			serverApps.Add (dlPacketSinkHelper.Install (ue));

			UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
			clientApps.Add (ulClientHelper.Install (ue));
			PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory",
					InetSocketAddress (Ipv4Address::GetAny (), ulPort));
			serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

			Ptr<EpcTft> tft = Create<EpcTft> ();
			EpcTft::PacketFilter dlpf;
			dlpf.localPortStart = dlPort;
			dlpf.localPortEnd = dlPort;
			tft->Add (dlpf);
			EpcTft::PacketFilter ulpf;
			ulpf.remotePortStart = ulPort;
			ulpf.remotePortEnd = ulPort;
			tft->Add (ulpf);
			EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
			lteHelper->ActivateDedicatedEpsBearer (ueLteDevs->Get (u), bearer, tft);

			Time startTime = Seconds (startTimeSeconds->GetValue ());
			serverApps.Start (startTime);
			clientApps.Start (startTime);

		} // end for b
	}
	ueNodes = NULL;
	ueLteDevs = NULL;
}

void LteNetworkConfiguration::setupTraces(){
	lteHelper->EnablePhyTraces ();
	lteHelper->EnableMacTraces ();
	lteHelper->EnableRlcTraces ();
	lteHelper->EnablePdcpTraces ();
}

}

#endif /* SCRATCH_LTENETWORKCONFIGURATION_H_ */
