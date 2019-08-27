#include "LteNetworkConfiguration.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/config-store-module.h"
#include <ns3/songMoonAlgorithm.h>
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include <ns3/Enbs.h>
#include <ns3/UE.h>
#include <time.h>

int numOfHAndoverSucceess = 0;
int numOfHAndoverFail = 0;
int numOfHAndoverInit = 0;
int numOfHAndoverPingPong = 0;
int numOfRLF = 0;
int numOfTooLateHO = 0;
int numOfTooEarlyHO = 0;
double HPI = 0;

ns3::songMoonAlgorithm::RLFStats calculateHPI(){
  return {
      (double)(numOfRLF)/(double)(numOfHAndoverInit?numOfHAndoverInit:0.0000001),
      (double)numOfTooLateHO/(double)(numOfHAndoverInit?numOfHAndoverInit:0.0000001),
      (double)numOfTooEarlyHO/(double)(numOfHAndoverInit?numOfHAndoverInit:0.0000001)
      };
}

struct cellUePair {
  uint16_t rnti;
  uint16_t connectedCellId;
  uint16_t targetCellId;
  ns3::Time time;
};

static std::map<uint64_t, cellUePair> ongoingHandovers;
static std::map<uint64_t, cellUePair> successfulHandovers;

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
// static const std::string & ToString (LteUeRrc::State s)
// {
//   return g_ueRrcStateName[s];
// }

void
UeStateTransition (uint64_t imsi, uint16_t cellId, uint16_t rnti, LteUeRrc::State oldState, LteUeRrc::State newState)
{
    // std::cout << Simulator::Now ().GetSeconds ()
    // << " UE with IMSI " << imsi << " camped or connected to cell " << cellId <<
    // " transitions from "<< ToString (oldState) << " to " << ToString (newState)<<std::endl;
}

void
EnbTimerExpiry (uint64_t imsi, uint16_t rnti, uint16_t cellId, std::string cause)
{
  if(cause=="HandoverJoiningTimeout" || cause== "HandoverLeavingTimeout")
    {
  //  std::cout << Simulator::Now ().GetSeconds ()
  //           << " IMSI " << imsi << ", RNTI " << rnti << ", cellId " << cellId
  //           << ", ENB RRC " << cause << std::endl;
    }
}

void
NotifyConnectionReleaseAtEnodeB (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  // std::cout<< Simulator::Now ().GetSeconds ()
  //           << " IMSI " << imsi << ", RNTI " << rnti << ", cellId " << cellId
  //           << ", UE context destroyed at eNodeB" << std::endl ;
}

void
NotifyHandoverStartUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellId,
                       uint16_t rnti,
                       uint16_t targetcellId)
{
  // std::cout << Simulator::Now ().GetSeconds () << " " << context
  //           << " UE IMSI " << imsi
  //           << ": previously connected to cellId " << cellId
  //           << " with RNTI " << rnti
  //           << ", doing handover to cellId " << targetcellId
  //           << std::endl;
}

void
NotifyHandoverEndOkUe (std::string context,
                       uint64_t imsi,
                       uint16_t cellId,
                       uint16_t rnti)
{
  // std::cout << Simulator::Now ().GetSeconds () << " " << context
  //           << " UE IMSI " << imsi
  //           << ": successful handover to cellId " << cellId
  //           << " with RNTI " << rnti
  //           << std::endl;
}

void
NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellId,
                        uint16_t rnti,
                        uint16_t targetcellId)
{
  auto it = ongoingHandovers.find(imsi);
  cellUePair pair = { rnti, cellId, targetcellId, Simulator::Now()};
  if(it != ongoingHandovers.end()){
    if(cellId == it->second.targetCellId && targetcellId == it->second.connectedCellId){
      double seconds = Simulator::Now().GetSeconds() - it->second.time.GetSeconds();
      if(seconds <= 2){
        numOfHAndoverPingPong+=1;
        it->second = pair;
      }
    }
  }
  ongoingHandovers.insert(std::make_pair(imsi,pair));
  // std::cout << Simulator::Now ().GetSeconds () << " " << context
  //           << " eNB cellId " << cellId
  //           << ": start handover of UE with IMSI " << imsi
  //           << " RNTI " << rnti
  //           << " to cellId " << targetcellId
  //           << std::endl;
  numOfHAndoverInit+=1;
}

void
NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellId,
                        uint16_t rnti)
{
  auto it = ongoingHandovers.find(imsi);
  it->second.time = Simulator::Now();
  successfulHandovers[imsi] = it->second;
  ongoingHandovers.erase(imsi);
  // std::cout << Simulator::Now ().GetSeconds () << " " << context
  //           << " eNB cellId " << cellId
  //           << ": completed handover of UE with IMSI " << imsi
  //           << " RNTI " << rnti
  //           << std::endl;
  numOfHAndoverSucceess+=1;
  ns3::songMoonAlgorithm::updateParameters(calculateHPI());
}

void
NotifyHandoverEndErrorUe (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  // std::cout << Simulator::Now ().GetSeconds ()
  //           << " IMSI " << imsi << ", RNTI " << rnti << ", cellId " << cellId
  //           << ", UE RRC Handover Failed" << std::endl;
}

void
NotifyRadioLinkFailureUe (uint64_t imsi, uint16_t rnti, uint16_t cellId)
{
  // std::cout<< Simulator::Now ().GetSeconds ()
  //           << " IMSI " << imsi << ", RNTI " << rnti << ", cellId " << cellId
  //           << ", UE RRC Random access Failed" << std::endl;
  auto it = successfulHandovers.find(imsi);
  if(it != successfulHandovers.end()){
    std::cout << "\n handover diff " <<  Simulator::Now().GetSeconds() - it->second.time.GetSeconds() << std::endl << std::endl;
    if( (Simulator::Now().GetSeconds() - it->second.time.GetSeconds()) <= 1){
      numOfTooEarlyHO+=1;
    } else {
      numOfTooLateHO+=1;
    }
  }
  numOfRLF+=1;
  ns3::songMoonAlgorithm::updateParameters(calculateHPI());
  ns3::songMoonAlgorithm::updateMeasConf = true;
}

void
HandoverFailureEnb (uint64_t imsi, uint16_t rnti, uint16_t cellId, std::string cause)
{
  // std::cout<< Simulator::Now ().GetSeconds ()
  //           << " IMSI " << imsi << ", RNTI " << rnti << ", cellId " << cellId
  //           << ", "<<cause << std::endl;
  numOfHAndoverFail+=1;
  numOfTooLateHO+=1;//
  ongoingHandovers.erase(imsi);

  ns3::songMoonAlgorithm::updateParameters(calculateHPI());
  ns3::songMoonAlgorithm::updateMeasConf = true;
}

void
NotifyNewUeContext(const uint16_t cellId, const uint16_t rnti){
  // std::cout<< Simulator::Now ().GetSeconds ()
  //         << " CellID " << cellId << " RNTI " << rnti << " addded UE" << std::endl;
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
  HPI = currentTime.GetSeconds();
	double percentage = currentTime.GetSeconds()/(totalSimTime);
  time_t epoch = start + difftime(time(0), start)/(percentage?percentage:0.0000001);
	std::cout << "simulation completion "
	 << percentage*100 << "% estimated time " << asctime(localtime(&epoch)) << std::endl;
	Simulator::Schedule(Seconds(0.5),
	&simulationCompletion, totalSimTime);
}

void accessPositions(std::string context, const std::map<uint32_t, UE::historyPos> poses){
	std::cout << "poses" << std::endl;
}


int main(int argc, char *argv[]) {
	int numberOfEnbs = 7;
	int numberOfUes = 126;
	int distance = 600; //m  sqrt(3) * radius/2
	Enbs::Position_Types type = Enbs::HEX_MATRIX;
	double simulationTime = 90;
	double eNbTxPower = 43; //dbm
	int xCenter = 800;
	int yCenter = 800;

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
  // Config::SetDefault ("ns3::LteEnbRrc::HandoverJoiningTimeoutDuration", TimeValue (MilliSeconds (500)));
  Config::SetDefault ("ns3::songMoonAlgorithm::NumberOfNeighbours", UintegerValue(numberOfEnbs));
  Config::SetDefault ("ns3::songMoonAlgorithm::ThresholdChange", UintegerValue(3));

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
//	lteNetwork.setupTraces();
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
   Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteUeRrc/RadioLinkFailure",
                                     MakeCallback (&NotifyRadioLinkFailureUe));
   Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverFailure",
                                     MakeCallback (&HandoverFailureEnb));
   Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteEnbRrc/NotifyConnectionRelease",
                                     MakeCallback (&NotifyConnectionReleaseAtEnodeB));
   Config::ConnectWithoutContext ("/NodeList/*/DeviceList/*/LteEnbRrc/NewUeContext",
                                     MakeCallback (&NotifyNewUeContext));
                                     

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

  std::cout << "------------------------------------------------"<< std::endl;
  std::cout << "------------------------------------------------"<< std::endl;
  std::cout << "Total handovers: " << numOfHAndoverInit;
  std::cout << "\n Initiated: " << numOfHAndoverInit;
  std::cout << "\n Successful: " << numOfHAndoverSucceess;
  std::cout << "\n Failed: " << numOfHAndoverInit - numOfHAndoverSucceess;
  std::cout << "\n Failed Retransmissions: " << numOfHAndoverFail - numOfHAndoverSucceess + numOfHAndoverInit;
  std::cout << "\n Ping Pong: " << numOfHAndoverPingPong << std::endl;
  std::cout << "RLF: " << numOfTooEarlyHO + numOfTooLateHO;
  std::cout << "\n Too Early: " << numOfTooEarlyHO;
  std::cout << "\n Too Late: " << numOfTooLateHO;
  std::cout << "\nHPI: " << calculateHPI().HPI << std::endl;
  std::cout << "------------------------------------------------"<< std::endl;
  std::cout << "------------------------------------------------"<< std::endl;

	Simulator::Destroy();

	return 0;
}

