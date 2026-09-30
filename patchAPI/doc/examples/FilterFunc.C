template <class T>
      class FilterFunc {
public:
  bool operator()(Point::Type type, Location loc, T arg)
  {
    // The logic to check whether this point is what we need
    return true;
  }
};
