#include "bc.hpp"
#include "bc.h"
#include "bcx.h"
#include <iostream>

constexpr int MSG_BC_TO_RT_ID = 1;
constexpr int MSG_RT_TO_BC_ID = 2;
constexpr int MSG_RT_TO_RT_ID = 3;

constexpr int DATA_BLK_BC_TO_RT_ID = 4;
constexpr int DATA_BLK_RT_TO_BC_ID = 5;
constexpr int DATA_BLK_RT_TO_RT_ID = 6;

constexpr int OP_CODE_1 = 1;
constexpr int OP_CODE_2 = 2;
constexpr int OP_CODE_3 = 3;

constexpr int MNR_FRAME = 1;

constexpr int MJR_FRAME_1 = 2;
constexpr int MJR_FRAME_2 = 3;

BC::BC() : m_devNum(0x0000) {}

BC::~BC() { aceFree(m_devNum); }

int BC::startBc(S16BIT devNum) {
  S16BIT Err;

  m_devNum = devNum;

  Err = aceInitialize(m_devNum, ACE_ACCESS_CARD, ACE_MODE_BC, 0, 0, 0);

  if (Err) {
    return Err;
  }

  U16BIT initialBuffer[32] = {0x0000};

  // Create BC -> RT data block
  Err =
      aceBCDataBlkCreate(m_devNum, DATA_BLK_BC_TO_RT_ID, 32, initialBuffer, 32);

  if (Err) {
    return Err;
  }

  // Create RT -> BC data block
  Err =
      aceBCDataBlkCreate(m_devNum, DATA_BLK_RT_TO_BC_ID, 32, initialBuffer, 32);

  if (Err) {
    return Err;
  }

  // Create BC -> RT message block
  Err = aceBCMsgCreateBCtoRT(m_devNum, MSG_BC_TO_RT_ID, DATA_BLK_BC_TO_RT_ID, 0,
                             0, 0, 0, ACE_BCCTRL_CHL_A);
  if (Err) {
    return Err;
  }

  // Create RT -> BC message block
  Err = aceBCMsgCreateBCtoRT(m_devNum, MSG_RT_TO_BC_ID, DATA_BLK_RT_TO_BC_ID, 0,
                             0, 0, 0, ACE_BCCTRL_CHL_A);
  if (Err) {
    return Err;
  }

  // Create XEQ opcode that will use BC -> RT msg block
  Err = aceBCOpCodeCreate(m_devNum, OP_CODE_1, ACE_OPCODE_XEQ,
                          ACE_CNDTST_ALWAYS, MSG_BC_TO_RT_ID, 0, 0);
  if (Err) {
    return Err;
  }

  // Create XEQ opcode that will use RT -> BC msg block
  Err = aceBCOpCodeCreate(m_devNum, OP_CODE_3, ACE_OPCODE_XEQ,
                          ACE_CNDTST_ALWAYS, MSG_RT_TO_BC_ID, 0, 0);
  if (Err) {
    return Err;
  }

  // Create CAL opcode that will call BC -> RT mnr frame from major
  Err = aceBCOpCodeCreate(m_devNum, OP_CODE_2, ACE_OPCODE_CAL,
                          ACE_CNDTST_ALWAYS, MNR_FRAME, 0, 0);
  if (Err) {
    return Err;
  }

  S16BIT aOpCodes[10] = {0x0000};

  // Create BC -> RT Minor Frame
  aOpCodes[0] = OP_CODE_1;
  Err =
      aceBCFrameCreate(m_devNum, MNR_FRAME, ACE_FRAME_MINOR, aOpCodes, 1, 0, 0);
  if (Err) {
    return Err;
  }

  // Create BC -> RT Major Frame
  aOpCodes[0] = OP_CODE_2;
  Err = aceBCFrameCreate(m_devNum, MJR_FRAME_1, ACE_FRAME_MAJOR, aOpCodes, 1,
                         1000, 0);
  if (Err) {
    return Err;
  }

  // Create RT -> BC Major Frame
  aOpCodes[0] = OP_CODE_3;
  Err = aceBCFrameCreate(m_devNum, MJR_FRAME_2, ACE_FRAME_MAJOR, aOpCodes, 1,
                         1000, 0);
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

  stopBc();

  Err = aceBCMsgModifyBCtoRT(m_devNum, MSG_BC_TO_RT_ID, DATA_BLK_BC_TO_RT_ID,
                             rt, sa, wc, 0, bus, 0x000F);
  if (Err) {
    return Err;
  }

  // Convert string array to unsigned short array
  for (int i = 0; i < 32; ++i) {
    messageBuffer[i] =
        static_cast<unsigned short>(strtoul(data.at(i).c_str(), nullptr, 16));
  }

  Err = aceBCDataBlkWrite(m_devNum, DATA_BLK_BC_TO_RT_ID, messageBuffer, 32, 0);
  if (Err) {
    return Err;
  }

  // Start BC
  int repeatCount = 1; // Set to -1 for infinite
  Err = aceBCStart(m_devNum, MJR_FRAME_1, repeatCount);
  if (Err) {
    return Err;
  }

  return 0;
}

int BC::rtToBc(int rt, int sa, int wc, U8BIT bus) {
  S16BIT Err;

  stopBc();

  Err = aceBCMsgModifyRTtoBC(m_devNum, MSG_RT_TO_BC_ID, DATA_BLK_RT_TO_BC_ID,
                             rt, sa, wc, 0, bus, 0x000F);
  if (Err) {
    return Err;
  }

  Err = aceBCDataBlkWrite(m_devNum, DATA_BLK_RT_TO_BC_ID, messageBuffer, 32, 0);
  if (Err) {
    return Err;
  }

  // Start BC
  int repeatCount = 1; // Set to -1 for infinite
  Err = aceBCStart(m_devNum, MJR_FRAME_2, repeatCount);
  if (Err) {
    return Err;
  }

  return 0;
}

// TODO: implement
int BC::rtToRt(int rt, int sa, int wc, U8BIT bus) { return 0; }
