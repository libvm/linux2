#include "Logger.hh"

using namespace std;

ofstream logfile;
Logger::Logger(int pid) {
    string filename = "audit_log_" + to_string(pid) + ".txt";
    logfile.open(filename, ios::out | ios::app);
    if (!logfile.is_open()) {
      cerr << "Error opening log file" << endl;
    }
}

Logger::~Logger() {
     if (logfile.is_open()) {
       logfile.close();
     }
}

void Logger::log(const string& message) {
    if (logfile.is_open()) {
      logfile << message << endl;
    }
}
