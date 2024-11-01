#include "bc.hpp"
#include "bc.h"
#include "bcx.h"
#include "common.hpp"
#include <iostream>

constexpr int MOD_FLAGS = 0x000F;

constexpr int MSG_BC_TO_RT_ID = 1;
constexpr int MSG_RT_TO_BC_ID = 2;
constexpr int MSG_RT_TO_RT_ID = 3;

constexpr int DATA_BLK_BC_TO_RT_ID = 4;
constexpr int DATA_BLK_RT_TO_BC_ID = 5;
constexpr int DATA_BLK_RT_TO_RT_ID = 6;

constexpr int OP_CODE_1 = 1;
constexpr int OP_CODE_2 = 2;
constexpr int OP_CODE_3 = 3;
constexpr int OP_CODE_4 = 4;

constexpr int MNR_FRAME = 1;

constexpr int MJR_FRAME_1 = 2;
constexpr int MJR_FRAME_2 = 3;
constexpr int MJR_FRAME_3 = 4;

constexpr int MNR_FRAME_TIME = 1000;

BC::BC() : m_messageBuffer(), m_devNum(0x0000) {
  // TODO(renda): read config.json here
  // TODO(renda): add feature to read multiple configs with multiple data sets to periodically transmit
}

BC::~BC() { aceFree(m_devNum); }

S16BIT BC::startBc(S16BIT devNum) {
  S16BIT err = 0;

  m_devNum = devNum;

  err = aceInitialize(m_devNum, ACE_ACCESS_CARD, ACE_MODE_BC, 0, 0, 0);

  if (err != 0) {
    return err;
  }

  U16BIT initialBuffer[RT_SA_MAX_COUNT] = {0x0000}; // NOLINT(hicpp-avoid-c-arrays, modernize-avoid-c-arrays,
                                                    // cppcoreguidelines-avoid-c-arrays)

  // Create BC -> RT data block
  err = aceBCDataBlkCreate(m_devNum, DATA_BLK_BC_TO_RT_ID, RT_SA_MAX_COUNT, initialBuffer, RT_SA_MAX_COUNT);

  if (err != 0) {
    return err;
  }

  // Create RT -> BC data block
  err = aceBCDataBlkCreate(m_devNum, DATA_BLK_RT_TO_BC_ID, RT_SA_MAX_COUNT, initialBuffer, RT_SA_MAX_COUNT);

  if (err != 0) {
    return err;
  }

  // Create RT -> RT data block
  err = aceBCDataBlkCreate(m_devNum, DATA_BLK_RT_TO_RT_ID, RT_SA_MAX_COUNT, initialBuffer, RT_SA_MAX_COUNT);

  if (err != 0) {
    return err;
  }

  // Create BC -> RT message block
  err = aceBCMsgCreateBCtoRT(m_devNum, MSG_BC_TO_RT_ID, DATA_BLK_BC_TO_RT_ID, 0, 0, 0, 0, ACE_BCCTRL_CHL_A);
  if (err != 0) {
    return err;
  }

  // Create RT -> BC message block
  err = aceBCMsgCreateBCtoRT(m_devNum, MSG_RT_TO_BC_ID, DATA_BLK_RT_TO_BC_ID, 0, 0, 0, 0, ACE_BCCTRL_CHL_A);
  if (err != 0) {
    return err;
  }

  // Create RT -> RT message block
  err = aceBCMsgCreateRTtoRT(m_devNum, MSG_RT_TO_RT_ID, DATA_BLK_RT_TO_RT_ID, 0, 0, 0, 0, 0, 0, ACE_BCCTRL_CHL_A);
  if (err != 0) {
    return err;
  }

  // Create XEQ opcode that will use BC -> RT msg block
  err = aceBCOpCodeCreate(m_devNum, OP_CODE_1, ACE_OPCODE_XEQ, ACE_CNDTST_ALWAYS, MSG_BC_TO_RT_ID, 0, 0);
  if (err != 0) {
    return err;
  }

  // Create XEQ opcode that will use RT -> BC msg block
  err = aceBCOpCodeCreate(m_devNum, OP_CODE_3, ACE_OPCODE_XEQ, ACE_CNDTST_ALWAYS, MSG_RT_TO_BC_ID, 0, 0);
  if (err != 0) {
    return err;
  }

  // Create XEQ opcode that will use RT -> RT msg block
  err = aceBCOpCodeCreate(m_devNum, OP_CODE_4, ACE_OPCODE_XEQ, ACE_CNDTST_ALWAYS, MSG_RT_TO_RT_ID, 0, 0);
  if (err != 0) {
    return err;
  }

  // Create CAL opcode that will call BC -> RT mnr frame from major
  err = aceBCOpCodeCreate(m_devNum, OP_CODE_2, ACE_OPCODE_CAL, ACE_CNDTST_ALWAYS, MNR_FRAME, 0, 0);
  if (err != 0) {
    return err;
  }

  S16BIT aOpCodes[10] = {0x0000}; // NOLINT(hicpp-avoid-c-arrays, modernize-avoid-c-arrays,
                                  // cppcoreguidelines-avoid-c-arrays)

  // Create BC -> RT Minor Frame
  aOpCodes[0] = OP_CODE_1;
  err = aceBCFrameCreate(m_devNum, MNR_FRAME, ACE_FRAME_MINOR, aOpCodes, 1, 0, 0);
  if (err != 0) {
    return err;
  }

  // Create BC -> RT Major Frame
  aOpCodes[0] = OP_CODE_2;
  err = aceBCFrameCreate(m_devNum, MJR_FRAME_1, ACE_FRAME_MAJOR, aOpCodes, 1, MNR_FRAME_TIME, 0);
  if (err != 0) {
    return err;
  }

  // Create RT -> BC Major Frame
  aOpCodes[0] = OP_CODE_3;
  err = aceBCFrameCreate(m_devNum, MJR_FRAME_2, ACE_FRAME_MAJOR, aOpCodes, 1, MNR_FRAME_TIME, 0);
  if (err != 0) {
    return err;
  }

  // Create RT -> RT Major Frame
  aOpCodes[0] = OP_CODE_4;
  err = aceBCFrameCreate(m_devNum, MJR_FRAME_3, ACE_FRAME_MAJOR, aOpCodes, 1, MNR_FRAME_TIME, 0);
  if (err != 0) {
    return err;
  }

  return 0;
}

S16BIT BC::stopBc() const {
  S16BIT err = 0;

  err = aceBCStop(m_devNum);

  if (err != 0) {
    return err;
  }

  return 0;
}

S16BIT BC::bcToRt(int rt, int sa, int wc, U8BIT bus, std::array<std::string, RT_SA_MAX_COUNT> data, bool isRepeat) {
  S16BIT err = 0;

  stopBc();

  err = aceBCMsgModifyBCtoRT(m_devNum, MSG_BC_TO_RT_ID, DATA_BLK_BC_TO_RT_ID, rt, sa, wc, 0, bus, MOD_FLAGS);
  if (err != 0) {
    return err;
  }

  // Convert string array to unsigned short array
  for (int i = 0; i < RT_SA_MAX_COUNT; ++i) {
    m_messageBuffer[i] = static_cast<unsigned short>( // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        strtoul(data.at(i).c_str(), nullptr, HEX_BYTE));
  }

  err = aceBCDataBlkWrite(m_devNum, DATA_BLK_BC_TO_RT_ID, m_messageBuffer, RT_SA_MAX_COUNT, 0);
  if (err != 0) {
    return err;
  }

  int repeatCount = 1;

  if (isRepeat) {
    repeatCount = -1;
  }

  // Start BC
  err = aceBCStart(m_devNum, MJR_FRAME_1, repeatCount);
  if (err != 0) {
    return err;
  }

  return 0;
}

S16BIT BC::rtToBc(int rt, int sa, int wc, U8BIT bus, bool isRepeat) {
  S16BIT err = 0;

  stopBc();

  err = aceBCMsgModifyRTtoBC(m_devNum, MSG_RT_TO_BC_ID, DATA_BLK_RT_TO_BC_ID, rt, sa, wc, 0, bus, MOD_FLAGS);
  if (err != 0) {
    return err;
  }

  err = aceBCDataBlkWrite(m_devNum, DATA_BLK_RT_TO_BC_ID, m_messageBuffer, RT_SA_MAX_COUNT, 0);
  if (err != 0) {
    return err;
  }

  int repeatCount = 1;

  if (isRepeat) {
    repeatCount = -1;
  }

  // Start BC
  err = aceBCStart(m_devNum, MJR_FRAME_2, repeatCount);
  if (err != 0) {
    return err;
  }

  return 0;
}

S16BIT BC::rtToRt(int rtTx, int saTx, int rtRx, int saRx, int wc, U8BIT bus, bool isRepeat) {
  S16BIT err = 0;

  stopBc();

  // TODO(renda): rt_tr, rt_rc, sa_tr, sa_rc
  err = aceBCMsgModifyRTtoRT(m_devNum, MSG_RT_TO_RT_ID, DATA_BLK_RT_TO_RT_ID, rtRx, saRx, wc, rtTx, saTx, 0, bus,
                             MOD_FLAGS);
  if (err != 0) {
    return err;
  }

  err = aceBCDataBlkWrite(m_devNum, DATA_BLK_RT_TO_RT_ID, m_messageBuffer, RT_SA_MAX_COUNT, 0);
  if (err != 0) {
    return err;
  }

  int repeatCount = 1;

  if (isRepeat) {
    repeatCount = -1;
  }

  // Start BC
  err = aceBCStart(m_devNum, MJR_FRAME_3, repeatCount);
  if (err != 0) {
    return err;
  }

  return 0;
}
