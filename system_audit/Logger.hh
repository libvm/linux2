#ifndef AUDIT_HH
#define AUDIT_HH

#include <iostream>
#include <fstream>
#include <unordered_map>

class Logger {
private:
  std::ofstream logfile;

public:
  Logger(int pid);
  ~Logger();
  void log(const std::string& message);
};

#endif
