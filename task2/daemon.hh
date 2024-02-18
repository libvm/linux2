#ifndef DAEMON_HH
#define DAEMON_HH
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <syslog.h>
#include <csignal>
#include <string>

class SignalHandler
{
  private:
  static bool isRunning;
  static void signalStart(int signal);
  static void signalPause(int signal);
  static void signalStop(int signal);
  static void signalGetStatus(int signal);

  public:
  static void setStatus(bool value);
  static bool getStatus();
  static void setupSignalHandlers();
};

void backupFiles(const std::string& srcDir, const std::string& destDir);
void readConfigFile(const std::string& cfgFile, std::string& srcDir, std::string& destDir, int& interval);

#endif
