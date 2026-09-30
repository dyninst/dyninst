std::vector<Frame> stackwalk;
std::string s;

Walker* walker = Walker::newWalker();
walker->walkStack(stackwalk);
for (unsigned i=0; i<stackwalk.size(); i++)  {
  stackwalk[i].getName(s);
  cout << "Found function " << s << endl;
}
