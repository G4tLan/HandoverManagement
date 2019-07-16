#include "ns3/core-module.h"
#include "ns3/config-store-module.h"
#include "ns3/netanim-module.h"
#include "Enbs.h"
#include "LteNetworkConfiguration.h"
#include "UE.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LenaX2HandoverMeasures");

/**
 * Sample simulation script for an automatic X2-based handover based on the RSRQ measures.
 * It instantiates two eNodeB, attaches one UE to the 'source' eNB.
 * The UE moves between both eNBs, it reports measures to the serving eNB and
 * the 'source' (serving) eNB triggers the handover of the UE towards
 * the 'target' eNB when it considers it is a better eNB.
 */

int main(int argc, char *argv[]) {
	int numberOfEnbs = 19;
	int numberOfUes = 0;
	int distance = 200; //m
	Enbs::Position_Types type = Enbs::HEX_MATRIX;

	CommandLine cmd;
	cmd.AddValue("nEnbs", "Number of Enbs", numberOfEnbs);
	cmd.AddValue("nUes", "Number of Ues", numberOfUes);
	cmd.AddValue("distance", "distance between enbs", distance);
	cmd.Parse(argc, argv);
	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults ();
	cmd.Parse(argc, argv);

	Enbs enbContainer(numberOfEnbs, distance, type);
	UE ueContainer(numberOfUes, 20, 20); //different speeds

	double eNbTxPower = 46; //dbm
	Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(eNbTxPower));

	//setup the network
	LteNetworkConfiguration lteNetwork;
	Ptr<LteHelper> lteHelper = lteNetwork.getLteHelper();
	//generate lte devices
	NetDeviceContainer enbLteDevs = lteNetwork.generateLteEnbDevices(
			enbContainer.getEnbs(), LteNetworkConfiguration::DeviceTypes::Enb);
	NetDeviceContainer ueLteDevs = lteNetwork.generateLteEnbDevices(
			ueContainer.getUes(), LteNetworkConfiguration::DeviceTypes::UE);

	enbContainer.ConnectClosestEnbX2Interface(lteHelper, type);

	lteNetwork.installIpStackUe(ueContainer.getUes(), &ueLteDevs);
	lteNetwork.connectUeToNearestEnb(&ueLteDevs, &enbLteDevs);
	lteNetwork.startApps(ueContainer.getUes(), &ueLteDevs);
	lteNetwork.setupTraces();


	//Setup netAnim settings
	AnimationInterface anim("./scratch/animation-simulation.xml");
	int UEImageId = anim.AddResource("./scratch/UE.png");
	int ENBImageId = anim.AddResource("./scratch/ENB.png");
	enbContainer.setNetAnimProperties(&anim, ENBImageId);
	ueContainer.setNetAnimProperties(&anim, UEImageId);

	//Measurements
	Ptr<FlowMonitor> flowMonitor;
	FlowMonitorHelper flowHelper;
	flowMonitor = flowHelper.InstallAll();

	Simulator::Stop(Seconds(15));
	std::cout << "simulation start" << std::endl;
	Simulator::Run();
	flowMonitor->SerializeToXmlFile("./scratch/Simulation-measurements.xml", true, true);

	Simulator::Destroy();
	return 0;
}

