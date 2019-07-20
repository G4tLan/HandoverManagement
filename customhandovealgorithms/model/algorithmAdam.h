/*
 * This algorithm is developed based on the work by:
 * Rami Ahmad et al. from the paper:
 * Efficient Handover in LTE-A by Using Mobility 
 * Pattern and User Trajectory Prediction
 * 
 */

#ifndef SCRATCH_ALGORITHM_ADAM_H
#define SCRATCH_ALGORITHM_ADAM_H

#include <ns3/lte-handover-algorithm.h>
#include <ns3/lte-handover-management-sap.h>
#include <ns3/lte-rrc-sap.h>
#include <ns3/simple-ref-count.h>
#include <ns3/ptr.h>
#include <map>

namespace ns3
{
class algorithmAdam : public LteHandoverAlgorithm
{
public:
    algorithmAdam();
    virtual ~algorithmAdam();

    static TypeId GetTypeId();

    //inherited from LTEhandoverAlgorithm
    virtual void SetLteHandoverManagementSapUser(LteHandoverManagementSapUser *s);
    virtual LteHandoverManagementSapProvider *GetLteHandoverManagementSapProvider();

    friend class MemberLteHandoverManagementSapProvider<algorithmAdam>;

protected:
    //inherited from object
    virtual void DoInitialize();
    virtual void DoDispose();

    // inherited from LteHandoverAlgorithm as a Handover Management SAP implementation
    void DoReportUeMeas(uint16_t rnti, LteRrcSap::MeasResults measResults);

private:
    LteHandoverManagementSapUser *m_handoverManagementSapUser;
    LteHandoverManagementSapProvider *m_handoverManagementSapProvider;
};
} // namespace ns3

#endif