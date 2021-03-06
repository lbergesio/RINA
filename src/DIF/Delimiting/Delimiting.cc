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
 * @file Delimiting.cc
 * @author Marcel Marek (imarek@fit.vutbr.cz)
 * @date Jul 31, 2014
 * @brief
 * @detail
 */

#include "Delimiting.h"

Define_Module(Delimiting);

Delimiting::Delimiting()
{
 seqNum = 1;
}

void Delimiting::initialize(int step){


}

void Delimiting::initGates(){
  northI = this->gateHalf(GATE_DELIMIT_NORTHIO, cGate::INPUT);
  northO = this->gateHalf(GATE_DELIMIT_NORTHIO, cGate::OUTPUT);

  southI = this->gateHalf(GATE_DELIMIT_SOUTHIO, cGate::INPUT, 0);
  southO = this->gateHalf(GATE_DELIMIT_SOUTHIO, cGate::OUTPUT, 0);
}


void Delimiting::handleMessage(cMessage* msg){
  if(msg->isSelfMessage()){
    //self-message
  }else{
    if(msg->arrivedOn(northI->getId())){
      processMsgFromFAI((cPacket*)(msg));
    }else if(msg->arrivedOn(southI->getId())){
      handleMsgFromEfcpi((Data*)(msg));
    }else{
      //A2 panic!
    }


  }


}

void Delimiting::processMsgFromFAI(cPacket* msg){

  /*
   * 1. Create new SDU and put msg to this new SDU.
   * 2. Check if SDU.size < (MAXSDUSIZE - header)
   *  2a partition it creating multiple SDUFrag and put them into some vector
   * 3. Go through Data vector and send them to EFCPI
   */


  SDU* sdu = new SDU();
  sdu->setDataType(SDU_COMPLETE_TYPE);
  sdu->addUserData(msg);
  sdu->setSeqNum(seqNum);

  //TODO A1 handle multiple gates -> change to cGate*
  send(sdu, southO);
}

void Delimiting::handleMsgFromEfcpi(Data* msg){

  SDU* sdu = (SDU*) msg;
//  std::vector<CDAPMessage*> &msgVector = sdu->getMUserData();
  cPacket* cdap = sdu->getUserData();
  take(cdap);

  send(cdap, northO);
  delete sdu;
}

Delimiting::~Delimiting()
{

}

