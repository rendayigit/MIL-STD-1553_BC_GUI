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
  int bcToRt(int rt, int sa, int wc, U8BIT bus, std::array<std::string, 32> data);
  int rtToBc(int rt, int sa, int wc, U8BIT bus);
  int rtToRt(int rt, int sa, int wc, U8BIT bus);

private:
  U16BIT messageBuffer[32];
  S16BIT m_devNum;
};

#endif // BC_HPP