#include "algorithmAdam.h"
#include <ns3/log.h>

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("algorithmAdam");
NS_OBJECT_ENSURE_REGISTERED(algorithmAdam);

algorithmAdam::algorithmAdam() : m_handoverManagementSapUser(0)
{
    m_handoverManagementSapProvider = new MemberLteHandoverManagementSapProvider<algorithmAdam>(this);
}

algorithmAdam::~algorithmAdam()
{
    std::cout << "destroying Adam algo" << std::endl;
}

TypeId algorithmAdam::GetTypeId()
{
    static TypeId tid = TypeId("ns3::algorithmAdam")
    .SetParent<LteHandoverAlgorithm>()
    .SetGroupName("Lte")
    .AddConstructor<algorithmAdam>();

    return tid;
}

void algorithmAdam::SetLteHandoverManagementSapUser(LteHandoverManagementSapUser* s)
{
    m_handoverManagementSapUser = s;
}

LteHandoverManagementSapProvider* algorithmAdam::GetLteHandoverManagementSapProvider()
{
    return m_handoverManagementSapProvider;
}

void algorithmAdam::DoInitialize()
{
    LteHandoverAlgorithm::DoInitialize();
}

void algorithmAdam::DoDispose() 
{
    delete m_handoverManagementSapProvider;
}

void algorithmAdam::DoReportUeMeas (uint16_t rnti, LteRrcSap::MeasResults measResults)
{
    std::cout << "reporting by: " << rnti << " " << measResults.rsrpResult << std::endl;
}

} // namespace ns3
