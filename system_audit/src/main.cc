#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <regex>
#include "logger.hh"

using namespace std;

unordered_map<long, string> syscall_names;

void populateSyscallNames() {
  ifstream file("/usr/include/x86_64-linux-gnu/asm/unistd_64.h");
  string line;
  bool in_syscall_section = false;
  regex syscall_def_regex("^#define\\s+__NR_(\\w+)\\s+(\\d+)");
  smatch match;
  while (getline(file, line)) {
    if (regex_search(line, match, syscall_def_regex)) {
      in_syscall_section = true;
      string name = match[1];
      long num = stol(match[2]);
      syscall_names[num] = name;
    } else if (in_syscall_section && line.find("#ifndef") != string::npos) {
      break;
    }
  }
}

string getSyscallName(long num) {
  auto it = syscall_names.find(num);
  return (it != syscall_names.end()) ? it->second : "Unknown";
}

int main(int argc, char** argv) {
  populateSyscallNames();
  if (argc != 2) {
    cout << "Usage: sudo ./Audit <pid>" << endl;
    return EXIT_FAILURE;
  }

  int pid = stoi(argv[1]);

  Logger logger(pid);

  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) == -1) {
    perror("ptrace attach");
    return EXIT_FAILURE;
    }

  logger.log("Attached");

  if (ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_TRACESYSGOOD) == -1) {
    perror("ptrace setoptions");
    return EXIT_FAILURE;
  }

  logger.log("Set sys call listener");

  int status;
  waitpid(pid, &status, 0);
  user_regs_struct regs;
  long last_syscall = -1;

  while (WIFSTOPPED(status)) {
    if (ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr) == -1) {
        perror("ptrace syscall");
        return EXIT_FAILURE;
    }

    waitpid(pid, &status, 0);

    if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) {
      perror("ptrace getregs");
      return EXIT_FAILURE;
    }

    long current_syscall = regs.orig_rax;
    if (current_syscall != last_syscall) {
      logger.log(getSyscallName(current_syscall)); // Log syscall name
      last_syscall = current_syscall;
      }
  }

  if (ptrace(PTRACE_DETACH, pid, nullptr, nullptr) == -1) {
    perror("ptrace detach");
    return EXIT_FAILURE;
  }

  logger.log("Detached");

  return 0;
}

