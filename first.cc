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

void simulationCompletion(double totalSimTime ) {
	ns3::Time currentTime = Simulator::Now();
	std::cout << "simulation completion "
	 << currentTime.GetMilliSeconds()/(totalSimTime*10) << "%" << std::endl;
	Simulator::Schedule(Seconds(currentTime.GetMilliSeconds()/1000.0 + 0.5), 
	&simulationCompletion, totalSimTime);
}

double calculateDistance(Vector p1, Vector p2){
	double dist  = (p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y);

	return std::sqrt(dist);
}

void updateUePositionHistory(NodeContainer* UENodes, 
		std::map<uint32_t, UE::historyPos>* uePositionHistory,
		double loggingDistance) {
	double currentTime = Simulator::Now().GetSeconds();
	for (uint n = 0; n < UENodes->GetN(); n++) {
		Ptr<Node> node = UENodes->Get(n)->GetObject<Node>();
		Ptr<MobilityModel> mob = UENodes->Get(n)->GetObject<MobilityModel>();
		auto it = uePositionHistory->find(node->GetId());

		if(it == uePositionHistory->end()){
			UE::historyPos historyP1P2;
			historyP1P2.p1 = mob->GetPosition();
			historyP1P2.p2 = mob->GetPosition();
			uePositionHistory->insert(std::make_pair(node->GetId(), historyP1P2));
		} else {
			if(calculateDistance(mob->GetPosition(), it->second.p2) >= loggingDistance){
				std::cout << "-----adding for node " << node->GetId() << " t " 
					<< currentTime << std::endl;
				it->second.p1 = it->second.p2;
				it->second.p2 = mob->GetPosition();
			}
		}
	}

	UENodes = NULL;
	uePositionHistory = NULL;
}

void accessPositions(std::string context, std::map<uint32_t, UE::historyPos> poses){
	std::cout << "poses" << std::endl;
}

int main(int argc, char *argv[]) {
	int numberOfEnbs = 11;
	int numberOfUes = 1;
	int distance = 200; //m
	Enbs::Position_Types type = Enbs::HEX_MATRIX;
	double simulationTime = 20;

	CommandLine cmd;
	cmd.AddValue("nEnbs", "Number of Enbs", numberOfEnbs);
	cmd.AddValue("nUes", "Number of Ues", numberOfUes);
	cmd.AddValue("distance", "distance between enbs", distance);
	cmd.Parse(argc, argv);
	ConfigStore inputConfig;
	inputConfig.ConfigureDefaults ();
	cmd.Parse(argc, argv);

	Enbs enbContainer(numberOfEnbs, distance, type);
	//UE ueContainer(numberOfUes, 20, 20); //different speeds
	int xCenter = 512;
	int yCenter = 512;
	int radius = 300;
	UE ueContainer(numberOfUes, xCenter, yCenter, radius + distance / 4);
	updateUePositionHistory(ueContainer.getUes(), ueContainer.getUEPositionHistory(), 
		ueContainer.getLoggingDistance());

	double eNbTxPower = 43; //dbm
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

	Simulator::Schedule(Seconds(0), &simulationCompletion, simulationTime);
	for(double timer = 0; timer <= simulationTime; timer += 0.9){
		Simulator::Schedule(Seconds(timer), updateUePositionHistory, ueContainer.getUes(),
			ueContainer.getUEPositionHistory(), ueContainer.getLoggingDistance());
	}
	Config::Connect("/ns3::UE/UEHistoryPositins", MakeCallback(&accessPositions));
	Simulator::Stop(Seconds(simulationTime));
	std::cout << "simulation start" << std::endl;
	Simulator::Run();
	flowMonitor->SerializeToXmlFile("./scratch/Simulation-measurements.xml", true, true);

	Simulator::Destroy();
	return 0;
}

