
// A representation of a variable x = x + var1 + var2 + var3 + ...
// where int is an integer and var1...varN are unknown variables.

#include <map>

template <typename T>
struct Var {
  typedef typename std::map<T, int> Unknowns;
  Var<T> &operator+=(const Var<T> &rhs) {
    x += rhs.x;
    for (typename Unknowns::const_iterator i = rhs.unknowns.begin();
	 i != rhs.unknowns.end(); ++i) {
      unknowns[i->first] += i->second;
    }
    return *this;
  }

  Var<T> &operator+=(const int &rhs) {
    x += rhs;
    return *this;
  }

  Var<T> operator+(const Var<T> &rhs) const {
    Var<T> tmp = *this;
    tmp += rhs;
    return tmp;
  }

  Var<T> operator+(const int &rhs) const {
    Var<T> tmp = *this;
    tmp += rhs;
    return tmp;
  }

  Var<T> operator*=(const int &rhs) {
    if (rhs == 0) { 
      unknowns.clear();
      x = 0;
    } 
    else if (rhs != 1) {
      x *= rhs;
      for (typename Unknowns::iterator i = unknowns.begin();
	   i != unknowns.end(); ++i) {
	i->second *= rhs;
      }
    }
    return *this;
  }

  Var<T> operator*(const int &rhs) const {
    if (rhs == 0) { return Var<T>(0); }
    if (rhs == 1) return *this;
    else {
      Var<T> tmp = *this;
      tmp *= rhs;
      return tmp;
    }
  }


  bool operator==(const Var<T> &rhs) const {
    if (x != rhs.x) return false;
    if (unknowns != rhs.unknowns) return false;
    return true;
  }
  
  bool operator!=(const Var<T> &rhs) const {
    return !(*this == rhs);
  }

  bool operator!=(const int &rhs) const {
    if (x != rhs) return true;
    if (!unknowns.empty()) return true;
    return false;
  }

Var() : x(0) {};
Var(int a) : x(a) {};
Var(T a) : x(0) { unknowns[a] = 1; };

  int x;
  Unknowns unknowns;
};

template <typename T> 
struct linVar {
  linVar<T> &operator+=(const linVar<T> &rhs) {
    if (bottom) return *this;
    if (rhs.bottom) {
      bottom = true;
    }
    else {
      a += rhs.a;
      b += rhs.b;
    }
    return *this;
  }
  const linVar<T> operator+(const linVar<T> &rhs) const {
    if (bottom) return *this;
    if (rhs.bottom) return rhs;
    return linVar(a + rhs.a, b + rhs.b);
  }
  const linVar<T> operator*=(const int &rhs) {
    a *= rhs;
    b *= rhs;
    return *this;
  }

  const linVar<T> operator*=(const linVar<T> &rhs) {
    if (bottom) return *this;
    if (rhs.bottom) {
      bottom = true;
    }
    else {
      if (b && rhs.b) {
	// Can't go quadratic
	bottom = true;
      }
      else {
	a *= rhs.a;
	b = a*rhs.b + b*rhs.a;
      }
    }
    return *this;
  }
  const linVar<T> operator*(const linVar<T> &rhs) const {
    if (bottom) return *this;
    if (rhs.bottom) return rhs;
    if ((b != 0) && (rhs.b != 0)) {
      return linVar<T>();
    }
    // Okay, I'm going to be lazy here because I don't believe it
    // matters...
    // Not sure how to multiply two sets of registers, so we'll force
    // one side to be numeric.
    if (b == 0) {
      // Just have a * rhs.b to worry about
      if (a.unknowns.empty()) {
	return linVar<T>(a + rhs.a, rhs.b*a.x);
      }
      else if (rhs.b.unknowns.empty()) {
	return linVar<T>(a + rhs.a, a*rhs.b.x);
      }
      else { 
	return linVar<T>();
      }
    }
    else if (rhs.b == 0) {
      if (b.unknowns.empty()) {
	return linVar<T>(a + rhs.a, rhs.a*b.x);
      }
      else if (rhs.a.unknowns.empty()) {
	return linVar<T>(a + rhs.a, b*rhs.a.x);
      }
      else { 
	return linVar<T>();
      }
    }
    return linVar<T>();
  }
  
linVar() : bottom(true) {};
linVar(T x, T y) : bottom(false), a(x), b(y) {};
linVar(int x, int y) : bottom(false), a(x), b(y) {};
linVar(T x, int y): bottom(false), a(x), b(y) {};
linVar(Var<T> x, Var<T> y) : bottom(false), a(x), b(y) {};
  bool bottom;
  Var<T> a;
  Var<T> b;
};


