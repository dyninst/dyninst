/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef ITERATORS_H__
#define ITERATORS_H__

/*
 * An iterator and a predicate interface, and a 
 * ContainerWrapper that can provide a forward
 * predicate iterator for any container that exports
 * a begin() and end()
 */
 
namespace Dyninst {
namespace ParseAPI {

/*** a simple forward iterator ***/
template <
    typename DERIVED,
    typename VALUE,
    typename REFERENCE = VALUE &
>
class iterator_base {
public:
    DERIVED&    operator++();
    DERIVED     operator++(int);
    bool        operator==(const DERIVED &) const;
    bool        operator!=(const DERIVED &) const;
    REFERENCE   operator*() const;
};

/*** A predicate interface ***/
template <
    typename DERIVED,
    typename VALUE,
    typename REFERENCE = VALUE &
>
class iterator_predicate {
public:
    bool    operator()(REFERENCE) const;
};

 // A default predicate type
template<typename V, typename R = V&>
class default_predicate
    : public iterator_predicate<
        default_predicate<V,R>,
        V,
        R
      >
{
 private:
    bool pred_impl(R) const { return true; }
    friend class iterator_predicate<default_predicate<V,R>,V,R>;
};

/*** Container wrapper and iterators for predicate containers ***/
template <
    typename CONTAINER,
    typename VALUE,
    typename REFERENCE,
    typename PREDICATE
>
class ContainerWrapper;

template <
    typename CONTAINER,
    typename VALUE,
    typename REFERENCE = VALUE &,
    typename PREDICATE = class default_predicate<VALUE,REFERENCE>
>
class PredicateIterator 
    : public iterator_base<
        PredicateIterator<CONTAINER,VALUE,REFERENCE,PREDICATE>,
        VALUE,
        REFERENCE
      >
{
 public:
    typedef ContainerWrapper<CONTAINER,VALUE,REFERENCE,PREDICATE> wrapper_t;

    PredicateIterator() : 
        m_cont_(NULL), 
        m_pred_(NULL), 
        m_init_(false) 
    { }
    PredicateIterator(const wrapper_t * cw) :
        m_cont_(cw),
        m_pred_(NULL),
        m_cur_(cw->m_container_.begin()),
        m_init_(true)
    { }
    PredicateIterator(const wrapper_t * cw, PREDICATE * p) :
        m_cont_(cw),
        m_pred_(p),
        m_cur_(cw->m_container_.begin()),
        m_init_(true)
    { }

 private:
    void        increment();
    REFERENCE   dereference() const;
    bool        equal(PredicateIterator const& o) const;

    bool        uninit() const { return !m_init_; }

    const wrapper_t *                   m_cont_;
    PREDICATE *                         m_pred_;
    typename CONTAINER::const_iterator  m_cur_;
    bool                                m_init_;

    friend class iterator_base<PredicateIterator,VALUE,REFERENCE>;
};


template <
    typename CONTAINER,
    typename VALUE,
    typename REFERENCE = VALUE &,
    typename PREDICATE = default_predicate<VALUE,REFERENCE>
>
class ContainerWrapper
{
 public:
    typedef PredicateIterator<CONTAINER,VALUE,REFERENCE,PREDICATE> iterator;

    PARSER_EXPORT ContainerWrapper(CONTAINER & cont) : m_container_(cont) { }
    PARSER_EXPORT ~ContainerWrapper() { }

    iterator        begin() const;
    iterator        begin(PREDICATE * p) const;
    iterator const& end() const;
    size_t          size() const;
    bool            empty() const;
 private:
    CONTAINER const& m_container_;
    iterator         m_end_;

    friend class PredicateIterator<CONTAINER,VALUE,REFERENCE,PREDICATE>;
};

/*** implementation details ***/

template<typename C,typename V,typename R,typename P>
inline PredicateIterator<C,V,R,P>
ContainerWrapper<C,V,R,P>::begin() const
{
    return iterator(this);
}

template<typename C,typename V,typename R,typename P>
inline PredicateIterator<C,V,R,P>
ContainerWrapper<C,V,R,P>::begin(P * p) const
{
    iterator ret(this,p);
    if(ret != m_end_ && !(*p)(*ret))
        ++ret;
    return ret;
}
template<typename C,typename V,typename R,typename P>
inline PredicateIterator<C,V,R,P> const&
ContainerWrapper<C,V,R,P>::end() const
{
    return m_end_;
}
template<typename C,typename V,typename R,typename P>
inline size_t
ContainerWrapper<C,V,R,P>::size() const
{
    return m_container_.size();
}
template<typename C,typename V,typename R,typename P>
inline bool
ContainerWrapper<C,V,R,P>::empty() const
{
    return m_container_.empty();
}

template<typename C,typename V,typename R, typename P>
inline void PredicateIterator<C,V,R,P>::increment()
{
    if(*this == m_cont_->end())
        return;

    do {
        (void)++m_cur_;
    } while(*this != m_cont_->end() && (m_pred_ && !(*m_pred_)(*(*this))) );
}

template<typename C,typename V,typename R, typename P>
inline R PredicateIterator<C,V,R,P>::dereference() const
{
    return *m_cur_;
}

template<typename C,typename V,typename R, typename P>
inline bool PredicateIterator<C,V,R,P>::equal(PredicateIterator const& o) const
{
    if(o.uninit() && uninit())
        return true;
    return (o.uninit() && m_cur_ == m_cont_->m_container_.end()) ||
           (m_cont_ == o.m_cont_ && m_cur_ == o.m_cur_);
}

/*** static binding implementation ***/

template<typename DERIVED,typename VALUE,typename REFERENCE>
inline DERIVED& iterator_base<DERIVED,VALUE,REFERENCE>::operator++()
{
    static_cast<DERIVED*>(this)->increment();
    return *static_cast<DERIVED*>(this);
}

template<typename DERIVED,typename VALUE,typename REFERENCE>
inline DERIVED iterator_base<DERIVED,VALUE,REFERENCE>::operator++(int)
{
    DERIVED copy(*static_cast<DERIVED*>(this));
    static_cast<DERIVED*>(this)->increment();
    return copy;
}

template<typename DERIVED,typename VALUE,typename REFERENCE>
inline bool iterator_base<DERIVED,VALUE,REFERENCE>::operator==(const DERIVED & o) const
{
    return static_cast<const DERIVED*>(this)->equal(o);
}

template<typename DERIVED,typename VALUE,typename REFERENCE>
inline bool iterator_base<DERIVED,VALUE,REFERENCE>::operator!=(const DERIVED & o) const
{
    return !static_cast<const DERIVED*>(this)->equal(o);
}

template<typename DERIVED,typename VALUE,typename REFERENCE>
inline REFERENCE iterator_base<DERIVED,VALUE,REFERENCE>::operator*() const
{
    return static_cast<const DERIVED*>(this)->dereference();
}

template<typename DERIVED,typename VALUE,typename REFERENCE>
inline bool iterator_predicate<DERIVED,VALUE,REFERENCE>::operator()(REFERENCE o) const
{
    return static_cast<const DERIVED*>(this)->pred_impl(o);
}

}
}

#endif
