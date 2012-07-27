/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
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

// Implementations of Node iterators

#if !defined(DDG_ITERATOR_H)
#define DDG_ITERATOR_H

#include "Node.h"
#include "DepGraphNode.h"


namespace Dyninst {

namespace DepGraphAPI {

class FormalParamSetIter : public NodeIteratorImpl {
 public:
    virtual void inc() { ++internal_; }
    virtual void dec() { --internal_; }
    virtual Node::Ptr get() { return *internal_; }
    virtual bool equals(NodeIteratorImpl *rhs) {
        FormalParamSetIter *tmp = dynamic_cast<FormalParamSetIter *>(rhs);
        if (tmp == NULL) return false;
        return internal_ == tmp->internal_;
    }

    virtual NodeIteratorImpl *copy() {
        NodeIteratorImpl *tmp = new FormalParamSetIter(internal_);
        return tmp;
    }

    virtual ~FormalParamSetIter() {
        // Nothing to do
    }
    
    FormalParamSetIter(const std::set<FormalParamNode::Ptr>::iterator iter) : internal_(iter) {};

 private:
    std::set<FormalParamNode::Ptr>::iterator internal_;
};

class FormalReturnSetIter : public NodeIteratorImpl {
 public:
    virtual void inc() { ++internal_; }
    virtual void dec() { --internal_; }
    virtual Node::Ptr get() { return *internal_; }
    virtual bool equals(NodeIteratorImpl *rhs) {
        FormalReturnSetIter *tmp = dynamic_cast<FormalReturnSetIter *>(rhs);
        if (tmp == NULL) return false;
        return internal_ == tmp->internal_;
    }

    virtual NodeIteratorImpl *copy() {
        return new FormalReturnSetIter(internal_);
    }

    virtual ~FormalReturnSetIter() {
        // Nothing to do
    }
    
    FormalReturnSetIter(const std::set<FormalReturnNode::Ptr>::iterator iter) : internal_(iter) {};

 private:
    std::set<FormalReturnNode::Ptr>::iterator internal_;
};

class DDGEntryIter : public NodeIteratorImpl {
 public:
    virtual void inc() {
        if (paramsCur_ == paramsEnd_) {
            assert(virtualsCur_ != virtualsEnd_);
            ++virtualsCur_;
        }
        else {
            ++paramsCur_;
        }
    }
    virtual void dec() {
        if (virtualsCur_ == virtualsBegin_) {
            assert(paramsCur_ != paramsBegin_); 
            --paramsCur_;
        }
        else {
            --virtualsCur_;
        }
    }
            
    virtual Node::Ptr get() {
        if (paramsCur_ == paramsEnd_) {
            assert(virtualsCur_ != virtualsEnd_);
            return *virtualsCur_;
        }
        else {
            return *paramsCur_;
        }
    }

    virtual bool equals(NodeIteratorImpl *rhs) {
        DDGEntryIter *tmp = dynamic_cast<DDGEntryIter *>(rhs);
        if (tmp == NULL) return false;
        
        return ((paramsBegin_ == tmp->paramsBegin_) &&
                (paramsCur_ == tmp->paramsCur_) &&
                (paramsEnd_ == tmp->paramsEnd_) &&
                (virtualsBegin_ == tmp->virtualsBegin_) &&
                (virtualsCur_ == tmp->virtualsCur_) &&
                (virtualsEnd_ == tmp->virtualsEnd_));
    }
    
    virtual NodeIteratorImpl *copy() {
        NodeIteratorImpl *tmp =  new DDGEntryIter(paramsBegin_, paramsCur_, paramsEnd_,
                                                  virtualsBegin_, virtualsCur_, virtualsEnd_);
        return tmp;
    }
    
    virtual ~DDGEntryIter() {
        // Nothing to do
    }
    
    DDGEntryIter(NodeIterator &paramsBegin, 
                 NodeIterator &paramsCur, 
                 NodeIterator &paramsEnd,
                 NodeIterator &virtualsBegin, 
                 NodeIterator &virtualsCur, 
                 NodeIterator &virtualsEnd) :
        paramsBegin_(paramsBegin),
        paramsCur_(paramsCur),
        paramsEnd_(paramsEnd),
        virtualsBegin_(virtualsBegin),
        virtualsCur_(virtualsCur),
        virtualsEnd_(virtualsEnd)
        {};

 private:
    // The set of parameter nodes...
    NodeIterator paramsBegin_;
    NodeIterator paramsCur_;
    NodeIterator paramsEnd_;

    // The next thing we want is the children (if any)
    // of the virtual node. That's, amusingly, a NodeIterator
    // over virtualNode.outs...
    NodeIterator virtualsBegin_;
    NodeIterator virtualsCur_;
    NodeIterator virtualsEnd_;
};


class DDGExitIter : public NodeIteratorImpl {
 public:
    virtual void inc() {
        if (returnsCur_ == returnsEnd_) {
            assert(virtualsCur_ != virtualsEnd_);
            ++virtualsCur_;
        }
        else {
            ++returnsCur_;
        }
    }
    virtual void dec() {
        if (virtualsCur_ == virtualsBegin_) {
            assert(returnsCur_ != returnsBegin_); 
            --returnsCur_;
        }
        else {
            --virtualsCur_;
        }
    }
            
    virtual Node::Ptr get() {
        if (returnsCur_ == returnsEnd_) {
            assert(virtualsCur_ != virtualsEnd_);
            return *virtualsCur_;
        }
        else {
            return *returnsCur_;
        }
    }

    virtual bool equals(NodeIteratorImpl *rhs) {
        DDGExitIter *tmp = dynamic_cast<DDGExitIter *>(rhs);
        if (tmp == NULL) return false;
        
        return ((returnsBegin_ == tmp->returnsBegin_) &&
                (returnsCur_ == tmp->returnsCur_) &&
                (returnsEnd_ == tmp->returnsEnd_) &&
                (virtualsBegin_ == tmp->virtualsBegin_) &&
                (virtualsCur_ == tmp->virtualsCur_) &&
                (virtualsEnd_ == tmp->virtualsEnd_));
    }
    
    virtual NodeIteratorImpl *copy() {
        NodeIteratorImpl *tmp =  new DDGExitIter(returnsBegin_, returnsCur_, returnsEnd_,
                                                  virtualsBegin_, virtualsCur_, virtualsEnd_);
        return tmp;
    }
    
    virtual ~DDGExitIter() {
        // Nothing to do
    }
    
    DDGExitIter(NodeIterator &returnsBegin, 
                 NodeIterator &returnsCur, 
                 NodeIterator &returnsEnd,
                 NodeIterator &virtualsBegin, 
                 NodeIterator &virtualsCur, 
                 NodeIterator &virtualsEnd) :
        returnsBegin_(returnsBegin),
        returnsCur_(returnsCur),
        returnsEnd_(returnsEnd),
        virtualsBegin_(virtualsBegin),
        virtualsCur_(virtualsCur),
        virtualsEnd_(virtualsEnd)
        {};

 private:
    // The set of parameter nodes...
    NodeIterator returnsBegin_;
    NodeIterator returnsCur_;
    NodeIterator returnsEnd_;

    // The next thing we want is the children (if any)
    // of the virtual node. That's, amusingly, a NodeIterator
    // over virtualNode.outs...
    NodeIterator virtualsBegin_;
    NodeIterator virtualsCur_;
    NodeIterator virtualsEnd_;
};

}
}


#endif
