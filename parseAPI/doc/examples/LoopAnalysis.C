void GetLoopInFunc(Function* f)
{
  // Get all loops in the function
  std::vector<Loop*> loops;
  f->getLoops(loops);

  // Iterate over all loops
  for (auto lit = loops.begin(); lit != loops.end(); ++lit)  {
    Loop* loop = *lit;

    // Get all the entry blocks of the loop
    std::vector<Block*> entries;
    loop->getLoopEntries(entries);

    // Get all the blocks in the loop
    std::vector<Block*> blocks;
    loop->getLoopBasicBlocks(blocks);

    // Get all the back edges in the loop
    std::vector<Edge*> backEdges;
    loop->getBackEdges(backEdges);
  }
}
