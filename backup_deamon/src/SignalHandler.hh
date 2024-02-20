#ifndef DAEMON_HH
#define DAEMON_HH

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

#endif
