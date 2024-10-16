#include "app-window.h"
#include "bc.hpp"
#include <iostream>

// TODO(renda): replace with data from gui
U16BIT data[] = {0X0000, 0x1111, 0x2222, 0x3333, 0x4444, 0x5555, 0x6666,
                 0x7777, 0x8888, 0x9999, 0xAAAA, 0xBBBB, 0xCCCC, 0xDDDD,
                 0xEEEE, 0xFFFF, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010,
                 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0800,
                 0x1000, 0x2000, 0x4000, 0x8000};

// TODO(renda): replace with bus selection from gui
BUS bus = BUS::A;

int main(int argc, char **argv) {
  auto ui = AppWindow::create();

  BC bc;
  bc.startBc(0x0000);

  ui->on_sendPressed([&](int commandType) {
    int rt = ui->global<guiGlobals>().get_rt();
    int sa = ui->global<guiGlobals>().get_sa();
    int wc = ui->global<guiGlobals>().get_wc();

    if (commandType == 0) {
      bc.bcToRt(rt, sa, wc, bus, data);
    } else if (commandType == 1) {
      bc.rtToBc(rt, sa, wc, bus, data);
    }
  });

  ui->run();
  return 0;
}
