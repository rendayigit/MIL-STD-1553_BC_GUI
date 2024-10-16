#include "bc.hpp"
#include "aceutils.h"
#include <stdace.h>

BC::BC() : m_devNum(0x0000) {}

BC::~BC() {}

int BC::startBc(S16BIT devNum) { return 0; }

int BC::stopBc() { return 0; }

int BC::bcToRt(int rt, int sa, int wc, BUS bus, U16BIT data[]) {
  BCMsgHandle msg;
  msg = BuBCXBCtoRT((U8BIT)rt, (U8BIT)sa, (U8BIT)wc, (U8BIT)bus, data,
                    BU_BCNOGAP, BU_BCALWAYS);
  return 0;
}

int BC::rtToBc(int rt, int sa, int wc, BUS bus, U16BIT data[]) {
  BCMsgHandle msg;
  msg = BuBCXRTtoBC((U8BIT)rt, (U8BIT)sa, (U8BIT)wc, (U8BIT)bus, BU_BCNOGAP,
                    BU_BCALWAYS);
  return 0;
}
