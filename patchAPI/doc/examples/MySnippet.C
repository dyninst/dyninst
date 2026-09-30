class MySnippet : public Snippet {
public:
  virtual bool generate(Point* pt, Buffer& buf)
  {
    // Generate and store binary code in the Buffer buf
    return true;
  }
};
MySnippet::Ptr snippet = MySnippet::create(new MySnippet);
