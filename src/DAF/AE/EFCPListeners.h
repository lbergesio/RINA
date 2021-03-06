//
// Copyright © 2014-2015 PRISTINE Consortium (http://ict-pristine.eu)
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
/*
 * @file EFCPListeners.h
 * @author Marcel Marek
 * @date Feb 12, 2015
 * @brief
 * @detail

 */

#ifndef EFCPLISTENERS_H_
#define EFCPLISTENERS_H_


#include <omnetpp.h>

//#include "CongestionDescriptor.h"
#include "EFCPTable.h"

class EFCPListeners : public cListener
{
  public:
    EFCPListeners(EFCPTable* efcpTable);
    virtual ~EFCPListeners();

  protected:
    EFCPTable* efcpTable;
};


class LisEFCPCongestFromRA : public EFCPListeners {
  public:
    LisEFCPCongestFromRA(EFCPTable* efcpTable): EFCPListeners(efcpTable){};
    void virtual receiveSignal(cComponent* src, simsignal_t id, cObject* obj);
};

#endif /* EFCPLISTENERS_H_ */
