Walker* walker = Walker::newWalker(pid);
std::vector<Frame> swalk;
for (;;)  {
  walker->walkStack(swalk);
  struct timeval timeout;
  timeout.tv_sec = 5;
  timeout.tv_usec = 0;
  int max = ProcDebug::getNotificationFD() + 1;
  fd_set readfds, writefds, exceptfds;
  FD_ZERO(&readfds); FD_ZERO(&writefds); FD_ZERO(&exceptfds);
  FD_SET(ProcDebug::getNotificationFD(), &readfds);
  for (;;)  {
    int result = select(max, &readfds, &writefds, &exceptfds, &timeout);
    if (FD_ISSET(ProcDebug::getNotificationFD(), &readfds))  {
      //Debug event
      ProcDebug::handleDebugEvent();
    }
    if (result == 0)  {
      //Timeout
      break;
    }
  }
}
