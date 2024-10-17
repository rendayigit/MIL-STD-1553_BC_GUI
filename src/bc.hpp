#ifndef BC_HPP
#define BC_HPP

#include "stdace.h"
#include <array>
#include <string>

enum class BUS { A = 0x0080, B = 0x0000 };

class BC {
public:
  BC();
  ~BC();

  int startBc(S16BIT devNum);
  int stopBc();
  int bcToRt(int rt, int sa, int wc, BUS bus, std::array<std::string, 32> data);
  int rtToBc(int rt, int sa, int wc, BUS bus);

private:
  int transmit(BCMsgHandle msg);
  int displayResult(BCMinorFrmHandle minorFrameHandle);

  S16BIT m_devNum;
};

#endif // BC_HPP