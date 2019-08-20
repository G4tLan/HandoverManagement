#include "ns3/core-module.h"
#include "ns3/config-store-module.h"
#include "ns3/netanim-module.h"
#include "Enbs.h"
#include "LteNetworkConfiguration.h"
#include "UE.h"
#include "ns3/flow-monitor-module.h"
#include <time.h>

//https://gitlab.cc-asp.fraunhofer.de/elena-ns3-lte/elena/tree/master ELENA

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LenaX2HandoverMeasures");

/**
 * Sample simulation script for an automatic X2-based handover based on the RSRQ measures.
 * It instantiates two eNodeB, attaches one UE to the 'source' eNB.
 * The UE moves between both eNBs, it reports measures to the serving eNB and
 * the 'source' (serving) eNB triggers the handover of the UE towards
 * the 'target' eNB when it considers it is a better eNB.
 */

time_t start = time(0);

void simulationCompletion(double totalSimTime ) {
	ns3::Time currentTime = Simulator::Now();
	double percentage = currentTime.GetMilliSeconds()/(totalSimTime*10);
	std::cout << "simulation completion "
	 << percentage << "% estimated time " << (100.0/percentage) * difftime(time(0), start) << std::endl;
	Simulator::Schedule(Seconds(currentTime.GetMilliSeconds()/1000.0 + 0.5), 
	&simulationCompletion, totalSimTime);
}

void accessPositions(std::string context, const std::map<uint32_t, UE::historyPos> poses){
	std::cout << "poses" << std::endl;
}

int main(int argc, char *argv[]) {
	int numberOfEnbs = 7;
	int numberOfUes = 42;
	int distance = 433; //m  sqrt(3) * radius/2
	Enbs::Position_Types type = Enbs::HEX_MATRIX;
	double simulationTime = 15;
	double eNbTxPower = 43; //dbm
	int xCenter = 512;
	int yCenter = 512;

	CommandLine cmd;
	cmd.AddValue("nEnbs", "Number of Enbs", numberOfEnbs);
	cmd.AddValue("nUes", "Number of Ues", numberOfUes);
	cmd.AddValue("distance", "distance between enbs", distance);
	cmd.Parse(argc, argv);
	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults ();
	cmd.Parse(argc, argv);

	Enbs enbContainer(numberOfEnbs, distance, type);
	Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(eNbTxPower));
	Config::SetDefault ("ns3::RrFfMacScheduler::HarqEnabled", BooleanValue (false));


	//setup the network
	LteNetworkConfiguration lteNetwork;
	Ptr<LteHelper> lteHelper = lteNetwork.getLteHelper();
	//generate lte devices
	NetDeviceContainer enbLteDevs = lteNetwork.generateLteEnbDevices(
			enbContainer.getEnbs(), LteNetworkConfiguration::DeviceTypes::Enb);

	// int radius = 500; //change on the algorithm as well
	// UE ueContainer(numberOfUes, xCenter, yCenter, radius + distance / 4,simulationTime,Enbs::enbPositions);
	enbContainer.populatePositions();
	UE ueContainer(numberOfUes,xCenter,yCenter,simulationTime,Enbs::enbPositions);

	NetDeviceContainer ueLteDevs = lteNetwork.generateLteEnbDevices(
			ueContainer.getUes(), LteNetworkConfiguration::DeviceTypes::UE);

	enbContainer.ConnectClosestEnbX2Interface(lteHelper, type);

	lteNetwork.installIpStackUe(ueContainer.getUes(), &ueLteDevs);
	lteNetwork.connectUeToNearestEnb(&ueLteDevs, &enbLteDevs);
	lteNetwork.startApps(ueContainer.getUes(), &ueLteDevs);
	lteNetwork.setupTraces();
	//ueContainer.updateUePositionHistory();

	enbContainer.populateNeighbours();

	//Setup netAnim settings
	AnimationInterface anim("./scratch/animation-simulation.xml");
	anim.SetMaxPktsPerTraceFile(300000000);
	int UEImageId = anim.AddResource("./scratch/UE.png");
	int ENBImageId = anim.AddResource("./scratch/ENB.png");
	enbContainer.setNetAnimProperties(&anim, ENBImageId);
	ueContainer.setNetAnimProperties(&anim, UEImageId);

	//Measurements
	Ptr<FlowMonitor> flowMonitor;
	FlowMonitorHelper flowHelper;
	flowMonitor = flowHelper.InstallAll();

	Simulator::Schedule(Seconds(0), &simulationCompletion, simulationTime);
	//ueContainer.TraceConnectWithoutContext("UEHistoryPositions", MakeCallback(&accessPositions));
	Simulator::Stop(Seconds(simulationTime));
	std::cout << "simulation start" << std::endl;
	Simulator::Run();
	flowMonitor->SerializeToXmlFile("./scratch/Simulation-measurements.xml", true, true);

	Simulator::Destroy();
	return 0;
}

