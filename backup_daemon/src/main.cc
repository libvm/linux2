#include <iostream>
#include <fstream>
#include <thread>
#include <syslog.h>
#include "SignalHandler.hh"

using namespace std;

void backupFiles(const string& srcDir, const string& destDir) {
  openlog("BackupDaemon", LOG_PID, LOG_DAEMON);
  string curTime = to_string(time(0));
  string command = "cp -r " + srcDir + " " + destDir + "/backup_" + curTime;
  system(command.c_str());

  syslog(LOG_INFO, "Backup completed successfully.");
  cout << "Backup completed: " << curTime << endl;
}

void readConfigFile(const string& cfgFile, string& srcDir, string& destDir, int& interval) {
  ifstream file(cfgFile);
  if (file.is_open()) {
      file >> srcDir >> destDir >> interval;
      file.close();
  } else {
      syslog(LOG_ERR, "Unable to open config file: %s", cfgFile.c_str());
      exit(EXIT_FAILURE);
  }
}

int main() {
  const string cfgPath = "/home/libvm/linux2/backup_daemon/daemon.conf";
  string srcDir, destDir;
  int interval;
  SignalHandler::setupSignalHandlers();
  openlog("BackupDaemon", LOG_PID, LOG_USER);
  while (SignalHandler::getStatus()) {
    readConfigFile(cfgPath, srcDir, destDir, interval);
    syslog(LOG_INFO, "Performing backup from %s to %s", srcDir.c_str(), destDir.c_str());
    backupFiles(srcDir, destDir);
    syslog(LOG_INFO, "Backup completed. Waiting for next interval...");

    this_thread::sleep_for(chrono::seconds(interval));
  }

  closelog();

  return 0;
}
