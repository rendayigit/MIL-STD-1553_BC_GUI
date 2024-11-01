#include "app-window.h"
#include "bc.hpp"
#include "common.hpp"
#include <string>

int main(int  /*argc*/, char ** /*argv*/) {
  S16BIT errorCode = 0;

  auto ui = AppWindow::create();

  BC bc;

  ui->on_connectPressed([&] {
    U8BIT deviceNum = static_cast<unsigned short>(strtoul(ui->global<guiGlobals>().get_device().data(), nullptr, HEX_BYTE));

    aceFree(deviceNum);
    errorCode = bc.startBc(deviceNum);
    if (errorCode == 0) {
      ui->invoke_setConnectStatus(true);
    } else {
      ui->invoke_setConnectStatus(false);
      ui->invoke_setError(getStatus(errorCode).c_str());
    }
  });

  ui->on_sendPressed([&](int commandType) {
    int rt = ui->global<guiGlobals>().get_rt();
    int sa = ui->global<guiGlobals>().get_sa();
    int rtRx = ui->global<guiGlobals>().get_rt_rx();
    int saRx = ui->global<guiGlobals>().get_sa_rx();
    int wc = ui->global<guiGlobals>().get_wc();
    slint::SharedString busString = ui->global<guiGlobals>().get_bus();
    int bus = busString == "A" ? ACE_BCCTRL_CHL_A : ACE_BCCTRL_CHL_B;
    bool isRepeat = ui->global<guiGlobals>().get_is_repeat();

    std::array<std::string, RT_SA_MAX_COUNT> data;

    for (int i = 0; i < ui->global<guiGlobals>().get_words()->row_count(); ++i) {
      data.at(i) = ui->global<guiGlobals>().get_words()->row_data(i)->data();
    }

    if (commandType == 0) {
      errorCode = bc.bcToRt(rt, sa, wc, bus, data, isRepeat);
    } else if (commandType == 1) {
      errorCode = bc.rtToBc(rt, sa, wc, bus, isRepeat);
    } else if (commandType == 2) {
      errorCode = bc.rtToRt(rt, sa, rtRx, saRx, wc, bus, isRepeat);
    }

    if (errorCode != 0) {
      ui->invoke_setError(getStatus(errorCode).c_str());
    }
  });

  ui->on_stopPressed([&] { bc.stopBc(); });

  ui->run();
  return 0;
}
