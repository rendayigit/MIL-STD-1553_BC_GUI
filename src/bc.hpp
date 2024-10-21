#ifndef BC_HPP
#define BC_HPP

#include "stdemace.h"
#include <array>
#include <string>

class BC {
public:
  BC();
  ~BC();

  int startBc(S16BIT devNum);
  int stopBc();
  int bcToRt(int rt, int sa, int wc, U8BIT bus,
             std::array<std::string, 32> data, bool isRepeat);
  int rtToBc(int rt, int sa, int wc, U8BIT bus, bool isRepeat);
  int rtToRt(int rt_tx, int sa_tx, int rt_rx, int sa_rx, int wc, U8BIT bus,
             bool isRepeat);

private:
  U16BIT messageBuffer[32];
  S16BIT m_devNum;
};

#endif // BC_HPP