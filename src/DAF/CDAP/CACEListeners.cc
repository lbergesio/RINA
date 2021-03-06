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

#include "CACEListeners.h"


CACEListeners::CACEListeners(CACE* ncace): cace(ncace)
{
}

CACEListeners::~CACEListeners() {
    cace = NULL;
}

void LisCACEConReq::receiveSignal(cComponent *src, simsignal_t id,  cObject *obj){
    CDAPMessage* msg = dynamic_cast<CDAPMessage*>(obj);
    if (msg)
        cace->processMConnect(msg);
    else
        EV << "Received not a CDAPMessage!" << endl;
}

void LisCACERelReq::receiveSignal(cComponent *src, simsignal_t id,  cObject *obj){
    CDAPMessage* msg = dynamic_cast<CDAPMessage*>(obj);
    if (msg)
        cace->processMRelease(msg);
    else
        EV << "Received not a CDAPMessage!" << endl;

}

void LisCACEAuthRes::receiveSignal(cComponent *src, simsignal_t id,  cObject *obj){
    CDAPMessage* msg = dynamic_cast<CDAPMessage*>(obj);

    if (msg)
        cace->treatAuthRes(msg);
    else
        EV << "Received not a CDAPMessage!" << endl;



}


