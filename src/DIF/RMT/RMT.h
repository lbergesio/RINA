//
// Copyright © 2014 - 2015 PRISTINE Consortium (http://ict-pristine.eu)
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
 * @file RMT.h
 * @author Tomas Hykel (xhykel01@stud.fit.vutbr.cz)
 * @brief Relaying and Multiplexing Task
 * @detail
 */

#ifndef RMT_H_
#define RMT_H_

#include <omnetpp.h>

#include "RINASignals.h"
#include "ExternConsts.h"
#include "Address.h"
#include "PDU.h"
#include "CDAPMessage_m.h"

#include "IntPDUForwarding.h"
#include "QueueAllocBase.h"
#include "AddressComparatorBase.h"

#include "RMTBase.h"
#include "RMTListeners.h"
#include "RMTModuleAllocator.h"
#include "RMTSchedulingBase.h"
#include "RMTQMonitorBase.h"
#include "RMTMaxQBase.h"

typedef std::map<int, cGate*> EfcpiMapping;

class RMT : public RMTBase
{
  friend class RA;
  friend class FAI;

  public:

    virtual ~RMT();

    virtual bool getRelayStatus() { return relayOn; };
    virtual bool isOnWire() { return onWire; };

    virtual void invokeQueueArrivalPolicies(cObject* obj);
    virtual void invokeQueueDeparturePolicies(cObject* obj);
    virtual void writeToPort(cObject* obj);
    virtual void readFromPort(cObject* obj);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  private:
    IntPDUForwarding* fwd;
    RMTModuleAllocator* rmtAllocator;

    int erroneousCount;

    bool relayOn;
    bool onWire;

    EfcpiMapping efcpiOut;
    EfcpiMapping efcpiIn;

    RMTQMonitorBase* qMonPolicy;
    RMTMaxQBase* maxQPolicy;
    RMTSchedulingBase* schedPolicy;

    QueueAllocBase* qAllocPolicy;
    QueueIDGenBase* queueIdGenerator;
    AddressComparatorBase* addrComparator;

    void processMessage(cMessage* msg);
    void efcpiToPort(PDU* msg);
    void efcpiToEfcpi(PDU* msg);
    void portToEfcpi(PDU* msg);
    void ribToPort(CDAPMessage* msg);
    void portToRIB(CDAPMessage* msg);
    void portToPort(cMessage* msg);

    RMTPort* fwTableLookup(const Address& destAddr, const unsigned short &pduQosId);
    RMTPort* fwTableLookup(const PDU * pdu);
    std::deque<cMessage*> invalidPDUs;

    simsignal_t sigRMTNoConnID;
    simsignal_t sigRMTPacketError;
    simsignal_t sigStatRMTPacketErrorCount;

    LisRMTQueuePDURcvd* lisRMTQueuePDURcvd;
    LisRMTQueuePDUSent* lisRMTQueuePDUSent;
    LisRMTPortReadyToServe* lisRMTPortReadyToServe;
    LisRMTPortReadyForRead* lisRMTPortReadyForRead;

    // management methods for Resource Allocator
    void setOnWire(bool status) { onWire = status; };
    void enableRelay() { relayOn = true; };
    void disableRelay() { relayOn = false; };

    // management methods for FAIs
    void createEfcpiGate(unsigned int efcpiId);
    void deleteEfcpiGate(unsigned int efcpiId);
};

#endif /* RMT_H_ */
