#include "bc.hpp"
#include "bc.h"
#include <iostream>

BC::BC() : m_devNum(0x0000) {}

BC::~BC() { aceFree(m_devNum); }

int BC::startBc(S16BIT devNum) {
  S16BIT Err;

  m_devNum = devNum;

  Err = aceInitialize(m_devNum, ACE_ACCESS_CARD, ACE_MODE_BC, 0, 0, 0);

  if (Err) {
    return Err;
  }

  return 0;
}

int BC::stopBc() {
  S16BIT Err;

  Err = aceBCStop(m_devNum);

  if (Err) {
    return Err;
  }

  return 0;
}

int BC::bcToRt(int rt, int sa, int wc, U8BIT bus,
               std::array<std::string, 32> data) {
  S16BIT Err;
  U16BIT hexData[32];

  // Convert string array to unsigned short array
  for (int i = 0; i < 32; ++i) {
    hexData[i] =
        static_cast<unsigned short>(strtoul(data.at(i).c_str(), nullptr, 16));
  }

  int messageId = 1;
  int dataBlockId = 1;
  dataBlockId++;

  int opCode1 = 1;
  int opCode2 = 2;

  int minorFrame = 1;
  int majorFrame = 2;

  // Create 3 data blocks
  Err = aceBCDataBlkCreate(m_devNum, dataBlockId, 32, hexData, 32);

  if (Err) {
    return Err;
  }

  // Create message block
  Err = aceBCMsgCreateBCtoRT(m_devNum, messageId, dataBlockId, rt, sa, wc, 0,
                             bus);
  if (Err) {
    return Err;
  }

  // Create XEQ opcode that will use msg block
  Err = aceBCOpCodeCreate(m_devNum, opCode1, ACE_OPCODE_XEQ, ACE_CNDTST_ALWAYS,
                          messageId, 0, 0);
  if (Err) {
    return Err;
  }

  // Create CAL opcode that will call mnr frame from major
  Err = aceBCOpCodeCreate(m_devNum, opCode2, ACE_OPCODE_CAL, ACE_CNDTST_ALWAYS,
                          minorFrame, 0, 0);
  if (Err) {
    return Err;
  }

  S16BIT aOpCodes[10] = {0x0000};

  // Create Minor Frame
  aOpCodes[0] = opCode1;
  Err = aceBCFrameCreate(m_devNum, minorFrame, ACE_FRAME_MINOR, aOpCodes, 1, 0,
                         0);
  if (Err) {
    return Err;
  }

  // Create Major Frame
  aOpCodes[0] = opCode2;
  Err = aceBCFrameCreate(m_devNum, majorFrame, ACE_FRAME_MAJOR, aOpCodes, 1,
                         1000, 0);
  if (Err) {
    return Err;
  }

  // Start BC
  int repeatCount = 1; // Set to -1 for infinite
  Err = aceBCStart(m_devNum, majorFrame, repeatCount);
  if (Err) {
    return Err;
  }

  aceBCDataBlkDelete(m_devNum, dataBlockId); // FIXME: does nothing

  return 0;
}

int BC::rtToBc(int rt, int sa, int wc, U8BIT bus) { return 0; }