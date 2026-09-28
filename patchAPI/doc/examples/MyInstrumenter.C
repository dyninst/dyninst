class MyInstrumenter : public Instrumenter {
public:
  virtual bool run()
  {
    // Specify how to install instrumentation
    return true;
  }
};
