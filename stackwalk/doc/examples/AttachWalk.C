Walker* walker = Walker::newWalker(pid);
std::vector<Frame> swalk;
for (;;)  {
  walker->walkStack(swalk);
  sleep(5);
}
