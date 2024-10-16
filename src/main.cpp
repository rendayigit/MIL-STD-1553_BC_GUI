#include "app-window.h"
#include "bc.hpp"
#include <iostream>

int main(int argc, char **argv) {
  auto ui = AppWindow::create();

  ui->on_sendPressed([&] {
    std::cout << "rt:" << ui->global<guiGlobals>().get_rt() << std::endl;
    std::cout << "sa:" << ui->global<guiGlobals>().get_sa() << std::endl;
    std::cout << "wc:" << ui->global<guiGlobals>().get_wc() << std::endl;
  });

  ui->run();
  return 0;
}
