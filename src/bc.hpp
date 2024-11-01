#ifndef BC_HPP
#define BC_HPP

#include "common.hpp"
#include "stdemace.h"
#include <array>
#include <string>

class BC {
public:
  BC();
  ~BC();

  S16BIT startBc(S16BIT devNum);
  S16BIT stopBc() const;
  S16BIT bcToRt(int rt, int sa, int wc, U8BIT bus, std::array<std::string, RT_SA_MAX_COUNT> data, bool isRepeat);
  S16BIT rtToBc(int rt, int sa, int wc, U8BIT bus, bool isRepeat);
  S16BIT rtToRt(int rtTx, int saTx, int rtRx, int saRx, int wc, U8BIT bus, bool isRepeat);

private:
  U16BIT m_messageBuffer[RT_SA_MAX_COUNT]; // NOLINT(hicpp-avoid-c-arrays, modernize-avoid-c-arrays,
                                           // cppcoreguidelines-avoid-c-arrays)
  S16BIT m_devNum;
};

#endif // BC_HPP