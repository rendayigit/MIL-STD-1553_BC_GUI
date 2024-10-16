#include "bc.hpp"
#include <iostream>
#include <stdace.h>

BC::BC() : m_devNum(0x0000) {}

BC::~BC() {
  // Must call at end of ACE library use
  BuClose();
}

int BC::startBc(S16BIT devNum) {
  BuConf_t Conf; /* ACE library configuration type */
  BuError_t Err; /* ACE library error status type  */
  Err = BuOpenLinux(&Conf);

  if (Err) {
    std::cerr << "BuError " << Err << BuErrorStr(Err) << std::endl;
  }

  // Opens bus controller mode
  BuBCOpen();

  // set response timeout to 50.5 us
  BuTimeout(RESPONSE_505);

  return 0;
}

int BC::stopBc() {
  // Closes bus controller mode
  BuBCClose();

  return 0;
}

int BC::bcToRt(int rt, int sa, int wc, BUS bus, U16BIT data[]) {
  BCMsgHandle msg;

  msg = BuBCXBCtoRT((U8BIT)rt, (U8BIT)sa, (U8BIT)wc, (U8BIT)bus, data,
                    BU_BCNOGAP, BU_BCALWAYS);

  int transmitStatus = transmit(msg);
  return transmitStatus;
}

int BC::rtToBc(int rt, int sa, int wc, BUS bus, U16BIT data[]) {
  BCMsgHandle msg;
  msg = BuBCXRTtoBC((U8BIT)rt, (U8BIT)sa, (U8BIT)wc, (U8BIT)bus, BU_BCNOGAP,
                    BU_BCALWAYS);

  int transmitStatus = transmit(msg);
  return transmitStatus;
}

int BC::transmit(BCMsgHandle msg) {
  // Minor frame handle
  BCMinorFrmHandle myframe;

  // Create minor frame
  myframe = BuBCXMinorFrm(30000l, 1, &msg);

  // load minor frame into ACE stack
  BuBCLoadMinor(BU_BCFRMBUFA, myframe);

  // Run frame
  BuBCRunMinor(BU_BCFRMBUFA, BU_BCSINGLE);

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

    printf(" Time %u uS\n", readmsg.TimeTag * 2); /*2us resolution*/
    printf(" GapT %u uS\n", readmsg.GapTime);     /*1us resolution*/
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
