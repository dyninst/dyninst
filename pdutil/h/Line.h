/************************************************************************
 * Line.h: line number information.
************************************************************************/





#if !defined(_Line_h_)
#define _Line_h_

#include <iostream.h>



/************************************************************************
 * class Line
************************************************************************/

class Line {
public:
     Line ();
     Line (unsigned);
    ~Line ();

    bool operator== (const Line &) const;
};

inline
Line::Line() {
}

inline
Line::Line(unsigned) {
}

inline
Line::~Line() {
}

inline
bool
Line::operator==(const Line& l) const {
    return true;
}

inline
ostream&
operator<<(ostream& os, const Line& l) {
    return os << "no line number information" << endl;
}





#endif /* !defined(_Line_h_) */
