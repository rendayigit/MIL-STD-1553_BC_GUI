#include "app-window.h"
#include "bc.hpp"
#include "common.hpp"
#include "fileOperations/fileOperations.hpp"
#include "json/json.hpp"

#include <exception>
#include <string>
#include <thread>

int main(int /*argc*/, char ** /*argv*/) {
  S16BIT errorCode = 0;
  std::thread configRunnerThread;
  bool threadLoop = false;
  auto ui = AppWindow::create();
  int threadDelay = 0;
  BC bc;

  ui->on_connectPressed([&] {
    U8BIT deviceNum =
        static_cast<unsigned short>(strtoul(ui->global<guiGlobals>().get_device().data(), nullptr, HEX_BYTE));

    errorCode = bc.startBc(deviceNum);

    slint::SharedString statusMessage = getStatus(errorCode).c_str();

    if (errorCode == 0) {
      ui->invoke_setConnectStatus(true);
    } else {
      ui->invoke_setConnectStatus(false);
      ui->invoke_setError(statusMessage);
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

  ui->on_browseConfig([&] {
    try {
      char filename[1024];
      FILE *f = popen("zenity --file-selection", "r");
      fgets(filename, 1024, f);
      ui->invoke_setConfigPath(filename);
      ui->invoke_setConnectStatus(true);
    } catch (std::exception &e) {
      ui->invoke_setError(e.what());
    }
  });

  ui->on_startConfigRun([&](const slint::SharedString &configFile) {
    if (configFile.empty()) {
      ui->invoke_setError("Config file path error");
      return false;
    }

    ui->invoke_setConnectStatus(true);

    threadDelay = Json(FileOperations::getInstance().getExecutableDirectory() + "../config.json")
                      .getNode("CONFIG_RUNNER_DELAY_BETWEEN_MESSAGES")
                      .getValue<int>();

    std::string path = std::string(configFile);

    // Remove newline characters
    path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());

    // test json file
    try {
      Json json(path);
      json.getNode("Commands").at(0).getNode("BC_MODE").getValue<int>();
    } catch (std::exception &e) {
      ui->invoke_setError(e.what());
      return false;
    }

    bc.setCommandFilePath(path);

    threadLoop = false;

    if (configRunnerThread.joinable()) {
      configRunnerThread.join();
    }

    try {
      threadLoop = true;
      configRunnerThread = std::thread([&] {
        while (threadLoop) {
          errorCode = bc.configRun();

          if (errorCode != 0) {
            slint::invoke_from_event_loop([&] { ui->invoke_setError(getStatus(errorCode).c_str()); });
            threadLoop = false;
            return false;
          }

          std::this_thread::sleep_for(std::chrono::milliseconds(threadDelay));
        }

        return true;
      });
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      return false;
    }

    return true;
  });

  ui->on_stopPressed([&] {
    threadLoop = false;

    if (configRunnerThread.joinable()) {
      configRunnerThread.join();
    }

    errorCode = bc.stopBc();

    if (errorCode != 0) {
      ui->invoke_setError(getStatus(errorCode).c_str());
    } else {
      ui->invoke_setConnectStatus(true);
    }

    return errorCode == 0;
  });

  ui->run();
  return 0;
}
