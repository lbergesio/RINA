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

#ifndef DLMaxQ_H_
#define DLMaxQ_H_

#include "RMTMaxQBase.h"
#include "DLMonitor.h"

class DLMaxQ : public RMTMaxQBase
{
  public:
    bool run(RMTQueue* queue);
    void onPolicyInit();
  protected:
    DLMonitor * monitor;
};

#endif /* ECNMARKER_H_ */
