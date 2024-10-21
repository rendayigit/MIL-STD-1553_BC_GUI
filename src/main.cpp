#include "app-window.h"
#include "bc.hpp"
#include "common.hpp"
#include <string>

int main(int argc, char **argv) {
  int errorCode = 0;

  auto ui = AppWindow::create();

  BC bc;

  ui->on_connectPressed([&] {
    U8BIT deviceNum = static_cast<unsigned short>(
        strtoul(ui->global<guiGlobals>().get_device().data(), nullptr, 16));

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
    int rt_rx = ui->global<guiGlobals>().get_rt_rx();
    int sa_rx = ui->global<guiGlobals>().get_sa_rx();
    int wc = ui->global<guiGlobals>().get_wc();
    slint::SharedString busString = ui->global<guiGlobals>().get_bus();
    int bus = busString == "A" ? ACE_BCCTRL_CHL_A : ACE_BCCTRL_CHL_B;

    std::array<std::string, 32> data;

    for (int i = 0; i < ui->global<guiGlobals>().get_words()->row_count();
         ++i) {
      data.at(i) = ui->global<guiGlobals>().get_words()->row_data(i)->data();
    }

    if (commandType == 0) {
      errorCode = bc.bcToRt(rt, sa, wc, bus, data);
    } else if (commandType == 1) {
      errorCode = bc.rtToBc(rt, sa, wc, bus);
    } else if (commandType == 2) {
      errorCode = bc.rtToRt(rt, sa, rt_rx, sa_rx, wc, bus);
    }

    if (errorCode != 0) {
      ui->invoke_setError(getStatus(errorCode).c_str());
    }
  });

  ui->run();
  return 0;
}
