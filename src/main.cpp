#include "app-window.h"
#include "bc.hpp"
#include <iostream>
#include <string>

int main(int argc, char **argv) {
  int errorCode = 0;

  auto ui = AppWindow::create();

  BC bc;
  errorCode = bc.startBc(0x0000);
  if (errorCode != 0) {
    ui->invoke_setError(BuErrorStr(errorCode));
  }

  ui->on_sendPressed([&](int commandType) {
    int rt = ui->global<guiGlobals>().get_rt();
    int sa = ui->global<guiGlobals>().get_sa();
    int wc = ui->global<guiGlobals>().get_wc();
    slint::SharedString busString = ui->global<guiGlobals>().get_bus();
    BUS bus = busString == "A" ? BUS::A : BUS::B;

    U16BIT data[32];

    for (int i = 0; i < ui->global<guiGlobals>().get_words()->row_count();
         ++i) {
      auto hexWordString =
          ui->global<guiGlobals>().get_words()->row_data(i)->data();

      // Convert string to unsigned short
      data[i] =
          static_cast<unsigned short>(strtoul(hexWordString, nullptr, 16));
    }

    if (commandType == 0) {
      errorCode = bc.bcToRt(rt, sa, wc, bus, data);

      if (errorCode != 0) {
        ui->invoke_setError(BuErrorStr(errorCode));
      }
    } else if (commandType == 1) {
      errorCode = bc.rtToBc(rt, sa, wc, bus, data);

      if (errorCode != 0) {
        ui->invoke_setError(BuErrorStr(errorCode));
      }
    }
  });

  ui->run();
  return 0;
}
