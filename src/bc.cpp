#include "bc.hpp"
#include <iostream>
#include <stdace.h>

BC::BC() : m_devNum(0x0000) {}

BC::~BC() {
  // Must call at end of ACE library use
  BuClose();
}

int BC::startBc(S16BIT devNum) {
  BuConf_t Conf; // ACE library configuration type
  BuError_t Err; // ACE library error status type

  Conf.ConfDev = 0x0000; // TODO(renda): set from gui
  Err = BuOpenLinux(&Conf);

  if (Err) {
    return Err;
  }

  // Opens bus controller mode
  Err = BuBCOpen();

  if (Err) {
    return Err;
  }

  // set response timeout to 50.5 us
  Err = BuTimeout(RESPONSE_505);

  if (Err) {
    return Err;
  }

  return 0;
}

int BC::stopBc() {
  BuError_t Err;

  // Closes bus controller mode
  Err = BuBCClose();

  if (Err) {
    return Err;
  }

  return 0;
}

int BC::bcToRt(int rt, int sa, int wc, BUS bus,
               std::array<std::string, 32> data) {
  BCMsgHandle msg;
  U16BIT hexData[32];

  for (int i = 0; i < 32; ++i) {
    // Convert string to unsigned short
    hexData[i] =
        static_cast<unsigned short>(strtoul(data.at(i).c_str(), nullptr, 16));
  }

  msg = BuBCXBCtoRT((U8BIT)rt, (U8BIT)sa, (U8BIT)wc, (U8BIT)bus, hexData,
                    BU_BCNOGAP, BU_BCALWAYS);

  int transmitStatus = transmit(msg);
  return transmitStatus;
}

int BC::rtToBc(int rt, int sa, int wc, BUS bus) {
  BCMsgHandle msg;
  msg = BuBCXRTtoBC((U8BIT)rt, (U8BIT)sa, (U8BIT)wc, (U8BIT)bus, BU_BCNOGAP,
                    BU_BCALWAYS);

  int transmitStatus = transmit(msg);
  return transmitStatus;
}

int BC::transmit(BCMsgHandle msg) {
  BuError_t Err;

  // Minor frame handle
  BCMinorFrmHandle myframe;

  // Create minor frame
  myframe = BuBCXMinorFrm(30000l, 1, &msg);

  // load minor frame into ACE stack
  Err = BuBCLoadMinor(BU_BCFRMBUFA, myframe);

  if (Err) {
    return Err;
  }

  // Run frame
  Err = BuBCRunMinor(BU_BCFRMBUFA, BU_BCSINGLE);

  if (Err) {
    return Err;
  }

  while (BuBCIsFrmActive())
    ;

  displayResult(myframe);

  return 0;
}

int BC::displayResult(BCMinorFrmHandle minorFrameHandle) {
  // Holds message result read from frame
  MsgType readmsg;

  // Display data
  if (BuBCReadMsgNum(minorFrameHandle, 0, &readmsg)) {
    printf("error reading message result\n");
  } else {
    U16BIT i;

    printf("Message Type = %s", BuMsgTypeStr(0xFF)); // Freadmsg.Type));

    if (readmsg.BlockStatus & MT_ERR)
      printf(" (EXCEPTION)\n");
    else
      printf("\n");

    printf(" Cmd1 %04X %s\n", readmsg.CmdWord1, BuCmdStr(readmsg.CmdWord1));

    if (readmsg.CmdWord2flag) {
      printf(" Cmd2 %04X %s\n", readmsg.CmdWord2, BuCmdStr(readmsg.CmdWord2));
    }

    printf(" Time %u uS\n", readmsg.TimeTag * 2); // 2us resolution
    printf(" GapT %u uS\n", readmsg.GapTime);     // 1us resolution
    printf(" BSW  %04X %s\n", readmsg.BlockStatus,
           BuBCBSWErrorStr(readmsg.BlockStatus));
    printf(" Ctrl %04X \n", readmsg.ControlWord);

    for (i = 0; i < readmsg.DataLength; ++i) {
      if (i == 0)
        printf(" Data ");

      printf("%04X  ", readmsg.Data[i]);

      if ((i % 6) == 5)
        printf("\n      ");
    }

    if (readmsg.Status1flag)
      printf("\n Sta1 %04X", readmsg.Status1);

    if (readmsg.Status2flag)
      printf("\n Sta2 %04X", readmsg.Status2);

    if (readmsg.LoopBack1flag)
      printf("\n Lpb1 %04X", readmsg.LoopBack1);
    if (readmsg.LoopBack2flag)
      printf("\n Lpb2 %04X", readmsg.LoopBack2);

    printf("\n\n");
  }

  return 0;
}
