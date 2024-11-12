#include "bc.hpp"
#include "common.hpp"
#include "configData.hpp"
#include "fileOperations/fileOperations.hpp"
#include "json/json.hpp"

#include <array>
#include <iostream>
#include <string>

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

const std::string CONFIG_PATH = FileOperations::getInstance().getExecutableDirectory() + "../config.json";

BC::BC() : m_messageBuffer(), m_devNum(Json(CONFIG_PATH).getNode("DEFAULT_DEVICE_NUMBER").getValue<S16BIT>()) {
  std::array<std::string, RT_SA_MAX_COUNT> dataArray;

  for (int i = 0; i < RT_SA_MAX_COUNT; i++) {
    dataArray.at(i) = Json(CONFIG_PATH).getNode("UI_DEFAULT_Data").at(i).getValue<std::string>();
  }

  m_configData =
      new ConfigData(std::to_string(m_devNum), Json(CONFIG_PATH).getNode("UI_DEFAULT_BUS").getValue<std::string>(),
                     Json(CONFIG_PATH).getNode("UI_DEFAULT_RT_RX").getValue<int>(),
                     Json(CONFIG_PATH).getNode("UI_DEFAULT_SA_RX").getValue<int>(),
                     Json(CONFIG_PATH).getNode("UI_DEFAULT_RT_TX").getValue<int>(),
                     Json(CONFIG_PATH).getNode("UI_DEFAULT_SA_TX").getValue<int>(),
                     Json(CONFIG_PATH).getNode("UI_DEFAULT_WORD_COUNT").getValue<int>(),
                     Json(CONFIG_PATH).getNode("UI_DEFAULT_BC_MODE").getValue<int>(), dataArray);
}

BC::~BC() { aceFree(m_devNum); }

S16BIT BC::startBc(S16BIT devNum) {
  S16BIT err = 0;

  m_devNum = devNum;

  m_logger.log(LOG_INFO, "start bc with dev: " + std::to_string(m_devNum));

  err = aceFree(m_devNum);

  if (err != 0) {
    return err;
  }

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

S16BIT BC::stopBc() {
  S16BIT err = 0;

  m_logger.log(LOG_INFO, "stop bc with dev: " + std::to_string(m_devNum));

  err = aceBCStop(m_devNum);

  if (err != 0) {
    return err;
  }

  return 0;
}

S16BIT BC::bcToRt(int rt, int sa, int wc, U8BIT bus, std::array<std::string, RT_SA_MAX_COUNT> data, bool isRepeat) {
  S16BIT err = 0;

  std::string dataString;

  for (auto &item : data) {
    dataString += item;
  }

  m_logger.log(LOG_INFO, "bc->rt with dev: " + std::to_string(m_devNum) + " rt: " + std::to_string(rt) + " sa: " +
                             std::to_string(sa) + " wc: " + std::to_string(wc) + "bus: " + std::to_string(bus) +
                             " data: " + dataString + " is repeat: " + std::to_string(static_cast<int>(isRepeat)));

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

  m_logger.log(LOG_INFO, "rt->bc with dev: " + std::to_string(m_devNum) + " rt: " + std::to_string(rt) + " sa: " +
                             std::to_string(sa) + " wc: " + std::to_string(wc) + "bus: " + std::to_string(bus) +
                             " is repeat: " + std::to_string(static_cast<int>(isRepeat)));

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

  m_logger.log(LOG_INFO, "rt->rt with dev: " + std::to_string(m_devNum) + " rt tx: " + std::to_string(rtTx) +
                             " sa tx: " + std::to_string(saTx) + " rt rx: " + std::to_string(rtRx) + " sa rx: " +
                             std::to_string(saRx) + " wc: " + std::to_string(wc) + "bus: " + std::to_string(bus) +
                             " is repeat: " + std::to_string(static_cast<int>(isRepeat)));

  stopBc();

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

S16BIT BC::configRun() {
  S16BIT err = 0;
  int rt = 0;
  int sa = 0;
  int rtRx = 0;
  int saRx = 0;
  int rtTx = 0;
  int saTx = 0;

  std::array<std::string, RT_SA_MAX_COUNT> data;

  Json commands = Json(m_commandFilePath).getNode("Commands");
  // Json commands = Json("/home/t12023031214/renda/MIL-STD-1553_BC_GUI/commands.json").getNode("Commands");

  for (int i = 0; i < commands.getSize(); i++) {
    Json command = commands.at(i);
    int wc = command.getNode("WORD_COUNT").getValue<int>();
    U8BIT bus = command.getNode("Bus").getValue<std::string>() == "A" ? ACE_BCCTRL_CHL_A : ACE_BCCTRL_CHL_B;

    switch (command.getNode("BC_MODE").getValue<int>()) {
    case 0:
      rt = command.getNode("RT").getValue<int>();
      sa = command.getNode("SA").getValue<int>();

      data.fill("");

      for (int dataIndex = 0; dataIndex < command.getNode("Data").getSize(); dataIndex++) {
        data.at(dataIndex) = command.getNode("Data").at(dataIndex).getValue<std::string>();
      }

      err = bcToRt(rt, sa, wc, bus, data, false);
      if (err != 0) {
        return err;
      }

      break;
    case 1:
      rt = command.getNode("RT").getValue<int>();
      sa = command.getNode("SA").getValue<int>();

      err = rtToBc(rt, sa, wc, bus, false);
      if (err != 0) {
        return err;
      }

      break;
    case 2:
      rtRx = command.getNode("RT_RX").getValue<int>();
      saRx = command.getNode("SA_RX").getValue<int>();
      rtTx = command.getNode("RT_TX").getValue<int>();
      saTx = command.getNode("SA_TX").getValue<int>();

      err = rtToRt(rtTx, saTx, rtRx, saRx, wc, bus, false);
      if (err != 0) {
        return err;
      }

      break;
    default:
      break;
    }
  }

  return 0;
}
