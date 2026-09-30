MySnippet::Ptr snippet = MySnippet::create(new MySnippet);

Patcher patcher(mgr);
for (std::vector<Point*>::iterator iter = pts.begin();
      iter != pts.end(); ++iter)  {
  Point* pt = *iter;
  patcher.add(PushBackCommand::create(pt, snippet));
}
patcher.commit();
