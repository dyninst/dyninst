#ifndef __libthread_predicate_h__
#define __libthread_predicate_h__

template<class Subject>
class predicate {
  public:
    virtual bool satisfied_by(Subject subject);
    virtual ~predicate()  { }
};

template<class Subject>
class always_true_predicate : public predicate<Subject> {
  public:
    virtual bool satisfied_by(Subject subject);
    virtual ~always_true_predicate()  { }
};

template<class Subject>
class always_false_predicate : public predicate<Subject> {
  public:
    virtual bool satisfied_by(Subject subject);
    virtual ~always_false_predicate() { }
};

template<class Subject>
class and_predicate_composition : public predicate<Subject> {
  private:
    predicate<Subject>* pred1;
    predicate<Subject>* pred2;    
  public:
    and_predicate_composition() :
        pred1(0), pred2(0) {}

    and_predicate_composition(predicate<Subject>* p1, predicate<Subject>* p2) :
        pred1(p1), pred2(p2) {}

    void set_pred1(predicate<Subject>* p1) {
        pred1 = p1;
    }
    
    void set_pred2(predicate<Subject>* p2) {
        pred2 = p2;
    }
    
    virtual bool satisfied_by(Subject subject);
    virtual ~and_predicate_composition() { }
};


template<class Subject>
class or_predicate_composition : public predicate<Subject> {
  private:
    predicate<Subject>* pred1;
    predicate<Subject>* pred2;    
  public:
    or_predicate_composition(predicate<Subject>* p1, predicate<Subject>* p2) :
        pred1(p1), pred2(p2) {}
    
    virtual bool satisfied_by(Subject subject);
    virtual ~or_predicate_composition() { }
};


template <class Subject>
bool predicate<Subject>::satisfied_by(Subject subject) {
    return false;
}

template <class Subject>
bool always_true_predicate<Subject>::satisfied_by(Subject subject) {
    return true;
}

template <class Subject>
bool always_false_predicate<Subject>::satisfied_by(Subject subject) {
    return false;
}

template <class Subject>
bool and_predicate_composition<Subject>::satisfied_by(Subject subject) {
    return
        pred1->satisfied_by(subject) && pred2->satisfied_by(subject);
}

template <class Subject>
bool or_predicate_composition<Subject>::satisfied_by(Subject subject) {
    return
        pred1->satisfied_by(subject) && pred2->satisfied_by(subject);
}
#endif /* __libthread_predicate_h__ */
