#include "app-window.h"
#include "bc.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char **argv) {
  int errorCode = 0;

  auto ui = AppWindow::create();

  // std::thread outputThread([&] {
  //   std::string line;
  //   while (true) {
  //     // Get the current time from the system clock
  //     auto now = std::chrono::system_clock::now();

  //     // Convert to time_t to extract calendar date
  //     auto time_t_now = std::chrono::system_clock::to_time_t(now);
  //     std::tm *local_time = std::localtime(&time_t_now);

  //     // Extract milliseconds
  //     auto duration = now.time_since_epoch();
  //     auto milliseconds =
  //         std::chrono::duration_cast<std::chrono::milliseconds>(duration) %
  //         1000;

  //     // Extract nanoseconds
  //     auto nanoseconds =
  //         std::chrono::duration_cast<std::chrono::nanoseconds>(duration) %
  //         1000000;

  //     // Allocate a char* to hold the formatted date and time string
  //     char *dateTime =
  //         new char[50]; // Allocate enough space for the formatted string

  //     // Format the date and time into the char* (snprintf is safer than
  //     // sprintf)
  //     snprintf(dateTime, 50, "%02d/%02d/%04d %02d:%02d:%02d:%03lld:%06lld",
  //              local_time->tm_mday,
  //              local_time->tm_mon + 1,     // tm_mon is 0-based, so add 1
  //              local_time->tm_year + 1900, // tm_year is years since 1900
  //              local_time->tm_hour, local_time->tm_min, local_time->tm_sec,
  //              milliseconds.count(), nanoseconds.count());

  //     slint::invoke_from_event_loop(
  //         [&]() { ui->invoke_setTime(dateTime); });
  //   }
  // });

  BC bc;

  ui->on_connectPressed([&] {
    U8BIT deviceNum = static_cast<unsigned short>(
        strtoul(ui->global<guiGlobals>().get_device().data(), nullptr, 16));

    errorCode = bc.startBc(deviceNum);
    if (errorCode == 0) {
      ui->invoke_setStatus("✅");
    } else {
      ui->invoke_setError(BuErrorStr(errorCode));
    }
  });

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
