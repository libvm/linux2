#include <iostream>
#include <syslog.h>
#include <csignal>
#include "SignalHandler.hh"

using namespace std;

bool SignalHandler::isRunning = true;
void SignalHandler::signalStart(int signal) {
  isRunning = true;
  syslog(LOG_INFO, "Started");
  }

void SignalHandler::signalPause(int signal) {
  isRunning = false;
  syslog(LOG_INFO, "Paused");
  }

void SignalHandler::signalStop(int signal) {
  syslog(LOG_INFO, "Stoped");
  closelog();
  exit(EXIT_SUCCESS);
  }

void SignalHandler::signalGetStatus(int signal) {
  if (isRunning) syslog(LOG_INFO, "Daemon is active");
  else syslog(LOG_INFO, "Daemon is inactive");
  }

void SignalHandler::setStatus(bool value) {
  isRunning = value;
  }

bool SignalHandler::getStatus() {
  return isRunning;
  }

void SignalHandler::setupSignalHandlers() {
  signalStart(0);
  signal(SIGCONT, signalStart);
  signal(SIGTSTP, signalPause);
  signal(SIGTERM, signalStop);
  signal(SIGUSR1, signalGetStatus);
  signalStart(0);
  }
