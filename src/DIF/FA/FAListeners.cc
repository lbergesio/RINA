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

#include "FAListeners.h"

void LisFAAllocReq::receiveSignal(cComponent* src, simsignal_t id, cObject* obj) {
    EV << "AllocateRequest initiated by " << src->getFullPath() << " and processed by " << fa->getFullPath() << endl;
    fa->receiveAllocateRequest(obj);
    return;
}

void LisFADeallocReq::receiveSignal(cComponent* src, simsignal_t id,
        cObject* obj) {
    EV << "DeallocateRequest initiated by " << src->getFullPath() << " and processed by " << fa->getFullPath() << endl;
    fa->receiveDeallocateRequest(obj);
    return;
}

/**
 *
 * @param src
 * @param id
 * @param obj
 */
void LisFACreReq::receiveSignal(cComponent* src, simsignal_t id, cObject* obj) {
    EV << "CreateRequest initiated by " << src->getFullPath() << " and processed by " << fa->getFullPath() << endl;
    Flow* flow = dynamic_cast<Flow*>(obj);
    if (flow)
        fa->receiveCreateFlowRequest(flow);
    else
        EV << "Received not a flow object!" << endl;
    return;
}

void LisFAAllocResPosi::receiveSignal(cComponent* src, simsignal_t id,
        cObject* obj) {
    EV << "AllocateResponsePositive initiated by " << src->getFullPath() << " and processed by " << fa->getFullPath() << endl;
    Flow* flow = dynamic_cast<Flow*>(obj);
    if (flow) {
        //Pass request to FA that has this Flow in its Flow table
        if (fa->getFaiTable()->findEntryByFlow(flow))
            fa->receiveAllocateResponsePositive(flow);
        else
            EV << "Flow not in FaiTable" << endl;
    }
    else
        EV << "Received not a flow object!" << endl;
}

void LisFAAllocResNega::receiveSignal(cComponent* src, simsignal_t id,
        cObject* obj) {
    EV << "AllocateResponseNegative initiated by " << src->getFullPath() << " and processed by " << fa->getFullPath() << endl;
    Flow* flow = dynamic_cast<Flow*>(obj);
    if (flow) {
        //Pass request to FA that has this Flow in its Flow table
        if (fa->getFaiTable()->findEntryByFlow(flow))
            fa->receiveAllocateResponseNegative(flow);
        else
            EV << "Flow not in FaiTable" << endl;
    }
    else
        EV << "Received not a flow object!" << endl;
}
