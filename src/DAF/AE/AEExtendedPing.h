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

#ifndef __RINA_AEEXTENDEDPING_H_
#define __RINA_AEEXTENDEDPING_H_

//Standard libraries
#include <omnetpp.h>
//RINASim libraries
#include "AE.h"


class AEExtendedPing : public AE
{
  public:
    AEExtendedPing();
    virtual ~AEExtendedPing();

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    void handleSelfMessage(cMessage* msg);

  private:
    std::string myPath;

    std::string dstApName;
    std::string dstApInstance;
    std::string dstAeName;
    std::string dstAeInstance;

    simtime_t startAt;
    simtime_t stopAt;
    simtime_t connectAt;
    simtime_t pingAt;
    int rate;
    unsigned int size;

    void prepareAllocateRequest();
    void preparePing();
    void prepareConnectionRequest();
    void prepareDeallocateRequest();

    virtual void processMRead(CDAPMessage* msg);
    virtual void processMReadR(CDAPMessage* msg);

};

#endif

