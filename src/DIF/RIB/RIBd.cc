//
// Copyright © 2014 PRISTINE Consortium (http://ict-pristine.eu)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
/**
 * @file RIBd.cc
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @date Apr 30, 2014
 * @brief Kind of a Notification Board for DIF
 * @detail
 */

#include "RIBd.h"

const char* MSG_CONGEST         = "Congestion";
const char* MSG_FLO             = "Flow";
const char* MSG_FLOPOSI         = "Flow+";
const char* MSG_FLONEGA         = "Flow-";
const char* CLS_FLOW            = "Flow";
const int   VAL_DEFINSTANCE     = -1;
const int   VAL_FLOWPOSI        = 1;
const int   VAL_FLOWNEGA        = 0;
const char* VAL_FLREQ           = "Request  ";
const char* VAL_FLREQPOSI       = "Response+  ";
const char* VAL_FLREQNEGA       = "Response-  ";
const char* MSG_ROUTINGUPDATE       = "RoutingUpdate";

Define_Module(RIBd);

void RIBd::initialize() {
    //Init signals and listeners
    initSignalsAndListeners();
    //Init MyAddress
    initMyAddress();
}

void RIBd::handleMessage(cMessage *msg) {

}

void RIBd::sendCreateRequestFlow(Flow* flow) {
    Enter_Method("sendCreateRequestFlow()");

    //Prepare M_CREATE Flow
    CDAP_M_Create* mcref = new CDAP_M_Create(MSG_FLO);

    //Prepare object
    std::ostringstream os;
    os << flow->getFlowName();
    object_t flowobj;
    flowobj.objectClass = flow->getClassName();
    flowobj.objectName = os.str();
    flowobj.objectVal = flow;
    //TODO: Vesely - Assign appropriate values
    flowobj.objectInstance = VAL_DEFINSTANCE;
    mcref->setObject(flowobj);

    //Append destination address for RMT "routing"
    mcref->setDstAddr(flow->getDstNeighbor());

    //Generate InvokeId
    if (!flow->getAllocInvokeId())
        flow->setAllocInvokeId(getNewInvokeId());
    mcref->setInvokeID(flow->getAllocInvokeId());

    //Send it
    signalizeSendData(mcref);
}

void RIBd::processMCreate(CDAPMessage* msg) {
    CDAP_M_Create* msg1 = check_and_cast<CDAP_M_Create*>(msg);

    EV << "Received M_Create";
    object_t object = msg1->getObject();
    EV << " with object '" << object.objectClass << "'" << endl;

    //CreateRequest Flow
    if (dynamic_cast<Flow*>(object.objectVal)) {
        Flow* fl = (check_and_cast<Flow*>(object.objectVal))->dup();
        //EV << fl->info();
        fl->swapFlow();
        //EV << "\n===========\n" << fl->info();
        signalizeCreateRequestFlow(fl);
    }
}

void RIBd::sendDeleteRequestFlow(Flow* flow) {
    Enter_Method("sendDeleteRequestFlow()");

    //Prepare M_CREATE Flow
    CDAP_M_Delete* mdereqf = new CDAP_M_Delete(MSG_FLO);

    //Prepare object
    std::ostringstream os;
    os << flow->getFlowName();
    object_t flowobj;
    flowobj.objectClass = flow->getClassName();
    flowobj.objectName = os.str();
    flowobj.objectVal = flow;
    //TODO: Vesely - Assign appropriate values
    flowobj.objectInstance = VAL_DEFINSTANCE;
    mdereqf->setObject(flowobj);

    //Append destination address for RMT "routing"
    mdereqf->setDstAddr(flow->getDstAddr());

    //Generate InvokeId
    if (!flow->getDeallocInvokeId())
        flow->setDeallocInvokeId(getNewInvokeId());
    mdereqf->setInvokeID(flow->getDeallocInvokeId());

    //Send it
    signalizeSendData(mdereqf);
}

void RIBd::processMCreateR(CDAPMessage* msg) {
    CDAP_M_Create_R* msg1 = check_and_cast<CDAP_M_Create_R*>(msg);

    EV << "Received M_Create_R";
    object_t object = msg1->getObject();
    EV << " with object '" << object.objectClass << "'" << endl;

    //CreateResponseFlow
    if (dynamic_cast<Flow*>(object.objectVal)) {
        Flow* flow = (check_and_cast<Flow*>(object.objectVal))->dup();
        flow->swapFlow();
        //Positive response
        if (!msg1->getResult().resultValue) {
            signalizeCreateResponseFlowPositive(flow);
        }
        //Negative response
        else
            signalizeCreateResponseFlowNegative(flow);
    }
}

void RIBd::receiveData(CDAPMessage* msg) {
    Enter_Method("receiveData()");
    //M_CREATE_Request
    if (dynamic_cast<CDAP_M_Create*>(msg)) {
        processMCreate(msg);
    }
    //M_CREATE_Response
    else if (dynamic_cast<CDAP_M_Create_R*>(msg)) {
        processMCreateR(msg);
    }
    //M_DELETE_Request
    else if (dynamic_cast<CDAP_M_Delete*>(msg)) {
        processMDelete(msg);
    }
    //M_DELETE_Request
    else if (dynamic_cast<CDAP_M_Delete_R*>(msg)) {
        processMDeleteR(msg);
    }
    //M_WRITE_Request
    else if (dynamic_cast<CDAP_M_Write*>(msg)) {
        processMWrite(msg);
    }
    //M_START
    else if (dynamic_cast<CDAP_M_Start*>(msg)) {
        processMStart(msg);
    }

    delete msg;
}

void RIBd::initSignalsAndListeners() {
    cModule* catcher1 = this->getParentModule();
    cModule* catcher2 = this->getParentModule()->getParentModule();
    cModule* catcher3 = this->getParentModule()->getParentModule()->getParentModule();

    //Signals that this module is emitting
    sigRIBDSendData      = registerSignal(SIG_RIBD_DataSend);
    sigRIBDCreReqFlo     = registerSignal(SIG_RIBD_CreateRequestFlow);
    sigRIBDDelReqFlo     = registerSignal(SIG_RIBD_DeleteRequestFlow);
    sigRIBDDelResFlo     = registerSignal(SIG_RIBD_DeleteResponseFlow);
    sigRIBDAllocResPosi  = registerSignal(SIG_AERIBD_AllocateResponsePositive);
    sigRIBDAllocResNega  = registerSignal(SIG_AERIBD_AllocateResponseNegative);
    sigRIBDCreFlow       = registerSignal(SIG_RIBD_CreateFlow);
    sigRIBDCreResFloPosi = registerSignal(SIG_RIBD_CreateFlowResponsePositive);
    sigRIBDCreResFloNega = registerSignal(SIG_RIBD_CreateFlowResponseNegative);
   // sigRIBDFwdUpdateRecv = registerSignal(SIG_RIBD_ForwardingUpdateReceived);
    sigRIBDRoutingUpdateRecv = registerSignal(SIG_RIBD_RoutingUpdateReceived);
    sigRIBDCongNotif     = registerSignal(SIG_RIBD_CongestionNotification);

    //Signals that this module is processing

    lisRIBDCreReqByForward = new LisRIBDCreReq(this);
    catcher2->subscribe(SIG_FA_CreateFlowRequestForward, lisRIBDCreReqByForward);
    lisRIBDCreReq = new LisRIBDCreReq(this);
    catcher2->subscribe(SIG_FAI_CreateFlowRequest, lisRIBDCreReq);

    lisRIBDDelReq = new LisRIBDDelReq(this);
    catcher2->subscribe(SIG_FAI_DeleteFlowRequest, lisRIBDDelReq);
    lisRIBDDelRes = new LisRIBDDelRes(this);
    catcher2->subscribe(SIG_FAI_DeleteFlowResponse, lisRIBDDelRes);

    lisRIBDCreResNegaFromFa = new LisRIBDCreResNega(this);
    catcher2->subscribe(SIG_FA_CreateFlowResponseNegative, lisRIBDCreResNegaFromFa);
    lisRIBDCreResNega = new LisRIBDCreResNega(this);
    catcher2->subscribe(SIG_FAI_CreateFlowResponseNegative, lisRIBDCreResNega);

    lisRIBDCreResPosi = new LisRIBDCreResPosi(this);
    catcher2->subscribe(SIG_FAI_CreateFlowResponsePositive, lisRIBDCreResPosi);
    lisRIBDCreResPosiForward = new LisRIBDCreResPosi(this);
    catcher2->subscribe(SIG_FA_CreateFlowResponseForward, lisRIBDCreResPosiForward);

    lisRIBDRcvData = new LisRIBDRcvData(this);
    catcher1->subscribe(SIG_CDAP_DateReceive, lisRIBDRcvData);

    lisRIBDAllReqFromFai = new LisRIBDAllReqFromFai(this);
    catcher3->subscribe(SIG_FAI_AllocateRequest, lisRIBDAllReqFromFai);

    lisRIBDCreFloPosi = new LisRIBDCreFloPosi(this);
    catcher2->subscribe(SIG_RA_CreateFlowPositive, lisRIBDCreFloPosi);
    lisRIBDCreFloNega = new LisRIBDCreFloNega(this);
    catcher2->subscribe(SIG_RA_CreateFlowNegative, lisRIBDCreFloNega);

    lisRIBDRoutingUpdate = new LisRIBDRoutingUpdate(this);
    catcher2->subscribe(SIG_RIBD_RoutingUpdate, lisRIBDRoutingUpdate);

    lisRIBDCongNotif = new LisRIBDCongesNotif(this);
    catcher2->subscribe(SIG_RA_InvokeSlowdown, lisRIBDCongNotif);

}

void RIBd::receiveAllocationRequestFromFai(Flow* flow) {
    Enter_Method("receiveAllocationRequestFromFai()");
    //Execute flow allocate
    signalizeCreateFlow(flow);
}

void RIBd::sendCreateResponseNegative(Flow* flow) {
    Enter_Method("sendCreateResponseFlowNegative()");

    //Prepare M_CREATE_R Flow-
    CDAP_M_Create_R* mcref = new CDAP_M_Create_R(MSG_FLONEGA);

    //Prepare object
    std::ostringstream os;
    os << flow->getFlowName();
    object_t flowobj;
    flowobj.objectClass = flow->getClassName();
    flowobj.objectName = os.str();
    flowobj.objectVal = flow;
    //TODO: Vesely - Assign appropriate values
    flowobj.objectInstance = VAL_DEFINSTANCE;
    mcref->setObject(flowobj);

    //Prepare result object
    result_t resultobj;
    resultobj.resultValue = R_FAIL;
    mcref->setResult(resultobj);

    //TODO: Vesely - Work more on InvokeId

    //Append destination address for RMT "routing"
    mcref->setDstAddr(flow->getDstAddr());

    //Send it
    signalizeSendData(mcref);
}

void RIBd::sendCreateResponsePostive(Flow* flow) {
    Enter_Method("sendCreateResponseFlowPositive()");

    //Prepare M_CREATE_R Flow+
    CDAP_M_Create_R* mcref = new CDAP_M_Create_R(MSG_FLOPOSI);

    //Prepare object
    std::ostringstream os;
    os << flow->getFlowName();
    object_t flowobj;
    flowobj.objectClass = flow->getClassName();
    flowobj.objectName = os.str();
    flowobj.objectVal = flow;
    //TODO: Vesely - Assign appropriate values
    flowobj.objectInstance = VAL_DEFINSTANCE;
    mcref->setObject(flowobj);

    //Prepare result object
    result_t resultobj;
    resultobj.resultValue = R_SUCCESS;
    mcref->setResult(resultobj);

    //Generate InvokeId
    mcref->setInvokeID(flow->getAllocInvokeId());

    //Append destination address for RMT "routing"
    mcref->setDstAddr(flow->getDstAddr());

    //Send it
    signalizeSendData(mcref);
}

void RIBd::signalizeSendData(CDAPMessage* msg) {
    //Check dstAddress
    if (msg->getDstAddr() == Address::UNSPECIFIED_ADDRESS) {
        EV << "Destination address cannot be UNSPECIFIED!" << endl;
        return;
    }

    msg->setBitLength(msg->getBitLength() + msg->getHeaderBitLength());
    //Pass message to CDAP
    EV << "Emits SendData signal for message " << msg->getName() << endl;
    emit(sigRIBDSendData, msg);
}

void RIBd::signalizeAllocateResponsePositive(Flow* flow) {
    EV << "Emits AllocateResponsePositive signal for flow" << endl;
    emit(sigRIBDAllocResPosi, flow);
}

void RIBd::signalizeCreateRequestFlow(Flow* flow) {
    EV << "Emits CreateRequestFlow signal for flow" << endl;
    emit(sigRIBDCreReqFlo, flow);
}

void RIBd::signalizeCreateResponseFlowPositive(Flow* flow) {
    EV << "Emits CreateResponsetFlowPositive signal for flow" << endl;
    emit(sigRIBDCreResFloPosi, flow);
}

void RIBd::signalizeCreateResponseFlowNegative(Flow* flow) {
    EV << "Emits CreateResponsetFlowNegative signal for flow" << endl;
    emit(sigRIBDCreResFloNega, flow);
}

void RIBd::signalizeCreateFlow(Flow* flow) {
    EV << "Emits CreateFlow signal for flow" << endl;
    emit(sigRIBDCreFlow, flow);
}

void RIBd::processMDelete(CDAPMessage* msg) {
    CDAP_M_Delete* msg1 = check_and_cast<CDAP_M_Delete*>(msg);

    EV << "Received M_Delete";
    object_t object = msg1->getObject();
    EV << " with object '" << object.objectClass << "'" << endl;

    //DeleteRequest Flow
    if (dynamic_cast<Flow*>(object.objectVal)) {
        Flow* fl = (check_and_cast<Flow*>(object.objectVal))->dup();
        fl->swapFlow();
        signalizeDeleteRequestFlow(fl);
    }

}

void RIBd::signalizeDeleteRequestFlow(Flow* flow) {
    EV << "Emits DeleteRequestFlow signal for flow" << endl;
    emit(sigRIBDDelReqFlo, flow);
}

void RIBd::sendDeleteResponseFlow(Flow* flow) {
    Enter_Method("sendDeleteResponseFlow()");

    //Prepare M_CREATE_R Flow+
    CDAP_M_Delete_R* mderesf = new CDAP_M_Delete_R(MSG_FLOPOSI);

    //Prepare object
    std::ostringstream os;
    os << flow->getFlowName();
    object_t flowobj;
    flowobj.objectClass = flow->getClassName();
    flowobj.objectName = os.str();
    flowobj.objectVal = flow;
    //TODO: Vesely - Assign appropriate values
    flowobj.objectInstance = VAL_DEFINSTANCE;
    mderesf->setObject(flowobj);

    //Prepare result object
    result_t resultobj;
    resultobj.resultValue = R_SUCCESS;
    mderesf->setResult(resultobj);

    //Generate InvokeId
    mderesf->setInvokeID(flow->getDeallocInvokeId());

    //Append destination address for RMT "routing"
    mderesf->setDstAddr(flow->getDstAddr());

    //Send it
    signalizeSendData(mderesf);

}

void RIBd::signalizeDeleteResponseFlow(Flow* flow) {
    EV << "Emits DeleteResponseFlow signal for flow" << endl;
    emit(sigRIBDDelResFlo, flow);
}

void RIBd::receiveCreateFlowPositiveFromRa(Flow* flow) {
    signalizeAllocateResponsePositive(flow);
}

void RIBd::receiveCreateFlowNegativeFromRa(Flow* flow) {
    signalizeAllocateResponseNegative(flow);
}

void RIBd::signalizeAllocateResponseNegative(Flow* flow) {
    EV << "Emits AllocateResponseNegative signal for flow" << endl;
    emit(sigRIBDAllocResNega, flow);
}

void RIBd::processMDeleteR(CDAPMessage* msg) {
    CDAP_M_Delete_R* msg1 = check_and_cast<CDAP_M_Delete_R*>(msg);

    EV << "Received M_Delete_R";
    object_t object = msg1->getObject();
    EV << " with object '" << object.objectClass << "'" << endl;

    //DeleteResponseFlow
    if (dynamic_cast<Flow*>(object.objectVal)) {
        Flow* flow = (check_and_cast<Flow*>(object.objectVal))->dup();
        flow->swapFlow();
        signalizeDeleteResponseFlow(flow);
    }

}

void RIBd::sendCongestionNotification(PDU* pdu) {
    Enter_Method("sendCongestionNotification()");

    //Prepare M_START ConDescr
    CDAP_M_Start* mstarcon = new CDAP_M_Start(MSG_CONGEST);
    CongestionDescriptor* conDesc = new CongestionDescriptor(pdu->getConnId().getDstCepId(), pdu->getConnId().getSrcCepId(), pdu->getConnId().getQoSId());
    //Prepare object
    std::ostringstream os;
    os << conDesc->getCongesDescrName();
    object_t condesobj;
    condesobj.objectClass = conDesc->getClassName();
    condesobj.objectName = os.str();
    condesobj.objectVal = conDesc;
    //TODO: Vesely - Assign appropriate values
    condesobj.objectInstance = VAL_DEFINSTANCE;
    mstarcon->setObject(condesobj);

    //Generate InvokeId
    mstarcon->setInvokeID(DONTCARE_INVOKEID);

    //Append destination address for RMT "routing"
    mstarcon->setDstAddr(pdu->getSrcAddr());

    //Send it
    signalizeSendData(mstarcon);
}

void RIBd::processMWrite(CDAPMessage* msg)
{
    CDAP_M_Write * msg1 = check_and_cast<CDAP_M_Write *>(msg);

    EV << "Received M_Write";
    object_t object = msg1->getObject();
    EV << " with object '" << object.objectClass << "'" << endl;

    //CreateRequest Flow
    if (dynamic_cast<IntRoutingUpdate *>(object.objectVal))
    {
        IntRoutingUpdate * update = (check_and_cast<IntRoutingUpdate *>(object.objectVal));

        /* Signal that an update obj has been received. */
        emit(sigRIBDRoutingUpdateRecv, update);
    }
}

void RIBd::receiveRoutingUpdateFromRouting(IntRoutingUpdate * info)
{
    EV << getFullPath() << " Forwarding update to send to " << info->getDestination();

    /* Emits the CDAP message. */

    CDAP_M_Write * cdapm = new CDAP_M_Write(MSG_ROUTINGUPDATE);
    std::ostringstream os;
    object_t flowobj;

    /* Prepare the object to send. */

    os << "RoutingUpdateTo" << info->getDestination();

    flowobj.objectClass = info->getClassName();
    flowobj.objectName  = os.str();
    flowobj.objectVal   = info;
    //TODO: Vesely - Assign appropriate values
    flowobj.objectInstance = VAL_DEFINSTANCE;

    cdapm->setObject(flowobj);

    //TODO: Vesely - Work more on InvokeId

    /* This message will be sent to... */
    cdapm->setDstAddr(info->getDestination());

    /* Finally order to send the data... */
    signalizeSendData(cdapm);
}

void RIBd::signalizeCongestionNotification(CongestionDescriptor* congDesc) {
    EV << "Emits CongestionNotification" << endl;
    emit(sigRIBDCongNotif, congDesc);
}

void RIBd::processMStart(CDAPMessage* msg) {
    CDAP_M_Start* msg1 = check_and_cast<CDAP_M_Start*>(msg);

    EV << "Received M_Start";
    object_t object = msg1->getObject();
    EV << " with object '" << object.objectClass << "'" << endl;

    //CongestionNotification CongestDescr
    if (dynamic_cast<CongestionDescriptor*>(object.objectVal)) {
        CongestionDescriptor* congdesc = (check_and_cast<CongestionDescriptor*>(object.objectVal))->dup();
        congdesc->getConnectionId().swapCepIds();
        EV << "\n===========\n" << congdesc->getConnectionId().info();
        signalizeCongestionNotification(congdesc);
    }

}
