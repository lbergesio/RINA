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

#include "AESender.h"

Define_Module(AESender);

PingMsg::PingMsg(){
    pingAt = simTime();
}
PongMsg::PongMsg(simtime_t _pingAt){
    pingAt = _pingAt;
    pongAt = simTime();
}


AESender::AESender() : AE() {
    S_TIM_START           = "StartCommunication";
    S_TIM_COM             = "MakeCommunication";
    S_TIM_STOP            = "StopCommunication";

    S_PAR_START           = "startAt";
    S_PAR_STOP            = "stopAt";
    S_PAR_SEND            = "sendAfter";

    S_PAR_DSTAPNAME       = "dstApName";
    S_PAR_DSTAPINSTANCE   = "dstApInstance";
    S_PAR_DSTAENAME       = "dstAeName";
    S_PAR_DSTAEINSTANCE   = "dstAeInstance";

    S_PAR_RATE            = "rate";
    S_PAR_RATE_VAR        = "ratevar";
    S_PAR_SIZE            = "size";
    S_PAR_SIZE_VAR        = "sizevar";

    S_MSG_PING            = "PING-";
    S_PAR_PING            = "pingAt";
    S_VAL_MODULEPATH      = "getFullPath()";
}

AESender::~AESender() {
    connectionState = NIL;
    FlowObject = NULL;
    Irm = NULL;
    Cdap = NULL;
}

void AESender::prepareAllocateRequest() {
    //Schedule AllocateRequest
    cMessage* m1 = new cMessage(S_TIM_START);
    scheduleAt(startAt, m1);
}


void AESender::prepareDeallocateRequest() {
    //Schedule DeallocateRequest
    cMessage* m3 = new cMessage(S_TIM_STOP);
    scheduleAt(stopAt, m3);
}

void AESender::initialize()
{
    //Init pointers
    initPointers();
    //Source info
    initNamingInfo();
    //Setup signals
    initSignalsAndListeners();
    //Init QoSRequirements
    initQoSRequiremets();

    //Timers
    startAt = simTime() + par(S_PAR_START);
    stopAt  = simTime() + par(S_PAR_STOP);
    sendAfter  = par(S_PAR_SEND);
    if(sendAfter<1){
        sendAfter = 1;
    }

    //Time between PDUS (s)
    rate    = par(S_PAR_RATE).doubleValue();
    ratevar    = par(S_PAR_RATE_VAR).doubleValue();

    //SIZE of PDU
    size    = par(S_PAR_SIZE);
    sizevar    = par(S_PAR_SIZE_VAR);

    //Destination for flow
    dstApName     = this->par(S_PAR_DSTAPNAME).stringValue();
    dstApInstance = this->par(S_PAR_DSTAPINSTANCE).stringValue();
    dstAeName     = this->par(S_PAR_DSTAENAME).stringValue();
    dstAeInstance = this->par(S_PAR_DSTAEINSTANCE).stringValue();

    //Schedule AllocateRequest
    if (startAt > 0)
        prepareAllocateRequest();
    //Schedule DeallocateRequest
    /*
    if (stopAt > 0)
        prepareDeallocateRequest();
    */
    myPath = this->getFullPath();

    send = 0;
    received = 0;
    pingreceived = 0;
    sendSize = 0;
    receivedSize = 0;
    pingreceivedSize = 0;

    minDelay = 999;
    maxDelay = -1;
    firstR = -1;
    lastR = 0;

    //Watchers
    WATCH(FlowObject);
    WATCH(send);
    WATCH(received);
    WATCH(pingreceived);
    WATCH(sendSize);
    WATCH(receivedSize);
    WATCH(pingreceivedSize);
    WATCH(minDelay);
    WATCH(maxDelay);
}

void AESender::finish()
{
    if(par("printAtEnd").boolValue()){
        EV << "At "<<this->getApni()<<endl;
        if(FlowObject != NULL) {
            EV << "With QoS " << FlowObject->getConId().getQoSId() <<endl;
        }
        EV << "send " << send << " ("<<sendSize << ")"<<endl;
        EV << "pingsRcv "  << pingreceived << " ("<<pingreceivedSize << ")"<<endl;
        EV << "pongsRcv "  << received << " ("<<receivedSize << ")"<<endl;
        EV << "delay "  << minDelay << " / "<<maxDelay<<endl;
        EV << "timestamps "  << firstR << " -> "<<lastR<<endl;
        EV << "-----------------"<<endl;
    }
}

void AESender::handleSelfMessage(cMessage *msg) {
    //EV << flows.back().info() << endl;
    if ( !strcmp(msg->getName(), S_TIM_START) ) {
        //FIXME: Vesely - last flow in a list?!

        //Flow
        APNamingInfo src = this->getApni();
        APNamingInfo dst = APNamingInfo( APN(this->dstApName), this->dstApInstance,
                                         this->dstAeName, this->dstAeInstance);

        FlowObject = new Flow(src, dst);
        FlowObject->setQosParameters(this->getQoSRequirements());

        //Insert it to the Flows ADT
        insertFlow();

        sendAllocationRequest(FlowObject);

        //Schedule ComRequest
        cMessage* m = new cMessage(S_TIM_COM);
        scheduleAt(simTime()+sendAfter, m);
    }
    else if ( !strcmp(msg->getName(), S_TIM_STOP) ) {
        sendDeallocationRequest(FlowObject);
    }
    else if ( !strcmp(msg->getName(), S_TIM_COM) ) {
        if(stopAt > simTime()){
            int msgSize = size + intuniform(-sizevar,sizevar);
            double msgWait = rate + uniform(-ratevar,ratevar);

            //Create PING messsage
            CDAP_M_Read* ping = new PingMsg();

            ping->setByteLength(msgSize);

            //Send message
            sendData(FlowObject, ping);
            send++;
            sendSize += msgSize;

            //Schedule ComRequest
            cMessage* m = new cMessage(S_TIM_COM);
            scheduleAt(simTime()+msgWait, m);
        }
    }
    else
        EV << this->getFullPath() << " received unknown self-message " << msg->getName();
    delete(msg);
}

void AESender::handleMessage(cMessage *msg)
{
    if ( msg->isSelfMessage() )
            this->handleSelfMessage(msg);
}

void AESender::processMRead(CDAPMessage* msg) {
    PingMsg* ping = check_and_cast<PingMsg*>(msg);
    if(ping){
        PongMsg* pong = new PongMsg(ping->pingAt);
        pong->setByteLength(msg->getByteLength());

        sendData(FlowObject, pong);

        pingreceived++;
        pingreceivedSize += msg->getByteLength();
        simtime_t delay = simTime() - ping->pingAt;
        if(minDelay>delay){
            minDelay = delay;
        }
        if(maxDelay<delay){
            maxDelay = delay;
        }
        if(firstR<0)  {
            firstR = simTime();
        }
        lastR = simTime();
    }
}

void AESender::processMReadR(CDAPMessage* msg) {
    PongMsg* pong = check_and_cast<PongMsg*>(msg);
    if(pong){
        received++;
        receivedSize += msg->getByteLength();
        simtime_t delay = simTime() - pong->pingAt;
        if(minDelay>delay){
            minDelay = delay;
        }
        if(maxDelay<delay){
            maxDelay = delay;
        }
        if(firstR<0)  {
            firstR = simTime();
        }
        lastR = simTime();
    }
}
