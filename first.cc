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

void
NotifyConnectionEstablishedUe (std::string context,
                               uint64_t imsi,
                               uint16_t cellId,
                               uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " UE IMSI " << imsi
            << ": connected to cellId " << cellId
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyConnectionEstablishedEnb (std::string context,
                                uint64_t imsi,
                                uint16_t cellId,
                                uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB cellId " << cellId
            << ": successful connection of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

/// Map each of UE RRC states to its string representation.
static const std::string g_ueRrcStateName[LteUeRrc::NUM_STATES] =
{
  "IDLE_START",
  "IDLE_CELL_SEARCH",
  "IDLE_WAIT_MIB_SIB1",
  "IDLE_WAIT_MIB",
  "IDLE_WAIT_SIB1",
  "IDLE_CAMPED_NORMALLY",
  "IDLE_WAIT_SIB2",
  "IDLE_RANDOM_ACCESS",
  "IDLE_CONNECTING",
  "CONNECTED_NORMALLY",
  "CONNECTED_HANDOVER",
  "CONNECTED_PHY_PROBLEM",
  "CONNECTED_REESTABLISHING"
};

/**
 * \param s The UE RRC state.
 * \return The string representation of the given state.
 */
static const std::string & ToString (LteUeRrc::State s)
{
  return g_ueRrcStateName[s];
}

void
UeStateTransition (uint64_t imsi, uint16_t cellId, uint16_t rnti, LteUeRrc::State oldState, LteUeRrc::State newState)
{
    std::cout << Simulator::Now ().GetSeconds ()
    << " UE with IMSI " << imsi << " camped or connected to cell " << cellId <<
    " transitions from "<< ToString (oldState) << " to " << ToString (newState)<<std::endl;
}

void
EnbTimerExpiry (uint64_t imsi, uint16_t rnti, uint16_t cellId, std::string cause)
{
  if(cause=="HandoverJoiningTimeout" || cause== "HandoverLeavingTimeout")
    {
   std::cout << Simulator::Now ().GetSeconds ()
            << " IMSI " << imsi << ", RNTI " << rnti << ", cellId " << cellId
            << ", ENB RRC " << cause << std::endl;
    }
}

void
NotifyConnectionReleaseAtEnodeB (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  std::cout<< Simulator::Now ().GetSeconds ()
            << " IMSI " << imsi << ", RNTI " << rnti << ", cellId " << cellId
            << ", UE context destroyed at eNodeB" << std::endl ;
}

void
NotifyHandoverStartUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellId,
                       uint16_t rnti,
                       uint16_t targetcellId)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " UE IMSI " << imsi
            << ": previously connected to cellId " << cellId
            << " with RNTI " << rnti
            << ", doing handover to cellId " << targetcellId
            << std::endl;
}

void
NotifyHandoverEndOkUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellId,
                       uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " UE IMSI " << imsi
            << ": successful handover to cellId " << cellId
            << " with RNTI " << rnti
            << std::endl;
}

void
NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellId,
                        uint16_t rnti,
                        uint16_t targetcellId)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB cellId " << cellId
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to cellId " << targetcellId
            << std::endl;
}

void
NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellId,
                        uint16_t rnti)
{
  std::cout << Simulator::Now ().GetSeconds () << " " << context
            << " eNB cellId " << cellId
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

void
NotifyHandoverEndErrorUe (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
 std::cout << Simulator::Now ().GetSeconds ()
            << " IMSI " << imsi << ", RNTI " << rnti << ", cellId " << cellId
            << ", UE RRC Handover Failed" << std::endl;
}

void
NotifyRandomAccessErrorUe (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
std::cout<< Simulator::Now ().GetSeconds ()
            << " IMSI " << imsi << ", RNTI " << rnti << ", cellId " << cellId
            << ", UE RRC Random access Failed" << std::endl;
}

void
HandoverFailureEnb (uint64_t imsi, uint16_t rnti, uint16_t cellId, std::string cause)
{
std::cout<< Simulator::Now ().GetSeconds ()
            << " IMSI " << imsi << ", RNTI " << rnti << ", cellId " << cellId
            << ", "<<cause << std::endl;
}


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
	int numberOfUes = 3;
	int distance = 433; //m  sqrt(3) * radius/2
	Enbs::Position_Types type = Enbs::HEX_MATRIX;
	double simulationTime = 20;
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

	  // connect custom trace sinks
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/ConnectionEstablished",
                   MakeCallback (&NotifyConnectionEstablishedEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/ConnectionEstablished",
                   MakeCallback (&NotifyConnectionEstablishedUe));
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteUeRrc/StateTransition",
                                 MakeCallback (&UeStateTransition));
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteEnbRrc/TimerExpiry",
                                 MakeCallback (&EnbTimerExpiry));
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",
                   MakeCallback (&NotifyHandoverStartEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverStart",
                   MakeCallback (&NotifyHandoverStartUe));
  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
                   MakeCallback (&NotifyHandoverEndOkEnb));
  Config::Connect ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndOk",
                   MakeCallback (&NotifyHandoverEndOkUe));
  Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteUeRrc/HandoverEndError",
                                     MakeCallback (&NotifyHandoverEndErrorUe));
   Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteUeRrc/RandomAccessError",
                                     MakeCallback (&NotifyRandomAccessErrorUe));
   Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverFailure",
                                     MakeCallback (&HandoverFailureEnb));
   Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteEnbRrc/NotifyConnectionRelease",
                                     MakeCallback (&NotifyConnectionReleaseAtEnodeB));

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

