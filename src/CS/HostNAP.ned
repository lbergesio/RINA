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

// Author: Kewin Rausch <kewin.rausch@create-net.org>

package rina.CS;

import rina.Common.Test;
import rina.DAF.ApplicationProcess;
import rina.DAF.DA.DIFAllocator;
import rina.DAF.IRM.IPCResourceManager;
import rina.DIF.IPCProcess;

module HostNAP
{
    parameters:
        @display("i=device/pc;bgb=257,442");
        @node;

        int nofAP = default(1);

    gates:
        inout medium @loose;

    submodules:
        AP[nofAP]: ApplicationProcess {
            @display("p=64,68");
        }

        difAllocator: DIFAllocator {
            @display("p=195,68");
        }

        ipcProcess0: IPCProcess {
            @display("p=117,358");
        }

        ipcProcess1: IPCProcess {
            @display("p=117,276");
        }

        ipcResourceManager: IPCResourceManager {
            @display("p=117,180");
        }
    connections allowunconnected:
        ipcProcess0.southIo++ <--> medium;
        ipcProcess0.northIo++ <--> ipcProcess1.southIo++;
}
