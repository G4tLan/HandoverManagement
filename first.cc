#include "ns3/core-module.h"
#include "ns3/config-store-module.h"
#include "ns3/netanim-module.h"
#include "Enbs.h"
#include "LteNetworkConfiguration.h"
#include "UE.h"

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
	int numberOfEnbs = 5;
	int numberOfUes = 10;
	int distance = 50;

	CommandLine cmd;
	cmd.AddValue("nEnbs", "Number of Enbs", numberOfEnbs);
	cmd.AddValue("nUes", "Number of Ues", numberOfUes);
	cmd.AddValue("distance", "distance between enbs", distance);
	cmd.Parse(argc, argv);

	Enbs enbContainer(numberOfEnbs, distance);
	int xCenter = (enbContainer.GetNumOfEnbsInRow() + 1) * distance / 2;
	int yCenter = (enbContainer.GetNumOfRows() - 1) * distance / 2;
	int radius = yCenter;
	if (xCenter > yCenter) {
		radius = xCenter;
	}
	UE ueContainer(numberOfUes, xCenter, yCenter, radius + distance / 4);
	double eNbTxPower = 30;
	Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(eNbTxPower));
	Config::SetDefault("ns3::LteUePhy::TxPower", DoubleValue(10.0));

	//setup the network
	LteNetworkConfiguration lteNetwork;
	Ptr<LteHelper> lteHelper = lteNetwork.getLteHelper();
	//generate lte devices
	NetDeviceContainer enbLteDevs = lteNetwork.generateLteEnbDevices(
			enbContainer.getEnbs(), LteNetworkConfiguration::DeviceTypes::Enb);
	NetDeviceContainer ueLteDevs = lteNetwork.generateLteEnbDevices(
			ueContainer.getUes(), LteNetworkConfiguration::DeviceTypes::UE);

	enbContainer.ConnectClosestEnbX2Interface(lteHelper);

	lteNetwork.installIpStackUe(ueContainer.getUes(), &ueLteDevs);
	lteNetwork.connectUeToNearestEnb(&ueLteDevs, &enbLteDevs);

	//Setup netAnim settings
	AnimationInterface anim("./scratch/first.xml");
	int UEImageId = anim.AddResource("./scratch/UE.png");
	int ENBImageId = anim.AddResource("./scratch/ENB.png");
	enbContainer.setNetAnimProperties(&anim, ENBImageId);
	ueContainer.setNetAnimProperties(&anim, UEImageId);

	Simulator::Stop(Seconds(10));
	Simulator::Run();

	Simulator::Destroy();
	return 0;
}

