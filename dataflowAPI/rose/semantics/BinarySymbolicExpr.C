#define __STDC_LIMIT_MACROS

//#include "sage3basic.h"
#include "../util/StringUtility.h"

#include "BinarySymbolicExpr.h"
#include "SMTSolver.h"
//#include "stringify.h"
#include "../integerOps.h"
#include "../util/Combinatorics.h"

#include <boost/foreach.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

namespace rose {
namespace BinaryAnalysis {
namespace SymbolicExpr {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Supporting functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A mutex that's used by various methods in this namespace
static boost::mutex symbolicExprMutex;

// Returns the next name counter. If @p useThis is specified then return that value and make sure the next value to be
// returned is larger.
static uint64_t
nextNameCounter(uint64_t useThis = (uint64_t)(-1)) {
    static boost::mutex mutex;
    static uint64_t counter = 0;
    boost::lock_guard<boost::mutex> lock(mutex);
    if (useThis == (uint64_t)(-1))
        return ++counter;
    counter = std::max(counter, useThis);
    return useThis;
}

const uint64_t
MAX_NNODES = UINT64_MAX;

std::string
toStr(Operator o) {
    char buf[64];
    //FIXME
    //std::string s = 1fyBinaryAnalysisSymbolicExprOperator(o, "OP_");
    std::string s = "";
    ASSERT_require(s.size()<sizeof buf);
    strcpy(buf, s.c_str());
    for (char *s=buf; *s; s++) {
        if ('_'==*s) {
            *s = '-';
        } else {
            *s = tolower(*s);
        }
    }
    return buf;
}

// Escape control and non-printing characters
static std::string
escapeCharacter(char ch) {
    switch (ch) {
        case '\a': return "\\a";
        case '\b': return "\\b";
        case '\t': return "\\t";
        case '\n': return "\\n";
        case '\v': return "\\v";
        case '\f': return "\\f";
        case '\r': return "\\r";
        default:
            if (!isprint(ch)) {
                char buf[16];
                sprintf(buf, "\\%03o", (unsigned)ch);
                return buf;
            }
            return std::string(1, ch);
    }
}

// Escape a string when it appears where a symbol name could appear.
static std::string
nameEscape(const std::string &s) {
    std::string retval;
    BOOST_FOREACH (char ch, s) {
        switch (ch) {
            case '(':
            case ')':
            case '[':
            case ']':
            case '<':
            case '>':
                retval += std::string("\\") + ch;
                break;
            default:
                retval += escapeCharacter(ch);
                break;
        }
    }
    return retval;
}

// Escape text that appears inside a "<...>" style comment.
static std::string
commentEscape(const std::string &s) {
    std::string retval;

    // Escape angle brackets if they're not balanced.  We could be smarter and escape only the unbalanced angle brackets. We'll
    // leave that as an exercise for the reader. ;-)
    bool escapeAngleBrackets = false;
    int angleBracketDepth = 0;
    BOOST_FOREACH (char ch, s) {
        if ('<' == ch) {
            ++angleBracketDepth;
        } else if ('>' == ch && --angleBracketDepth < 0) {
            escapeAngleBrackets = true;
            break;
        }
    }

    BOOST_FOREACH (char ch, s) {
        switch (ch) {
            case '<':
            case '>':
                if (escapeAngleBrackets) {
                    retval += std::string("\\") + ch;
                } else {
                    retval += ch;
                }
                break;
            default:
                retval += ch;
                break;
        }
    }
    return retval;
}

bool
ExpressionLessp::operator()(const Ptr &a, const Ptr &b) const {
    if (a == NULL || b == NULL)
        return a == NULL && b != NULL;
    return a->hash() < b->hash();
}

Ptr
setToIte(const Ptr &set) {
    ASSERT_not_null(set);
    InteriorPtr iset = set->isInteriorNode();
    if (!iset || iset->getOperator() != OP_SET)
        return set;
    ASSERT_require(iset->nChildren() >= 1);
    Ptr condVar = makeVariable(32);
    Ptr retval;
    for (size_t i=iset->nChildren(); i>0; --i) {
        Ptr member = iset->child(i-1);
        if (!retval) {
            retval = member;
        } else {
            Ptr cond = makeEq(condVar, makeInteger(32, i));
            retval = makeIte(cond, member, retval);
        }
    }
    return retval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Base node
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ptr
Node::newFlags(unsigned newFlags) {
    if (newFlags == flags_)
        return sharedFromThis();
    if (InteriorPtr inode = isInteriorNode())
        return Interior::create(0, inode->getOperator(), inode->children(), comment(), newFlags);
    LeafPtr lnode = isLeafNode();
    ASSERT_not_null(lnode);
    if (lnode->isNumber())
        return makeConstant(lnode->bits(), comment(), newFlags);
    if (lnode->isVariable())
        return makeVariable(nBits(), comment(), newFlags);
    if (lnode->isMemory())
        return makeMemory(domainWidth(), nBits(), comment(), newFlags);
    ASSERT_not_reachable("invalid leaf node type");
}
    
std::set<LeafPtr>
Node::getVariables() {
    struct T1: public Visitor {
        std::set<LeafPtr> vars;
        VisitAction preVisit(const Ptr&) {
            return CONTINUE;
        }
        VisitAction postVisit(const Ptr &node) {
            LeafPtr l_node = node->isLeafNode();
            if (l_node && !l_node->isNumber())
                vars.insert(l_node);
            return CONTINUE;
        }
    } t1;
    depthFirstTraversal(t1);
    return t1.vars;
}

struct Hasher: Visitor {
    virtual VisitAction preVisit(const Ptr &node)  {
        return 0 == node->isHashed() ? CONTINUE : TRUNCATE;
    }

    virtual VisitAction postVisit(const Ptr &node)  {
        if (!node->isHashed()) {                        // probably true, but some other thread may have beaten us here.
            uint64_t h = hash(hash(node->domainWidth(), node->nBits()), node->flags());
            if (LeafPtr leaf = node->isLeafNode()) {
                if (leaf->isNumber()) {
                    if (leaf->nBits() <= 64) {
                        h = hash(h, leaf->toInt());
                    } else {
                        for (size_t i=0; i<leaf->nBits(); i+=64) {
                            typedef Sawyer::Container::BitVector::BitRange BitRange;
                            size_t j = std::min(i+64, leaf->nBits());
                            h = hash(h, leaf->bits().toInteger(BitRange::hull(i, j-1)));
                        }
                    }
                } else {
                    // It's okay to not hash whether the leaf is a variable or memory because variables always have a zero
                    // domain width and memory has a non-zero width, which is already incorporated into the hash from above.
                    h = hash(h, leaf->nameId());
                }
            } else {
                InteriorPtr inode = node->isInteriorNode();
                ASSERT_not_null(inode);
                h = hash(h, inode->getOperator());
                BOOST_FOREACH (const Ptr &child, inode->children()) {
                    ASSERT_require(child->isHashed());
                    h = hash(h, child->hash());
                }
            }
            node->hash(h);
        }
        return CONTINUE;
    }

    // Incorporates data into the existing hash, h, and returns a new hash. This is no particular well-known algorithm, but
    // testing showed that it gives pretty well-distributed results for close values, particularly when called on two or more
    // pieces of data.
    uint64_t hash(uint64_t h, uint64_t data) {
        for (size_t i=0; i<64-6; i += 6) {
            unsigned sa = ((data >> i) ^ h ^ i) & 0x3f;
            h = (h >> (64-sa)) | (h << sa);
            h ^= data;
        }
        return h;
    }
};

uint64_t
Node::hash() {
    if (0==hashval_) {
        Hasher hasher;
        depthFirstTraversal(hasher);
    }
    boost::unique_lock<boost::mutex> lock(symbolicExprMutex);
    return hashval_;
}

void
Node::hash(uint64_t h) {
    boost::unique_lock<boost::mutex> lock(symbolicExprMutex);
    hashval_ = h;
}

void
Node::assertAcyclic() {
#ifndef NDEBUG
    struct T1: Visitor {
        std::vector<const Node*> ancestors;
        VisitAction preVisit(const Ptr &node) {
            ASSERT_require(std::find(ancestors.begin(), ancestors.end(), getRawPointer(node))==ancestors.end());
            ancestors.push_back(getRawPointer(node));
            return CONTINUE;
        }
        VisitAction postVisit(const Ptr &node) {
            ASSERT_require(!ancestors.empty() && ancestors.back()==getRawPointer(node));
            ancestors.pop_back();
            return CONTINUE;
        }
    } t1;
    depthFirstTraversal(t1);
#endif
}

uint64_t
Node::nNodesUnique() {
    std::vector<Ptr> exprs(1, sharedFromThis());
    return SymbolicExpr::nNodesUnique(exprs.begin(), exprs.end());
}

std::vector<Ptr>
Node::findCommonSubexpressions() {
    return SymbolicExpr::findCommonSubexpressions(std::vector<Ptr>(1, sharedFromThis()));
}

void
Node::printFlags(std::ostream &o, unsigned flags, char &bracket) {
    if ((flags & INDETERMINATE) != 0) {
        o <<bracket <<"indet";
        bracket = ',';
        flags &= ~INDETERMINATE;
    }
    if ((flags & UNSPECIFIED) != 0) {
        o <<bracket <<"unspec";
        bracket = ',';
        flags &= ~UNSPECIFIED;
    }
    if ((flags & BOTTOM) != 0) {
        o <<bracket <<"bottom";
        bracket = ',';
        flags &= ~BOTTOM;
    }
    if (flags != 0) {
        o <<bracket <<"f=" <<std::hex <<flags <<std::dec;
        bracket = ',';
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Interior node
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Interior::Interior(size_t nbits, Operator op, const Ptr &a, const std::string &comment, unsigned flags)
    : Node(comment), op_(op), nnodes_(1) {
    addChild(a);
    adjustWidth();
    adjustBitFlags(flags);
    if (nbits != 0 && nbits != nBits()) {
        throw Exception("operator size mismatch (specified=" + StringUtility::numberToString(nbits) +
                        ", actual=" + StringUtility::numberToString(nBits()) + ")");
    }
}

Interior::Interior(size_t nbits, Operator op, const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags)
    : Node(comment), op_(op), nnodes_(1) {
    addChild(a);
    addChild(b);
    adjustWidth();
    adjustBitFlags(flags);
    if (nbits != 0 && nbits != nBits()) {
        throw Exception("operator size mismatch (specified=" + StringUtility::numberToString(nbits) +
                        ", actual=" + StringUtility::numberToString(nBits()) + ")");
    }
}

Interior::Interior(size_t nbits, Operator op, const Ptr &a, const Ptr &b, const Ptr &c, const std::string &comment,
                   unsigned flags)
    : Node(comment), op_(op), nnodes_(1) {
    addChild(a);
    addChild(b);
    addChild(c);
    adjustWidth();
    adjustBitFlags(flags);
    if (nbits != 0 && nbits != nBits()) {
        throw Exception("operator size mismatch (specified=" + StringUtility::numberToString(nbits) +
                        ", actual=" + StringUtility::numberToString(nBits()) + ")");
    }
}

Interior::Interior(size_t nbits, Operator op, const Nodes &children, const std::string &comment, unsigned flags)
    : Node(comment), op_(op), nnodes_(1) {
    for (size_t i=0; i<children.size(); ++i)
        addChild(children[i]);
    adjustWidth();
    adjustBitFlags(flags);
    if (nbits != 0 && nbits != nBits()) {
        throw Exception("operator size mismatch (specified=" + StringUtility::numberToString(nbits) +
                        ", actual=" + StringUtility::numberToString(nBits()) + ")");
    }
}

void
Interior::addChild(const Ptr &child)
{
    ASSERT_not_null(child);
    children_.push_back(child);
    if (nnodes_ != MAX_NNODES) {
        if (nnodes_ + child->nNodes() < nnodes_) {
            nnodes_ = MAX_NNODES;                       // overflow
        } else {
            nnodes_ += child->nNodes();
        }
    }
}

void
Interior::adjustWidth() {
    if (children_.empty())
        throw Exception(toStr(op_) + " operator requires argument(s)");
    switch (op_) {
        case OP_ASR:
        case OP_ROL:
        case OP_ROR:
        case OP_SHL0:
        case OP_SHL1:
        case OP_SHR0:
        case OP_SHR1: {
            if (nChildren() != 2)
                throw Exception(toStr(op_) + " operator expects two arguments");
            if (!child(0)->isScalar())
                throw Exception(toStr(op_) + " operator's first argument (shift amount) must be scalar");
            if (!child(1)->isScalar())
                throw Exception(toStr(op_) + " operator's second argument (value to shift) must be scalar");
            nBits_ = child(1)->nBits();
            domainWidth_ = 0;
            break;
        }
        case OP_CONCAT: {
            size_t totalWidth = 0;
            BOOST_FOREACH (const Ptr &child, children_) {
                if (!child->isScalar())
                    throw Exception(toStr(op_) + " operator's arguments must be scalar");
                totalWidth += child->nBits();
            }
            nBits_ = totalWidth;
            domainWidth_ = 0;
            break;
        }
        case OP_EQ:
        case OP_NE:
        case OP_SGE:
        case OP_SGT:
        case OP_SLE:
        case OP_SLT:
        case OP_UGE:
        case OP_UGT:
        case OP_ULE:
        case OP_ULT: {
            if (nChildren() != 2)
                throw Exception(toStr(op_) + " operator expects two arguments");
            if (child(0)->nBits() != child(1)->nBits())
                throw Exception(toStr(op_) + " operator's arguments must both be the same width");
            nBits_ = 1;
            domainWidth_ = 0;
            break;
        }
        case OP_EXTRACT: {
            if (nChildren() != 3)
                throw Exception(toStr(op_) + " operator expects three arguments");
            if (!child(0)->isNumber())
                throw Exception(toStr(op_) + " operator's first argument (begin bit) must be an integer constant");
            if (!child(1)->isNumber())
                throw Exception(toStr(op_) + " operator's second argument (end bit) must be an integer constant");
            if (!child(2)->isScalar())
                throw Exception(toStr(op_) + " operator's third argument must be scalar");
            if (child(0)->toInt() >= child(1)->toInt())
                throw Exception(toStr(op_) + " operator's first argument must be less than the second");
            size_t totalSize = child(1)->toInt() - child(0)->toInt();
            nBits_ = totalSize;
            domainWidth_ = 0;
            break;
        }
        case OP_ITE: {
            if (nChildren() != 3)
                throw Exception(toStr(op_) + " operator expects three arguments");
            if (!child(0)->isScalar())
                throw Exception(toStr(op_) + " operator's first argument (condition) must be scalar");
            if (child(0)->nBits() != 1)
                throw Exception(toStr(op_) + " operator's first argument (condition) must be Boolean");
            if (child(1)->nBits() != child(2)->nBits() || child(1)->domainWidth() != child(2)->domainWidth())
                throw Exception(toStr(op_) + " operator's second and third arguments must have equal width");
            nBits_ = child(1)->nBits();
            domainWidth_ = child(1)->domainWidth();
            break;
        }
        case OP_LSSB:
        case OP_MSSB:
        case OP_NEGATE: {
            if (nChildren() != 1)
                throw Exception(toStr(op_) + " operator expects one argument");
            if (!child(0)->isScalar())
                throw Exception(toStr(op_) + " argument must be scalar");
            nBits_ = child(0)->nBits();
            domainWidth_ = 0;
            break;
        }
        case OP_READ: {
            if (nChildren() != 2)
                throw Exception(toStr(op_) + " operator expects two arguments");
            if (child(0)->isScalar())
                throw Exception(toStr(op_) + " operator's first argument must be a memory state");
            if (!child(1)->isScalar())
                throw Exception(toStr(op_) + " operator's second argument (address) must be scalar");
            if (child(0)->domainWidth() != child(1)->nBits())
                throw Exception(toStr(op_) + " operator's arguments have mismatched sizes");
            nBits_ = child(0)->nBits();              // size of values stored in memory
            domainWidth_ = 0;
            break;
        }
        case OP_SDIV:
        case OP_UDIV: {
            if (nChildren() != 2)
                throw Exception(toStr(op_) + " operator expects two arguments");
            if (!child(0)->isScalar())
                throw Exception(toStr(op_) + " operator's first argument must be scalar");
            if (!child(1)->isScalar())
                throw Exception(toStr(op_) + " operator's second argument must be scalar");
            nBits_ = child(0)->nBits();
            domainWidth_ = 0;
            break;
        }
        case OP_SEXTEND:
        case OP_UEXTEND: {
            if (nChildren() != 2)
                throw Exception(toStr(op_) + " operator expects two arguments");
            if (!child(0)->isNumber())
                throw Exception(toStr(op_) + " operator's first argument (size) must be a numeric constant");
            if (!child(1)->isScalar())
                throw Exception(toStr(op_) + " operator's second argument must be scalar");
            nBits_ = child(0)->toInt();
            domainWidth_ = 0;
            break;
        }
        case OP_SMOD:
        case OP_UMOD: {
            if (nChildren() != 2)
                throw Exception(toStr(op_) + " operator expects two arguments");
            if (!child(0)->isScalar())
                throw Exception(toStr(op_) + " operator's first argument must be scalar");
            if (!child(1)->isScalar())
                throw Exception(toStr(op_) + " operator's second argument must be scalar");
            nBits_ = child(1)->nBits();
            domainWidth_ = 0;
            break;
        }
        case OP_SMUL:
        case OP_UMUL: {
            if (nChildren() < 1)
                throw Exception(toStr(op_) + " operator expects at least one argument");
            size_t totalWidth = 0;
            for (size_t i=0; i<nChildren(); ++i) {
                if (!child(i)->isScalar())
                    throw Exception(toStr(op_) + " operator's arguments must all be scalar");
                totalWidth += child(i)->nBits();
            }
            nBits_ = totalWidth;
            domainWidth_ = 0;
            break;
        }
        case OP_WRITE: {
            if (nChildren() != 3)
                throw Exception(toStr(op_) + " operator expects three arguments");
            if (child(0)->isScalar())
                throw Exception(toStr(op_) + " operator's first operand must be a memory state");
            if (!child(1)->isScalar())
                throw Exception(toStr(op_) + " operator's second argument (address) must be scalar");
            if (!child(2)->isScalar())
                throw Exception(toStr(op_) + " operator's third argument (value to write) must be scalar");
            if (child(1)->nBits() != child(0)->domainWidth())
                throw Exception(toStr(op_) + " operator's second argument (address) has incorrect width");
            if (child(2)->nBits() != child(0)->nBits())
                throw Exception(toStr(op_) + " operator's third argument (value to write) has incorrect width");
            nBits_ = child(0)->nBits();
            domainWidth_ = child(0)->domainWidth();
            break;
        }
        case OP_ZEROP: {
            if (nChildren() != 1)
                throw Exception(toStr(op_) + " operator expects one argument");
            if (!child(0)->isScalar())
                throw Exception(toStr(op_) + " operator's argument must be scalar");
            nBits_ = 1;
            domainWidth_ = 0;
            break;
        }
        default: {
            // All children must have the same width, which is the width of this expression. This is suitable for things like
            // bitwise operators, add, etc.
            if (!child(0)->isScalar())
                throw Exception(toStr(op_) + " operator's arguments must all be scalar");
            for (size_t i=1; i<nChildren(); ++i) {
                if (!child(i)->isScalar())
                    throw Exception(toStr(op_) + " operator's arguments must all be scalar");
                if (child(i)->nBits() != child(0)->nBits())
                    throw Exception(toStr(op_) + " operator's arguments must all have the same width");
            }
            nBits_ = child(0)->nBits();
            domainWidth_ = 0;
            break;
        }
    }
    ASSERT_require(nBits_ != 0);
}

void
Interior::adjustBitFlags(unsigned flags) {
    flags_ = flags;
    BOOST_FOREACH (const Ptr &child, children_)
        flags_ |= child->flags();
}

void
Interior::print(std::ostream &o, Formatter &fmt) {
    struct FormatGuard {
        Formatter &fmt;
        FormatGuard(Formatter &fmt): fmt(fmt) {
            ++fmt.cur_depth;
        }
        ~FormatGuard() {
            --fmt.cur_depth;
        }
    } formatGuard(fmt);

    o <<"(" <<toStr(op_);

    // The width of an operator is not normally too useful since it can also be inferred from the width of its operands, but we
    // print it anyway for the benefit of mere humans.
    char bracket = '[';
    if (fmt.show_width) {
        o <<bracket <<nBits_;
        bracket = ',';
    }
    if (fmt.show_flags)
        printFlags(o, flags(), bracket /*in,out*/);
    if (fmt.show_comments!=Formatter::CMT_SILENT && !comment_.empty()) {
        o <<bracket <<comment_;
        bracket = ',';
    }
    if (bracket != '[')
        o <<"]";

    // Print the operand list.
    if (fmt.max_depth!=0 && fmt.cur_depth>=fmt.max_depth && 0!=nChildren()) {
        o <<" ...";
    } else {
        for (size_t i=0; i<children_.size(); i++) {
            bool printed = false;
            LeafPtr child_leaf = children_[i]->isLeafNode();
            o <<" ";
            switch (op_) {
                case OP_ASR:
                case OP_ROL:
                case OP_ROR:
                case OP_UEXTEND:
                    if (0==i && child_leaf) {
                        child_leaf->printAsUnsigned(o, fmt);
                        printed = true;
                    }
                    break;

                case OP_EXTRACT:
                    if ((0==i || 1==i) && child_leaf) {
                        child_leaf->printAsUnsigned(o, fmt);
                        printed = true;
                    }
                    break;

                case OP_BV_AND:
                case OP_BV_OR:
                case OP_BV_XOR:
                case OP_CONCAT:
                case OP_UDIV:
                case OP_UGE:
                case OP_UGT:
                case OP_ULE:
                case OP_ULT:
                case OP_UMOD:
                case OP_UMUL:
                    if (child_leaf) {
                        child_leaf->printAsUnsigned(o, fmt);
                        printed = true;
                    }
                    break;

                default:
                    break;
            }

            if (!printed)
                children_[i]->print(o, fmt);
        }
    }
    o <<")";

    if (!comment().empty())
        o <<"<" <<commentEscape(comment()) <<">";
}

bool
Interior::mustEqual(const Ptr &other_, SMTSolver *solver/*NULL*/) {
    bool retval = false;
    if (this==getRawPointer(other_)) {
        retval = true;
    } else if (isEquivalentTo(other_)) {
        // This is probably faster than using an SMT solver. It also serves as the naive approach when an SMT solver
        // is not available.
        retval = true;
    } else if (solver) {
        Ptr assertion = makeNe(sharedFromThis(), other_);
        retval = SMTSolver::SAT_NO==solver->satisfiable(assertion); /*equal if there is no solution for inequality*/
    }
    return retval;
}

bool
Interior::mayEqual(const Ptr &other, SMTSolver *solver/*NULL*/) {
    bool retval = false;
    if (this==getRawPointer(other)) {
        return true;
    } else if (isEquivalentTo(other)) {
        // This is probably faster than using an SMT solver.  It also serves as the naive approach when an SMT solver
        // is not available.
        retval = true;
    } else if (solver) {
        Ptr assertion = makeEq(sharedFromThis(), other);
        retval = SMTSolver::SAT_YES==solver->satisfiable(assertion);
    }
    return retval;
}

int
Interior::compareStructure(const Ptr &other_) {
    InteriorPtr other = other_->isInteriorNode();
    if (this==getRawPointer(other)) {
        return 0;
    } else if (other==NULL) {
        return 1;                                       // leaf nodes < internal nodes
    } else if (op_ != other->op_) {
        return op_ < other->op_ ? -1 : 1;
    } else if (nBits() != other->nBits()) {
        return nBits() < other->nBits() ? -1 : 1;
    } else if (children_.size() != other->children_.size()) {
        return children_.size() < other->children_.size() ? -1 : 1;
    } else if (flags() != other->flags()) {
        return flags() < other->flags() ? -1 : 1;
    } else {
        // compare children
        ASSERT_require(children_.size()==other->children_.size());
        for (size_t i=0; i<children_.size(); ++i) {
            if (int cmp = children_[i]->compareStructure(other->children_[i]))
                return cmp;
        }
    }
    return 0;
}

bool
Interior::isEquivalentTo(const Ptr &other_) {
    bool retval = false;
    InteriorPtr other = other_->isInteriorNode();
    if (this==getRawPointer(other)) {
        retval = true;
    } else if (other==NULL || nBits()!=other->nBits() || flags()!=other->flags()) {
        retval = false;
    } else if (hashval_!=0 && other->hashval_!=0 && hashval_!=other->hashval_) {
        // Unequal hashvals imply non-equivalent expressions.  The converse is not necessarily true due to possible
        // collisions.
        retval = false;
    } else if (op_==other->op_ && children_.size()==other->children_.size()) {
        retval = true;
        for (size_t i=0; i<children_.size() && retval; ++i)
            retval = children_[i]->isEquivalentTo(other->children_[i]);
        // Cache hash values. There's no need to compute a hash value if we've determined that the two expressions are
        // equivalent because it wouldn't save us any work--two equal hash values doesn't necessarily mean that two expressions
        // are equivalent.  However, if we already know one of the hash values then we can cache that hash value in the other
        // expression too.
        if (retval) {
            if (hashval_!=0 && other->hashval_==0) {
                other->hashval_ = hashval_;
            } else if (hashval_==0 && other->hashval_!=0) {
                hashval_ = other->hashval_;
            } else {
                ASSERT_require(hashval_==other->hashval_);
            }
        } else {
#ifdef InsnInstructionExpr_USE_HASHES
            hashval_ = hash();
            other->hashval_ = other->hash();
#endif
        }
    }
    return retval;
}

Ptr
Interior::substitute(const Ptr &from, const Ptr &to) {
    ASSERT_require(from!=NULL && to!=NULL && from->nBits()==to->nBits());
    if (isEquivalentTo(from))
        return to;
    bool substituted = false;
    Nodes newnodes;
    for (size_t i=0; i<children_.size(); ++i) {
        if (children_[i]->isEquivalentTo(from)) {
            newnodes.push_back(to);
            substituted = true;
        } else {
            newnodes.push_back(children_[i]->substitute(from, to));
            if (newnodes.back()!=children_[i])
                substituted = true;
        }
    }
    if (!substituted)
        return sharedFromThis();
    return Interior::create(0, getOperator(), newnodes, comment());
}

VisitAction
Interior::depthFirstTraversal(Visitor &v) {
    Ptr self = sharedFromThis();
    VisitAction action = v.preVisit(self);
    if (CONTINUE==action) {
        for (std::vector<Ptr>::const_iterator ci=children_.begin(); ci!=children_.end(); ++ci) {
            action = (*ci)->depthFirstTraversal(v);
            if (TERMINATE==action)
                break;
        }
    }
    if (TERMINATE!=action)
        action = v.postVisit(self);
    return action;
}

InteriorPtr
Interior::associative() {
    Nodes newOperands;
    std::list<Ptr> worklist(children_.begin(), children_.end());
    bool modified = false;
    while (!worklist.empty()) {
        Ptr child = worklist.front();
        worklist.pop_front();
        InteriorPtr ichild = child->isInteriorNode();
        if (ichild && ichild->op_ == op_) {
            worklist.insert(worklist.begin(), ichild->children_.begin(), ichild->children_.end());
            modified = true;
        } else {
            newOperands.push_back(child);
        }
    }
    if (!modified)
        return isInteriorNode();

    // Return the new expression without simplifying it again.
    return InteriorPtr(new Interior(nBits(), op_, newOperands, comment()));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Simplification
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// compare expressions for sorting operands of commutative operators. Returns -1, 0, 1
static int
expr_cmp(const Ptr &a, const Ptr &b)
{
    ASSERT_not_null(a);
    ASSERT_not_null(b);
    InteriorPtr ai = a->isInteriorNode();
    InteriorPtr bi = b->isInteriorNode();
    LeafPtr al = a->isLeafNode();
    LeafPtr bl = b->isLeafNode();
    ASSERT_require((ai!=NULL) ^ (al!=NULL));
    ASSERT_require((bi!=NULL) ^ (bl!=NULL));

    if (a == b) {
        return 0;
    } else if ((ai==NULL) != (bi==NULL)) {
        // internal nodes are less than leaf nodes
        return ai!=NULL ? -1 : 1;
    } else if (al!=NULL) {
        // both are leaf nodes
        ASSERT_not_null(bl);
        if (al->isNumber() != bl->isNumber()) {
            // constants are greater than variables
            return al->isNumber() ? 1 : -1;
        } else if (al->isNumber()) {
            // both are constants, sort by unsigned value
            ASSERT_require(bl->isNumber());
            return al->bits().compare(bl->bits());
        } else if (al->isVariable() != bl->isVariable()) {
            // variables are less than memory
            return al->isVariable() ? -1 : 1;
        } else {
            // both are variables or both are memory; sort by variable name
            ASSERT_require((al->isVariable() && bl->isVariable()) || (al->isMemory() && bl->isMemory()));
            if (al->nameId() != bl->nameId())
                return al->nameId() < bl->nameId() ? -1 : 1;
            return 0;
        }
    } else {
        // both are internal nodes
        ASSERT_not_null(ai);
        ASSERT_not_null(bi);
        if (ai->getOperator() != bi->getOperator())
            return ai->getOperator() < bi->getOperator() ? -1 : 1;
        for (size_t i=0; i<std::min(ai->nChildren(), bi->nChildren()); ++i) {
            if (int cmp = expr_cmp(ai->child(i), bi->child(i)))
                return cmp;
        }
        if (ai->nChildren() != bi->nChildren())
            return ai->nChildren() < bi->nChildren() ? -1 : 1;
        return 0;
    }
}

static bool
commutative_order(const Ptr &a, const Ptr &b)
{
    if (int cmp = expr_cmp(a, b))
        return cmp<0;
    return getRawPointer(a) < getRawPointer(b); // make it a strict ordering
}

InteriorPtr
Interior::commutative() {
    const Nodes &orig_operands = children();
    Nodes sorted_operands = orig_operands;
    std::sort(sorted_operands.begin(), sorted_operands.end(), commutative_order);
    if (std::equal(sorted_operands.begin(), sorted_operands.end(), orig_operands.begin()))
        return isInteriorNode();

    // construct the new node but don't simplify it yet (i.e., don't use Interior::create())
    Interior *inode = new Interior(nBits(), getOperator(), sorted_operands, comment());
    return InteriorPtr(inode);
}

Ptr
Interior::involutary() {
    if (InteriorPtr inode = isInteriorNode()) {
        if (1==inode->nChildren()) {
            if (InteriorPtr sub1 = inode->child(0)->isInteriorNode()) {
                if (sub1->getOperator() == inode->getOperator() && 1==sub1->nChildren()) {
                    return sub1->child(0);
                }
            }
        }
    }
    return sharedFromThis();
}

// simplifies things like:
//   (shift a (shift b x)) ==> (shift (add a b) x)
// making sure a and b are extended to the same width
Ptr
Interior::additiveNesting() {
    InteriorPtr nested = child(1)->isInteriorNode();
    if (nested!=NULL && nested->getOperator()==getOperator()) {
        ASSERT_require(nested->nChildren()==nChildren());
        ASSERT_require(nested->nBits()==nBits());
        size_t additive_nbits = std::max(child(0)->nBits(), nested->child(0)->nBits());

        // The two addends must be the same width, so zero-extend them if necessary (or should we sign extend?)
        // Note that the first argument (new width) of the UEXTEND operator is not actually used.
        Ptr a = child(0)->nBits()==additive_nbits ? child(0) :
                makeExtend(makeInteger(8, additive_nbits), child(0));
        Ptr b = nested->child(0)->nBits()==additive_nbits ? nested->child(0) :
                makeExtend(makeInteger(8, additive_nbits), nested->child(0));
        
        // construct the new node but don't simplify it yet (i.e., don't use Interior::create())
        Interior *inode = new Interior(nBits(), getOperator(), makeAdd(a, b), nested->child(1), comment());
        return InteriorPtr(inode);
    }
    return isInteriorNode();
}

Ptr
Interior::identity(uint64_t ident) {
    Nodes args;
    bool modified = false;
    for (Nodes::const_iterator ci=children_.begin(); ci!=children_.end(); ++ci) {
        LeafPtr leaf = (*ci)->isLeafNode();
        if (leaf && leaf->isNumber()) {
            Sawyer::Container::BitVector identBv = Sawyer::Container::BitVector(leaf->nBits()).fromInteger(ident);
            if (0==leaf->bits().compare(identBv)) {
                // skip this arg
                modified = true;
            } else {
                args.push_back(*ci);
            }
        } else {
            args.push_back(*ci);
        }
    }
    if (!modified)
        return sharedFromThis();
    if (args.empty())
        return makeInteger(nBits(), ident, comment());
    if (1==args.size()) {
        if (args.front()->nBits()!=nBits())
            return makeExtend(makeInteger(8, nBits()), args.front());
        return args.front();
    }

    // Don't simplify the return value recursively
    Interior *inode = new Interior(0, getOperator(), args, comment());
    if (inode->nBits() != nBits())
        return sharedFromThis();                        // don't simplify if width changed.
    return InteriorPtr(inode);
}

Ptr
Interior::unaryNoOp() {
    return 1==nChildren() ? child(0) : sharedFromThis();
}

Ptr
Interior::rewrite(const Simplifier &simplifier) {
    if (Ptr simplified = simplifier.rewrite(this))
        return simplified;
    return sharedFromThis();
}

Ptr
Interior::foldConstants(const Simplifier &simplifier) {
    Nodes newOperands;
    bool modified = false;
    Nodes::const_iterator ci1 = children_.begin();
    while (ci1!=children_.end()) {
        Nodes::const_iterator ci2 = ci1;
        LeafPtr leaf;
        while (ci2!=children_.end() && (leaf=(*ci2)->isLeafNode()) && leaf->isNumber()) ++ci2;
        if (ci1==ci2 || ci1+1==ci2) {                           // arg is not a constant, or we had only one constant by itself
            newOperands.push_back(*ci1);
            ++ci1;
        } else if (Ptr folded = simplifier.fold(ci1, ci2)) { // able to fold all these constants into a new node
            newOperands.push_back(folded);
            modified = true;
            ci1 = ci2;
        } else {                                                // multiple constants, but unable to fold
            newOperands.insert(newOperands.end(), ci1, ci2);
            ci1 = ci2;
        }
    }
    if (!modified)
        return isInteriorNode();
    if (1==newOperands.size())
        return newOperands.front();

    // Do not simplify again (i.e., don't use Interior::create())
    return InteriorPtr(new Interior(nBits(), op_, newOperands, comment()));
}

Ptr
AddSimplifier::fold(Nodes::const_iterator begin, Nodes::const_iterator end) const {
    Sawyer::Container::BitVector accumulator((*begin)->nBits());
    unsigned flags = 0;
    for (/*void*/; begin!=end; ++begin) {
        accumulator.add((*begin)->isLeafNode()->bits());
        flags |= (*begin)->flags();
    }
    return makeConstant(accumulator, "", flags);
}

Ptr
AddSimplifier::rewrite(Interior *inode) const {
    // A and B are duals if they have one of the following forms:
    //    (1) A = x           AND  B = (negate x)
    //    (2) A = x           AND  B = (invert x)   [adjust constant]
    //    (3) A = (negate x)  AND  B = x
    //    (4) A = (invert x)  AND  B = x            [adjust constant]
    //
    // This makes use of the relationship:
    //   (add (negate x) -1) == (invert x)
    // by decrementing adjustment. The adjustment, whose width is the same as A and B, is allowed to overflow.  For example,
    // consider the expression, where all values are two bits wide:
    //   (add v1 (invert v1) v2 (invert v2) v3 (invert v3))            by substitution for invert:
    //   (add v1 (negate v1) -1 v2 (negate v2) -1 v3 (negate v3) -1)   canceling duals gives:
    //   (add -1 -1 -1)                                                rewriting as 2's complement (2 bits wide):
    //   (add 3 3 3)                                                   constant folding modulo 4:
    //   1
    // compare with v1=0, v2=1, v3=2 (i.e., -2 in two's complement):
    //   (add 0 3 1 2 2 1) == 1 mod 4
    struct are_duals {
        bool operator()(Ptr a, Ptr b, Sawyer::Container::BitVector &adjustment/*in,out*/) {
            ASSERT_not_null(a);
            ASSERT_not_null(b);
            ASSERT_require(a->nBits()==b->nBits());

            // swap A and B if necessary so we have form (1) or (2).
            if (b->isInteriorNode()==NULL)
                std::swap(a, b);
            if (b->isInteriorNode()==NULL)
                return false;

            InteriorPtr bi = b->isInteriorNode();
            if (bi->getOperator()==OP_NEGATE) {
                // form (3)
                ASSERT_require(1==bi->nChildren());
                return a->isEquivalentTo(bi->child(0));
            } else if (bi->getOperator()==OP_INVERT) {
                // form (4) and ninverts is small enough
                if (a->isEquivalentTo(bi->child(0))) {
                    adjustment.decrement();
                    return true;
                }
            }
            return false;
        }
    };

    // Rewrite (add ... (negate (add a b)) ...) => (add ... (negate a) (negate b) ...)
    struct distributeNegations {
        Ptr operator()(Interior *add) {
            Nodes children;
            bool distributed = false;
            for (size_t i=0; i<add->nChildren(); ++i) {
                bool pushed = false;
                InteriorPtr addArg = add->child(i)->isInteriorNode();
                if (addArg && addArg->getOperator()==OP_NEGATE && addArg->nChildren()==1) {
                    if (InteriorPtr negateArg = addArg->child(0)->isInteriorNode()) {
                        if (negateArg && negateArg->getOperator()==OP_ADD && negateArg->nChildren()>0) {
                            for (size_t j=0; j<negateArg->nChildren(); ++j)
                                children.push_back(makeNegate(negateArg->child(j)));
                            pushed = distributed = true;
                        }
                    }
                }
                if (!pushed)
                    children.push_back(add->child(i));
            }
            if (!distributed)
                return Ptr();
            return Interior::create(0, OP_ADD, children, add->comment());
        }
    };

    // Arguments that are negated cancel out similar arguments that are not negated
    bool had_duals = false;
    Sawyer::Container::BitVector adjustment(inode->nBits());
    Nodes children = inode->children();
    for (size_t i=0; i<children.size(); ++i) {
        if (children[i]!=NULL) {
            for (size_t j=i+1; j<children.size() && children[j]!=NULL; ++j) {
                if (children[j]!=NULL && are_duals()(children[i], children[j], adjustment/*in,out*/)) {
                    children[i] = Sawyer::Nothing();
                    children[j] = Sawyer::Nothing();
                    had_duals = true;
                    break;
                }
            }
        }
    }

    // Otherwise distribute negations across adds:
    //   (add ... (negate (add a b)) ...) => (add ... (negate a) (negate b) ...)
    if (!had_duals) {
        if (Ptr distributed = distributeNegations()(inode))
            return distributed;
        return Ptr();
    }

    // Build the new expression
    children.erase(std::remove(children.begin(), children.end(), Ptr()), children.end());
    if (!adjustment.isEqualToZero())
        children.push_back(makeConstant(adjustment));
    if (children.empty())
        return makeInteger(inode->nBits(), 0, inode->comment());
    if (children.size()==1)
        return children[0];
    return Interior::create(0, OP_ADD, children, inode->comment());
}

Ptr
AndSimplifier::fold(Nodes::const_iterator begin, Nodes::const_iterator end) const {
    Sawyer::Container::BitVector accumulator((*begin)->nBits(), true);
    unsigned flags = 0;
    for (/*void*/; begin!=end; ++begin) {
        accumulator.bitwiseAnd((*begin)->isLeafNode()->bits());
        flags |= (*begin)->flags();
    }
    return makeConstant(accumulator, "", flags);
}

Ptr
AndSimplifier::rewrite(Interior *inode) const {
    // Result is zero if any argument is zero
    for (size_t i=0; i<inode->nChildren(); ++i) {
        LeafPtr child = inode->child(i)->isLeafNode();
        if (child && child->isNumber() && child->bits().isEqualToZero())
            return makeInteger(inode->nBits(), 0, inode->comment(), child->flags());
    }

    // (and X X) => X (for any number of arguments that are all the same)
    bool allSameArgs = true;
    for (size_t i=1; i<inode->nChildren() && allSameArgs; ++i) {
        if (!inode->child(0)->isEquivalentTo(inode->child(i)))
            allSameArgs = false;
    }
    if (allSameArgs)
        return inode->child(0);

    return Ptr();
}

Ptr
OrSimplifier::fold(Nodes::const_iterator begin, Nodes::const_iterator end) const {
    Sawyer::Container::BitVector accumulator((*begin)->nBits());
    unsigned flags = 0;
    for (/*void*/; begin!=end; ++begin) {
        accumulator.bitwiseOr((*begin)->isLeafNode()->bits());
        flags |= (*begin)->flags();
    }
    return makeConstant(accumulator, "", flags);
}

Ptr
OrSimplifier::rewrite(Interior *inode) const {
    // Result has all bits set if any argument has all bits set
    for (size_t i=0; i<inode->nChildren(); ++i) {
        LeafPtr child = inode->child(i)->isLeafNode();
        if (child && child->isNumber() && child->bits().isAllSet())
            return makeConstant(child->bits(), inode->comment(), child->flags());
    }
    return Ptr();
}

Ptr
XorSimplifier::fold(Nodes::const_iterator begin, Nodes::const_iterator end) const {
    Sawyer::Container::BitVector accumulator((*begin)->nBits());
    unsigned flags = 0;
    for (++begin; begin!=end; ++begin) {
        accumulator.bitwiseXor((*begin)->isLeafNode()->bits());
        flags |= (*begin)->flags();
    }
    return makeConstant(accumulator, "", flags);
}

Ptr
XorSimplifier::rewrite(Interior *inode) const {
    SMTSolver *solver = NULL;   // FIXME

    // If any pairs of arguments are equal, then they don't contribute to the final answer.
    std::vector<bool> removed(inode->nChildren(), false);
    bool modified = false;
    for (size_t i=0; i<inode->nChildren(); ++i) {
        if (removed[i])
            continue;
        for (size_t j=i+1; j<inode->nChildren(); ++j) {
            if (!removed[j] && inode->child(i)->mustEqual(inode->child(j), solver)) {
                removed[i] = removed[j] = modified = true;
                break;
            }
        }
    }
    if (!modified)
        return Ptr();
    Nodes newargs;
    for (size_t i=0; i<inode->nChildren(); ++i) {
        if (!removed[i])
            newargs.push_back(inode->child(i));
    }
    if (newargs.empty())
        return makeInteger(inode->nBits(), 0, inode->comment());
    return Interior::create(0, inode->getOperator(), newargs, inode->comment());
}

Ptr
SmulSimplifier::fold(Nodes::const_iterator begin, Nodes::const_iterator end) const {
    // FIXME[Robb P. Matzke 2014-05-05]: Constant folding is not currently possible when the operands are wider than 64 bits
    // because Sawyer::Container::BitVector does not provide a multiplication method.
    size_t totalWidth = 0;
    int64_t product = 1;
    unsigned flags = 0;
    for (/*void*/; begin!=end; ++begin) {
        size_t nbits = (*begin)->nBits();
        totalWidth += nbits;
        if (totalWidth > 8*sizeof(product))
            return Ptr();
        LeafPtr leaf = (*begin)->isLeafNode();
        product *= (int64_t)leaf->toInt();
        flags |= (*begin)->flags();
    }
    return makeInteger(totalWidth, product, "", flags);
}

Ptr
UmulSimplifier::fold(Nodes::const_iterator begin, Nodes::const_iterator end) const {
    // FIXME[Robb P. Matzke 2014-05-05]: Constant folding is not currently possible when the operands are wider than 64 bits
    // because Sawyer::Container::BitVector does not provide a multiplication method.
    size_t totalWidth = 0;
    uint64_t product = 1;
    unsigned flags = 0;
    for (/*void*/; begin!=end; ++begin) {
        size_t nbits = (*begin)->nBits();
        totalWidth += nbits;
        if (totalWidth > 8*sizeof(product))
            return Ptr();
        LeafPtr leaf = (*begin)->isLeafNode();
        product *= (uint64_t)leaf->toInt();
        flags |= (*begin)->flags();
    }
    return makeInteger(totalWidth, product, "", flags);
}

Ptr
ConcatSimplifier::fold(Nodes::const_iterator begin, Nodes::const_iterator end) const {
    // first arg is high-order bits. Although this is nice to look at, it makes the operation a bit more difficult.
    size_t resultSize = 0;
    for (Nodes::const_iterator ti=begin; ti!=end; ++ti)
        resultSize += (*ti)->nBits();
    Sawyer::Container::BitVector accumulator(resultSize);

    // Copy bits into wherever they belong in the accumulator
    unsigned flags = 0;
    for (size_t sa=resultSize; begin!=end; ++begin) {
        LeafPtr leaf = (*begin)->isLeafNode();
        sa -= leaf->nBits();
        typedef Sawyer::Container::BitVector::BitRange BitRange;
        BitRange destination = BitRange::baseSize(sa, leaf->nBits());
        accumulator.copy(destination, leaf->bits(), leaf->bits().hull());
        flags |= (*begin)->flags();
    }
    return makeConstant(accumulator, "", flags);
}

Ptr
ConcatSimplifier::rewrite(Interior *inode) const {
    SMTSolver *solver = NULL; // FIXME

    // If all the concatenated expressions are extract expressions, all extracting bits from the same expression and
    // in the correct order, then we can simplify this to that expression.  For instance:
    //   (concat[32]
    //       (extract[8] 24[32] 32[32] v2[32])
    //       (extract[8] 16[32] 24[32] v2[32])
    //       (extract[8] 8[32] 16[32] v2[32])
    //       (extract[8] 0[32] 8[32] v2[32]))
    // can be simplified to
    //   v2
    Ptr retval;
    size_t offset = 0;
    for (size_t i=inode->nChildren(); i>0; --i) { // process args in little endian order
        InteriorPtr extract = inode->child(i-1)->isInteriorNode();
        if (!extract || OP_EXTRACT!=extract->getOperator())
            break;
        LeafPtr from_node = extract->child(0)->isLeafNode();
        ASSERT_require(from_node->nBits() <= 8*sizeof offset);
        if (!from_node || !from_node->isNumber() || from_node->toInt()!=offset ||
            extract->child(2)->nBits()!=inode->nBits())
            break;
        if (inode->nChildren()==i) {
            retval = extract->child(2);
        } else if (!extract->child(2)->mustEqual(retval, solver)) {
            break;
        }
        offset += extract->nBits();
    }
    if (offset==inode->nBits())
        return retval;
    return Ptr();
}

Ptr
ExtractSimplifier::rewrite(Interior *inode) const {
    LeafPtr from_node = inode->child(0)->isLeafNode();
    LeafPtr to_node   = inode->child(1)->isLeafNode();
    Ptr operand   = inode->child(2);
    ASSERT_require(!from_node->isNumber() || from_node->nBits() <= 8*sizeof(size_t));
    ASSERT_require(!to_node->isNumber()   || to_node->nBits() <= 8*sizeof(size_t));
    size_t from = from_node && from_node->isNumber() ? from_node->toInt() : 0;
    size_t to = to_node && to_node->isNumber() ? to_node->toInt() : 0;

    // If limits are backward or extend beyond the operand size, don't simplify
    if (from_node && to_node && from_node->isNumber() && to_node->isNumber() && (from>=to || to>operand->nBits()))
        return Ptr();

    // Constant folding
    if (from_node && to_node && from_node->isNumber() && to_node->isNumber() &&
        operand->isLeafNode() && operand->isLeafNode()->isNumber()) {
        Sawyer::Container::BitVector result(to-from);
        typedef Sawyer::Container::BitVector::BitRange BitRange;
        BitRange source = BitRange::hull(from, to-1);
        result.copy(result.hull(), operand->isLeafNode()->bits(), source);
        return makeConstant(result, inode->comment(), inode->flags());
    }

    // Extracting the whole thing is a no-op
    if (from_node && to_node && from_node->isNumber() && from==0 && to==operand->nBits())
        return operand;

    // Hoist concat operations to the outside of the extract
    // If the operand is a concat operation then take only the parts we need.  Some examples:
    // (extract 0 24 (concat X[24] Y[8]))  ==> (concat (extract 0 16 X) Y)
    Nodes newChildren;
    InteriorPtr ioperand = operand->isInteriorNode();
    if (from_node && to_node && from_node->isNumber() && to_node->isNumber() &&
        ioperand && OP_CONCAT==ioperand->getOperator()) {
        size_t partAt = 0;                              // starting bit number in child
        BOOST_REVERSE_FOREACH (const Ptr part, ioperand->children()) { // concatenated parts
            size_t partEnd = partAt + part->nBits();
            if (partEnd <= from) {
                // Part is entirely left of what we need
                partAt = partEnd;
            } else if (partAt >= to) {
                // Part is entirely right of what we need
                break;
            } else if (partAt < from && partEnd > to) {
                // We need the middle of this part, and then we're done
                size_t need = to-from;                  // number of bits we need
                newChildren.push_back(makeExtract(makeInteger(32, from-partAt), makeInteger(32, to-partAt), part));
                partAt = partEnd;
                from += need;
            } else if (partAt < from) {
                // We need the end of the part
                ASSERT_require(partEnd <= to);
                size_t need = partEnd - from;
                newChildren.push_back(makeExtract(makeInteger(32, from-partAt), makeInteger(32, part->nBits()), part));
                partAt = partEnd;
                from += need;
            } else if (partEnd > to) {
                // We need the beginning of the part
                ASSERT_require(partAt == from);
                size_t need = to-from;
                newChildren.push_back(makeExtract(makeInteger(32, 0), makeInteger(32, need), part));
                break;
            } else {
                // We need the whole part
                ASSERT_require(partAt >= from);
                ASSERT_require(partEnd <= to);
                newChildren.push_back(part);
                partAt = from = partEnd;
            }
        }

        // Concatenate all the parts.
        if (newChildren.size() > 1) {
            std::reverse(newChildren.begin(), newChildren.end());// high bits must be first
            return Interior::create(0, OP_CONCAT, newChildren, inode->comment());
        }
        newChildren[0]->comment(inode->comment());
        return newChildren[0];
    }

    // If the operand is another extract operation and we know all the limits then they can be replaced with a single extract.
    if (from_node && to_node && from_node->isNumber() && to_node->isNumber() &&
        ioperand && OP_EXTRACT==ioperand->getOperator()) {
        LeafPtr from2_node = ioperand->child(0)->isLeafNode();
        LeafPtr to2_node = ioperand->child(1)->isLeafNode();
        if (from2_node && to2_node && from2_node->isNumber() && to2_node->isNumber()) {
            size_t from2 = from2_node->toInt();
            return makeExtract(makeInteger(32, from2+from), makeInteger(32, from2+to), ioperand->child(2), inode->comment());
        }
    }

    // Simplifications for (extract 0 a (uextend b c))
    if (from_node && to_node && from_node->isNumber() && 0==from && to_node->isNumber()) {
        size_t a=to, b=operand->nBits();
        // (extract[a] 0 a (uextend[b] b c[a])) => c when b>=a
        if (ioperand && OP_UEXTEND==ioperand->getOperator() && b>=a && ioperand->child(1)->nBits()==a)
            return ioperand->child(1);
    }


    return Ptr();
}

Ptr
AsrSimplifier::rewrite(Interior *inode) const {
    ASSERT_require(2==inode->nChildren());

    // Constant folding
    LeafPtr shift_leaf   = inode->child(0)->isLeafNode();
    LeafPtr operand_leaf = inode->child(1)->isLeafNode();
    if (shift_leaf!=NULL && operand_leaf!=NULL && shift_leaf->isNumber() && operand_leaf->isNumber()) {
        size_t sa = shift_leaf->toInt();
        Sawyer::Container::BitVector result = operand_leaf->bits();
        result.shiftRightArithmetic(sa);
        return makeConstant(result, inode->comment(), inode->flags());
    }
    return Ptr();
}

Ptr
InvertSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr operand_node = inode->child(0)->isLeafNode();
    if (operand_node==NULL || !operand_node->isNumber())
        return Ptr();
    Sawyer::Container::BitVector result = operand_node->bits();
    result.invert();
    return makeConstant(result, inode->comment(), inode->flags());
}

Ptr
NegateSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr operand_node = inode->child(0)->isLeafNode();
    if (operand_node==NULL || !operand_node->isNumber())
        return Ptr();
    Sawyer::Container::BitVector result = operand_node->bits();
    result.negate();
    return makeConstant(result, inode->comment(), inode->flags());
}

Ptr
IteSimplifier::rewrite(Interior *inode) const {
    // Is the condition known?
    LeafPtr cond_node = inode->child(0)->isLeafNode();
    if (cond_node!=NULL && cond_node->isNumber()) {
        if (cond_node->nBits() != 1)
            throw Exception(toStr(inode->getOperator()) + " operator's first argument (condition) should be one bit wide");
        return cond_node->toInt() ? inode->child(1) : inode->child(2);
    }

    // Are both operands the same? Then the condition doesn't matter
    if (inode->child(1)->isEquivalentTo(inode->child(2)))
        return inode->child(1);

    return Ptr();
}

Ptr
NoopSimplifier::rewrite(Interior *inode) const {
    if (1==inode->nChildren())
        return inode->child(0);
    return Ptr();
}

Ptr
RolSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr sa_leaf = inode->child(0)->isLeafNode();
    LeafPtr val_leaf = inode->child(1)->isLeafNode();
    if (sa_leaf && val_leaf && sa_leaf->isNumber() && val_leaf->isNumber()) {
        Sawyer::Container::BitVector result = val_leaf->bits();
        result.rotateLeft(sa_leaf->toInt());
        return makeConstant(result, inode->comment(), inode->flags());
    }

    // If the shift amount is known and is a multiple of the operand size, then this is a no-op
    if (sa_leaf && sa_leaf->isNumber() && 0==sa_leaf->toInt() % inode->nBits())
        return inode->child(1);

    return Ptr();
}
Ptr
RorSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr sa_leaf = inode->child(0)->isLeafNode();
    LeafPtr val_leaf = inode->child(1)->isLeafNode();
    if (sa_leaf && val_leaf && sa_leaf->isNumber() && val_leaf->isNumber()) {
        Sawyer::Container::BitVector result = val_leaf->bits();
        result.rotateRight(sa_leaf->toInt());
        return makeConstant(result, inode->comment(), inode->flags());
    }

    // If the shift amount is known and is a multiple of the operand size, then this is a no-op
    if (sa_leaf && sa_leaf->isNumber() && 0==sa_leaf->toInt() % inode->nBits())
        return inode->child(1);

    return Ptr();
}

Ptr
UextendSimplifier::rewrite(Interior *inode) const {
    // Noop case
    size_t oldsize = inode->child(1)->nBits();
    size_t newsize = inode->nBits();
    if (oldsize==newsize)
        return inode->child(1);

    // Constant folding
    LeafPtr val_leaf = inode->child(1)->isLeafNode();
    if (val_leaf && val_leaf->isNumber()) {
        Sawyer::Container::BitVector result = val_leaf->bits();
        result.resize(newsize);
        return makeConstant(result, inode->comment(), inode->flags());
    }

    // If the new size is smaller than the old size, use OP_EXTRACT instead.
    if (newsize<oldsize) {
        return makeExtract(makeInteger(32, 0), makeInteger(32, newsize), inode->child(1), inode->comment());
    }

    return Ptr();
}

Ptr
SextendSimplifier::rewrite(Interior *inode) const {
    // Noop case
    size_t oldsize = inode->child(1)->nBits();
    size_t newsize = inode->nBits();
    if (oldsize==newsize)
        return inode->child(1);

    // Constant folding
    LeafPtr val_leaf = inode->child(1)->isLeafNode();
    if (val_leaf && val_leaf->isNumber()) {
        Sawyer::Container::BitVector result(inode->nBits());
        result.signExtend(val_leaf->bits());
        return makeConstant(result, inode->comment(), inode->flags());
    }

    // Downsizing should be represented as an extract operation
    if (newsize < oldsize) {
        return makeExtract(makeInteger(32, 0), makeInteger(32, newsize), inode->child(1), inode->comment());
    }

    return Ptr();
}

Ptr
EqSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber()) {
        int cmp = a_leaf->bits().compare(b_leaf->bits());
        return makeBoolean(0==cmp, inode->comment(), inode->flags());
    }

    // (eq x x) => 1
    if (inode->child(0)->mustEqual(inode->child(1), NULL))
        return makeBoolean(true, inode->comment(), inode->flags());

    return Ptr();
}

Ptr
SgeSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber()) {
        int cmp = a_leaf->bits().compareSigned(b_leaf->bits());
        return makeBoolean(cmp>=0, inode->comment(), inode->flags());
    }

    // (sge x x) => 1
    if (inode->child(0)->mustEqual(inode->child(1), NULL))
        return makeBoolean(true, inode->comment(), inode->flags());

    return Ptr();
}

Ptr
SgtSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber()) {
        int cmp = a_leaf->bits().compareSigned(b_leaf->bits());
        return makeBoolean(cmp>0, inode->comment(), inode->flags());
    }

    return Ptr();
}

Ptr
SleSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber()) {
        int cmp = a_leaf->bits().compareSigned(b_leaf->bits());
        return makeBoolean(cmp<=0, inode->comment(), inode->flags());
    }

    // (sle x x) => 1
    if (inode->child(0)->mustEqual(inode->child(1), NULL))
        return makeBoolean(true, inode->comment(), inode->flags());

    return Ptr();
}

Ptr
SltSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber()) {
        int cmp = a_leaf->bits().compareSigned(b_leaf->bits());
        return makeBoolean(cmp<0, inode->comment(), inode->flags());
    }

    return Ptr();
}

Ptr
UgeSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber()) {
        int cmp = a_leaf->bits().compare(b_leaf->bits());
        return makeBoolean(cmp>=0, inode->comment(), inode->flags());
    }

    // (uge x x) => 1
    if (inode->child(0)->mustEqual(inode->child(1), NULL))
        return makeBoolean(true, inode->comment(), inode->flags());

   return Ptr();
}

Ptr
UgtSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber()) {
        int cmp = a_leaf->bits().compare(b_leaf->bits());
        return makeBoolean(cmp>0, inode->comment(), inode->flags());
    }

    return Ptr();
}

Ptr
UleSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber()) {
        int cmp = a_leaf->bits().compare(b_leaf->bits());
        return makeBoolean(cmp<=0, inode->comment(), inode->flags());
    }

    // (ule x x) => 1
    if (inode->child(0)->mustEqual(inode->child(1), NULL))
        return makeBoolean(true, inode->comment(), inode->flags());

    return Ptr();
}

Ptr
UltSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber()) {
        int cmp = a_leaf->bits().compare(b_leaf->bits());
        return makeBoolean(cmp<0, inode->comment(), inode->flags());
    }

    return Ptr();
}

Ptr
ZeropSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    if (a_leaf && a_leaf->isNumber())
        return makeBoolean(a_leaf->bits().isEqualToZero(), inode->comment(), inode->flags());
    
    return Ptr();
}

Ptr
SdivSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber() && b_leaf->toInt()!=0) {
        if (a_leaf->nBits() <= 64 && b_leaf->nBits() <= 64) {
            int64_t a = IntegerOps::signExtend2(a_leaf->toInt(), a_leaf->nBits(), 8*sizeof(int8_t));
            int64_t b = IntegerOps::signExtend2(b_leaf->toInt(), b_leaf->nBits(), 8*sizeof(int8_t));
            return makeInteger(a_leaf->nBits(), a/b, inode->comment(), inode->flags());
        } else {
            // FIXME[Robb P. Matzke 2014-05-05]: not folding constants larger than 64 bits because Sawyer::Container::BitVector
            // does not currently define division.
        }
    }
    return Ptr();
}

Ptr
SmodSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber() && b_leaf->toInt()!=0) {
        if (a_leaf->nBits() <= 64 && b_leaf->nBits() <= 64) {
            int64_t a = IntegerOps::signExtend2(a_leaf->toInt(), a_leaf->nBits(), 8*sizeof(int8_t));
            int64_t b = IntegerOps::signExtend2(b_leaf->toInt(), b_leaf->nBits(), 8*sizeof(int8_t));
            return makeInteger(b_leaf->nBits(), a%b, inode->comment(), inode->flags());
        } else {
            // FIXME[Robb P. Matzke 2014-05-05]: not folding constants larger than 64 bits because Sawyer::Container::BitVector
            // does not currently define division.
        }
    }

    return Ptr();
}

Ptr
UdivSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber() && b_leaf->toInt()!=0) {
        if (a_leaf->nBits() <= 64 && b_leaf->nBits() <= 64) {
            uint64_t a = a_leaf->toInt();
            uint64_t b = b_leaf->toInt();
            return makeInteger(a_leaf->nBits(), a/b, inode->comment(), inode->flags());
        } else {
            // FIXME[Robb P. Matzke 2014-05-05]: not folding constants larger than 64 bits because Sawyer::Container::BitVector
            // does not currently define division.
        }
    }

    return Ptr();
}

Ptr
UmodSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    LeafPtr b_leaf = inode->child(1)->isLeafNode();
    if (a_leaf && b_leaf && a_leaf->isNumber() && b_leaf->isNumber() && b_leaf->toInt()!=0) {
        if (a_leaf->nBits() <= 64 && b_leaf->nBits() <= 64) {
            uint64_t a = a_leaf->toInt();
            uint64_t b = b_leaf->toInt();
            return makeInteger(b_leaf->nBits(), a%b, inode->comment(), inode->flags());
        } else {
            // FIXME[Robb P. Matzke 2014-05-05]: not folding constants larger than 64 bits because Sawyer::Container::BitVector
            // does not currently define division.
        }
    }

    return Ptr();
}

Ptr
ShiftSimplifier::combine_strengths(Ptr strength1, Ptr strength2, size_t value_width) const {
    if (!strength1 || !strength2)
        return Ptr();

    // Calculate the width for the sum of the strengths.  If the width of the value being shifted isn't a power of two then we
    // need to avoid overflow in the sum, otherwise overflow doesn't matter.  The sum should be wide enough to hold a shift
    // amount that's the same as the width of the value, otherwise we wouldn't be able to distinguish between the case where
    // modulo addition produced a shift amount that's large enough to decimate the value, as opposed to a shift count of zero
    // which is a no-op.
    size_t sum_width = std::max(strength1->nBits(), strength2->nBits());
    if (IntegerOps::isPowerOfTwo(value_width)) {
        sum_width = std::max(sum_width, IntegerOps::log2max(value_width)+1);
    } else {
        sum_width = std::max(sum_width+1, IntegerOps::log2max(value_width)+1);
    }
    if (sum_width > 64)
        return Ptr();

    // Zero-extend the strengths if they're not as wide as the sum.  This is because the ADD operator requires that its
    // operands are the same width, and the result will also be that width.
    if (strength1->nBits() < sum_width)
        strength1 = makeExtend(makeInteger(32, sum_width), strength1);
    if (strength2->nBits() < sum_width)
        strength2 = makeExtend(makeInteger(32, sum_width), strength2);

    return makeAdd(strength1, strength2);
}

Ptr
ShlSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr sa_leaf = inode->child(0)->isLeafNode();
    LeafPtr val_leaf = inode->child(1)->isLeafNode();
    if (sa_leaf && val_leaf && sa_leaf->isNumber() && val_leaf->isNumber()) {
        uint64_t sa = sa_leaf->toInt();
        sa = std::min((uint64_t)inode->nBits(), sa);
        Sawyer::Container::BitVector result = val_leaf->bits();
        result.shiftLeft(sa, newbits);
        return makeConstant(result, inode->comment(), inode->flags());
    }

    // If the shifted operand is itself a shift of the same kind, then simplify by combining the strengths:
    // (shl AMT1 (shl AMT2 X)) ==> (shl (add AMT1 AMT2) X)
    InteriorPtr val_inode = inode->child(1)->isInteriorNode();
    if (val_inode && val_inode->getOperator()==inode->getOperator()) {
        if (Ptr strength = combine_strengths(inode->child(0), val_inode->child(0), inode->child(1)->nBits())) {
            return Interior::create(0, inode->getOperator(), strength, val_inode->child(1));
        }
    }

    // If the shift amount is known to be at least as large as the value, then replace the value with a constant.
    if (sa_leaf && sa_leaf->isNumber() && sa_leaf->toInt() >= inode->nBits()) {
        Sawyer::Container::BitVector result(inode->nBits(), newbits);
        return makeConstant(result, inode->comment());
    }

    // If the shift amount is zero then this is a no-op
    if (sa_leaf && sa_leaf->isNumber() && sa_leaf->toInt()==0)
        return inode->child(1);

    // If the shift amount is a constant, then:
    // (shl0[N] AMT X) ==> (concat (extract 0 N-AMT X)<hiBits> 0[AMT]<loBits>)
    // (shl1[N] AMT X) ==> (concat (extract 0 N-AMT X)<hiBits> -1[AMT]<loBits>)
    if (sa_leaf && sa_leaf->isNumber()) {
        ASSERT_require(sa_leaf->toInt()>0 && sa_leaf->toInt()<inode->nBits());// handled above
        size_t nHiBits = inode->nBits() - sa_leaf->toInt();
        Ptr hiBits = makeExtract(makeInteger(32, 0), makeInteger(32, nHiBits), inode->child(1));
        Ptr loBits = makeInteger(sa_leaf->toInt(), newbits?uint64_t(-1):uint64_t(0));
        return makeConcat(hiBits, loBits);
    }
    
    return Ptr();
}

Ptr
ShrSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr sa_leaf = inode->child(0)->isLeafNode();
    LeafPtr val_leaf = inode->child(1)->isLeafNode();
    if (sa_leaf && val_leaf && sa_leaf->isNumber() && val_leaf->isNumber()) {
        uint64_t sa = sa_leaf->toInt();
        sa = std::min((uint64_t)inode->nBits(), sa);
        Sawyer::Container::BitVector result = val_leaf->bits();
        result.shiftRight(sa, newbits);
        return makeConstant(result, inode->comment(), inode->flags());
    }

    // If the shifted operand is itself a shift of the same kind, then simplify by combining the strengths:
    //   (shr0 AMT1 (shr0 AMT2 X)) ==> (shr0 (add AMT1 AMT2) X)
    InteriorPtr val_inode = inode->child(1)->isInteriorNode();
    if (val_inode && val_inode->getOperator()==inode->getOperator()) {
        if (Ptr strength = combine_strengths(inode->child(0), val_inode->child(0), inode->child(1)->nBits())) {
            return Interior::create(0, inode->getOperator(), strength, val_inode->child(1));
        }
    }
    
    // If the shift amount is known to be at least as large as the value, then replace the value with a constant.
    if (sa_leaf && sa_leaf->isNumber() && sa_leaf->toInt() >= inode->nBits()) {
        Sawyer::Container::BitVector result(inode->nBits(), newbits);
        return makeConstant(result, inode->comment(), inode->flags());
    }

    // If the shift amount is zero then this is a no-op
    if (sa_leaf && sa_leaf->isNumber() && sa_leaf->toInt()==0)
        return inode->child(1);

    // If the shift amount is a constant, then:
    // (shr0[N] AMT X) ==> (concat 0[AMT]  (extract AMT N X))
    // (shr1[N] AMT X) ==> (concat -1[AMT] (extract AMT N X))
    if (sa_leaf && sa_leaf->isNumber()) {
        ASSERT_require(sa_leaf->toInt()>0 && sa_leaf->toInt()<inode->nBits());// handled above
        Ptr loBits = makeExtract(makeInteger(32, sa_leaf->toInt()), makeInteger(32, inode->nBits()), inode->child(1));
        Ptr hiBits = makeInteger(sa_leaf->toInt(), newbits?uint64_t(-1):uint64_t(0));
        return makeConcat(hiBits, loBits);
    }
    
    return Ptr();
}

Ptr
LssbSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    if (a_leaf && a_leaf->isNumber()) {
        if (Sawyer::Optional<size_t> idx = a_leaf->bits().leastSignificantSetBit())
            return makeInteger(inode->nBits(), *idx, inode->comment());
        return makeInteger(inode->nBits(), 0, inode->comment(), inode->flags());
    }

    return Ptr();
}

Ptr
MssbSimplifier::rewrite(Interior *inode) const {
    // Constant folding
    LeafPtr a_leaf = inode->child(0)->isLeafNode();
    if (a_leaf && a_leaf->isNumber()) {
        if (Sawyer::Optional<size_t> idx = a_leaf->bits().mostSignificantSetBit())
            return makeInteger(inode->nBits(), *idx, inode->comment());
        return makeInteger(inode->nBits(), 0, inode->comment(), inode->flags());
    }

    return Ptr();
}

Ptr
SetSimplifier::rewrite(Interior *inode) const {
    SMTSolver *solver = NULL;                           // FIXME[Robb Matzke 2015-11-03]

    // (set x) => x
    if (1 == inode->nChildren())
        return inode->child(0);

    // Remove duplicate arguments
    bool removedDuplicate = false;
    Nodes elements;
    BOOST_FOREACH (const Ptr &elmt1, inode->children()) {
        bool isDuplicate = false;
        BOOST_FOREACH (const Ptr &elmt2, elements) {
            if (elmt1->mustEqual(elmt2, solver)) {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate) {
            elements.push_back(elmt1);
        } else {
            removedDuplicate = true;
        }
    }
    if (!removedDuplicate)
        return Ptr();
    if (1==elements.size())
        return elements[0];
    return Interior::create(0, inode->getOperator(), elements, inode->comment());
}

Ptr
Interior::simplifyTop() {
    Ptr node = sharedFromThis();
    while (InteriorPtr inode = node->isInteriorNode()) {
        Ptr newnode = node;
        switch (inode->getOperator()) {
            case OP_ADD:
                newnode = inode->rewrite(AddSimplifier());
                if (newnode==node)
                    newnode = inode->associative()->commutative()->identity(0);
                if (newnode==node)
                    newnode = inode->foldConstants(AddSimplifier());
                break;
            case OP_AND:
            case OP_BV_AND:
                newnode = inode->associative()->commutative()->identity((uint64_t)-1);
                if (newnode==node)
                    newnode = inode->foldConstants(AndSimplifier());
                if (newnode==node)
                    newnode = inode->rewrite(AndSimplifier());
                break;
            case OP_ASR:
                newnode = inode->additiveNesting();
                if (newnode==node)
                    newnode = inode->rewrite(AsrSimplifier());
                break;
            case OP_BV_XOR:
                newnode = inode->associative()->commutative()->foldConstants(XorSimplifier());
                if (newnode==node)
                    newnode = inode->rewrite(XorSimplifier());
                break;
            case OP_CONCAT:
                newnode = inode->associative()->foldConstants(ConcatSimplifier());
                if (newnode==node)
                    newnode = inode->rewrite(ConcatSimplifier());
                break;
            case OP_EQ:
                newnode = inode->commutative();
                if (newnode==node)
                    newnode = inode->rewrite(EqSimplifier());
                break;
            case OP_EXTRACT:
                newnode = inode->rewrite(ExtractSimplifier());
                break;
            case OP_INVERT:
                newnode = inode->involutary();
                if (newnode==node)
                    newnode = inode->rewrite(InvertSimplifier());
                break;
            case OP_ITE:
                newnode = inode->rewrite(IteSimplifier());
                break;
            case OP_LSSB:
                newnode = inode->rewrite(LssbSimplifier());
                break;
            case OP_MSSB:
                newnode = inode->rewrite(MssbSimplifier());
                break;
            case OP_NE:
                newnode = inode->commutative();
                break;
            case OP_NEGATE:
                newnode = inode->involutary();
                if (newnode==node)
                    newnode = inode->rewrite(NegateSimplifier());
                break;
            case OP_NOOP:
                newnode = inode->rewrite(NoopSimplifier());
                break;
            case OP_OR:
            case OP_BV_OR:
                newnode = inode->associative()->commutative()->identity(0);
                if (newnode==node)
                    newnode = inode->foldConstants(OrSimplifier());
                if (newnode==node)
                    newnode = inode->rewrite(OrSimplifier());
                break;
            case OP_READ:
                // no simplifications
                break;
            case OP_ROL:
                newnode = inode->rewrite(RolSimplifier());
                break;
            case OP_ROR:
                newnode = inode->rewrite(RorSimplifier());
                break;
            case OP_SDIV:
                newnode = inode->rewrite(SdivSimplifier());
                break;
            case OP_SET:
                newnode = inode->associative()->commutative();
                if (newnode==node)
                    newnode = inode->rewrite(SetSimplifier());
                break;
            case OP_SEXTEND:
                newnode = inode->rewrite(SextendSimplifier());
                break;
            case OP_SGE:
                newnode = inode->rewrite(SgeSimplifier());
                break;
            case OP_SGT:
                newnode = inode->rewrite(SgtSimplifier());
                break;
            case OP_SHL0:
                newnode = inode->additiveNesting();
                if (newnode==node)
                    newnode = inode->rewrite(ShlSimplifier(false));
                break;
            case OP_SHL1:
                newnode = inode->additiveNesting();
                if (newnode==node)
                    newnode = inode->rewrite(ShlSimplifier(true));
                break;
            case OP_SHR0:
                newnode = inode->additiveNesting();
                if (newnode==node)
                    newnode = inode->rewrite(ShrSimplifier(false));
                break;
            case OP_SHR1:
                newnode = inode->additiveNesting();
                if (newnode==node)
                    newnode = inode->rewrite(ShrSimplifier(true));
                break;
            case OP_SLE:
                newnode = inode->rewrite(SleSimplifier());
                break;
            case OP_SLT:
                newnode = inode->rewrite(SltSimplifier());
                break;
            case OP_SMOD:
                newnode = inode->rewrite(SmodSimplifier());
                break;
            case OP_SMUL:
                newnode = inode->associative()->commutative()->foldConstants(SmulSimplifier());
                break;
            case OP_UDIV:
                newnode = inode->rewrite(UdivSimplifier());
                break;
            case OP_UEXTEND:
                newnode = inode->rewrite(UextendSimplifier());
                break;
            case OP_UGE:
                newnode = inode->rewrite(UgeSimplifier());
                break;
            case OP_UGT:
                newnode = inode->rewrite(UgtSimplifier());
                break;
            case OP_ULE:
                newnode = inode->rewrite(UleSimplifier());
                break;
            case OP_ULT:
                newnode = inode->rewrite(UltSimplifier());
                break;
            case OP_UMOD:
                newnode = inode->rewrite(UmodSimplifier());
                break;
            case OP_UMUL:
                newnode = inode->associative()->commutative()->identity(1);
                if (newnode==node)
                    newnode = inode->foldConstants(UmulSimplifier());
                break;
            case OP_WRITE:
                // no simplifications
                break;
            case OP_ZEROP:
                newnode = inode->rewrite(ZeropSimplifier());
                break;
        }
        if (newnode==node)
            break;
        node = newnode;
    }
    return node;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Leaf nodes
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// class method
LeafPtr
Leaf::createVariable(size_t nbits, const std::string &comment, unsigned flags) {
    if (0 == nbits)
        throw Exception("variables must have positive width");
    Leaf *node = new Leaf(comment, flags);
    node->nBits_ = nbits;
    node->leafType_ = BITVECTOR;
    node->name_ = nextNameCounter();
    LeafPtr retval(node);
    return retval;
}

// class method
LeafPtr
Leaf::createExistingVariable(size_t nbits, uint64_t id, const std::string &comment, unsigned flags) {
    if (0 == nbits)
        throw Exception("variables must have positive width");
    Leaf *node = new Leaf(comment, flags);
    node->nBits_ = nbits;
    node->leafType_ = BITVECTOR;
    node->name_ = nextNameCounter(id);
    LeafPtr retval(node);
    return retval;
}

// class method
LeafPtr
Leaf::createInteger(size_t nbits, uint64_t n, const std::string &comment, unsigned flags) {
    if (0 == nbits)
        throw Exception("integers must have positive width");
    Leaf *node = new Leaf(comment, flags);
    node->nBits_ = nbits;
    node->leafType_ = CONSTANT;
    node->bits_ = Sawyer::Container::BitVector(nbits).fromInteger(n);
    LeafPtr retval(node);
    return retval;
}

// class method
LeafPtr
Leaf::createConstant(const Sawyer::Container::BitVector &bits, const std::string &comment, unsigned flags) {
    Leaf *node = new Leaf(comment, flags);
    node->nBits_ = bits.size();
    node->leafType_ = CONSTANT;
    node->bits_ = bits;
    LeafPtr retval(node);
    return retval;
}

// class method
LeafPtr
Leaf::createMemory(size_t addressWidth, size_t valueWidth, const std::string &comment, unsigned flags) {
    if (0 == addressWidth)
        throw Exception("memory addresses must have positive width");
    if (0 == valueWidth)
        throw Exception("memory values must have positive width");
    Leaf *node = new Leaf(comment, flags);
    node->nBits_ = valueWidth;
    node->domainWidth_ = addressWidth;
    node->leafType_ = MEMORY;
    node->name_ = nextNameCounter();
    LeafPtr retval(node);
    return retval;
}

// class method
LeafPtr
Leaf::createExistingMemory(size_t addressWidth, size_t valueWidth, uint64_t id, const std::string &comment, unsigned flags) {
    if (0 == addressWidth)
        throw Exception("memory addresses must have positive width");
    if (0 == valueWidth)
        throw Exception("memory values must have positive width");
    Leaf *node = new Leaf(comment, flags);
    node->nBits_ = valueWidth;
    node->domainWidth_ = addressWidth;
    node->leafType_ = MEMORY;
    node->name_ = nextNameCounter(id);
    LeafPtr retval(node);
    return retval;
}
    
bool
Leaf::isNumber() {
    return CONSTANT==leafType_;
}

uint64_t
Leaf::toInt() {
    ASSERT_require(isNumber());
    ASSERT_require(nBits() <= 64);
    return bits_.toInteger();
}

const Sawyer::Container::BitVector&
Leaf::bits() {
    ASSERT_require(isNumber());
    return bits_;
}

bool
Leaf::isVariable() {
    return BITVECTOR==leafType_;
}

bool
Leaf::isMemory() {
    return MEMORY==leafType_;
}

uint64_t
Leaf::nameId() {
    ASSERT_require(isVariable() || isMemory());
    return name_;
}

std::string
Leaf::toString() {
    if (isNumber())
        return "0x" + bits().toHex();
    if (isVariable())
        return "v" + StringUtility::numberToString(nameId());
    if (isMemory())
        return "m" + StringUtility::numberToString(nameId());
    ASSERT_not_reachable("invalid leaf type");
    return "";
}

void
Leaf::print(std::ostream &o, Formatter &formatter) {
    printAsSigned(o, formatter);
}

void
Leaf::printAsSigned(std::ostream &o, Formatter &formatter, bool as_signed) {
    bool showed_comment = false;
    if (isNumber()) {
        if (bits_.size() == 1) {
            // Boolean values
            if (bits_.toInteger()) {
                o <<"true";
            } else {
                o <<"false";
            }
        } else if (bits_.size() <= 64) {
            // Integer values that are small enough to use the machine's native type.
            uint64_t ival = bits_.toInteger();
            if ((32==nBits_ || 64==nBits_) && 0!=(ival & 0xffff0000) && 0xffff0000!=(ival & 0xffff0000)) {
                // The value is probably an address, so print it like one.
                if (formatter.use_hexadecimal) {
                    o <<StringUtility::unsignedToHex2(ival, nBits_);
                } else {
                    // The old behavior (which is enabled when formatter.use_hexadecimal is false) was to print only the
                    // hexadecimal format and not the decimal format, so we'll emulate that. [Robb P. Matzke 2013-12-26]
                    o <<StringUtility::addrToString(ival, nBits_);
                }
            } else if (as_signed) {
                if (formatter.use_hexadecimal) {
                    o <<StringUtility::toHex2(ival, nBits_); // show as signed and unsigned
                } else if (IntegerOps::signBit2(ival, nBits_)) {
                    o <<(int64_t)IntegerOps::signExtend2(ival, nBits_, 64);
                } else {
                    o <<ival;
                }
            } else {
                if (formatter.use_hexadecimal) {
                    o <<StringUtility::unsignedToHex2(ival, nBits_); // show only as unsigned
                } else {
                    o <<ival;
                }
            }
        } else {
            // Integers that are too wide -- use bit vector support instead.
            // FIXME[Robb P. Matzke 2014-05-05]: we should change StringUtility functions to handle BitVector arguments also.
            o <<"0x" <<bits_.toHex();
        }
    } else if (formatter.show_comments==Formatter::CMT_INSTEAD && !comment_.empty()) {
        // Use the comment as the variable name.
        o <<nameEscape(comment_);
        showed_comment = true;
    } else {
        // Show the variable name.
        uint64_t renamed = name_;
        if (formatter.do_rename) {
            /*RenameMap::iterator found = formatter.renames.find(name_);
            if (found==formatter.renames.end() && formatter.add_renames) {
                renamed = formatter.renames.size();
                formatter.renames.insert(std::make_pair(name_, renamed));
            } else {
                renamed = found->second;
            }*/
        }
        switch (leafType_) {
            case MEMORY:
                o <<"m";
                break;
            case BITVECTOR:
                o <<"v";
                break;
            case CONSTANT:
                ASSERT_not_reachable("handled above");
        }
        o <<renamed;
    }

    // Bit width of variable.  All variables have this otherwise there's no way for the parser to tell how wide a variable is
    // when reading it back in.
    if (formatter.show_width) {
        o <<'[' <<nBits_ <<']';
    }

    // Comment stuff
    char bracket='<';
    if (formatter.show_flags)
        printFlags(o, flags(), bracket /*in,out*/);
    if (!showed_comment && formatter.show_comments!=Formatter::CMT_SILENT && !comment_.empty()) {
        o <<bracket <<commentEscape(comment_);
        bracket = ',';
    }
    if (bracket != '<')
        o <<">";
}

bool
Leaf::mustEqual(const Ptr &other_, SMTSolver *solver) {
    bool retval = false;
    LeafPtr other = other_->isLeafNode();
    if (this==getRawPointer(other)) {
        retval = true;
    } else if (flags() != other_->flags()) {
        retval = false;
    } else if (other==NULL) {
        // We need an SMT solver to figure this out.  This handles things like "x mustEqual (not (not x))" which is true.
        if (solver) {
            Ptr assertion = makeNe(sharedFromThis(), other_);
            retval = SMTSolver::SAT_NO==solver->satisfiable(assertion); // must equal if there is no soln for inequality
        }
    } else if (isNumber()) {
        retval = other->isNumber() && 0==bits_.compare(other->bits_);
    } else {
        retval = !other->isNumber() && name_==other->name_;
    }
    return retval;
}

bool
Leaf::mayEqual(const Ptr &other_, SMTSolver *solver) {
    bool retval = false;
    LeafPtr other = other_->isLeafNode();
    if (this==getRawPointer(other)) {
        retval = true;
    } else if (other==NULL) {
        // We need an SMT solver to figure out things like "x mayEqual (add y 1))", which is true.
        if (solver) {
            Ptr assertion = makeEq(sharedFromThis(), other_);
            retval = SMTSolver::SAT_YES == solver->satisfiable(assertion);
        }
    } else if (!isNumber() || !other->isNumber() || 0==bits_.compare(other->bits_)) {
        retval = true;
    }
    return retval;
}

int
Leaf::compareStructure(const Ptr &other_) {
    LeafPtr other = other_->isLeafNode();
    if (this==getRawPointer(other)) {
        return 0;
    } else if (other==NULL) {
        return -1;                                      // leaf nodes < internal nodes
    } else if (nBits() != other->nBits()) {
        return nBits() < other->nBits() ? -1 : 1;
    } else if (flags() != other->flags()) {
        return flags() < other->flags() ? -1 : 1;
    } else if (isNumber() != other->isNumber()) {
        return isNumber() ? -1 : 1;                     // concrete values < non-concrete
    } else if (name_ != other->name_) {
        return name_ < other->name_ ? -1 : 1;
    }
    return 0;
}

bool
Leaf::isEquivalentTo(const Ptr &other_) {
    bool retval = false;
    LeafPtr other = other_->isLeafNode();
    if (this==getRawPointer(other)) {
        retval = true;
    } else if (other && nBits()==other->nBits() && flags()==other->flags()) {
        if (isNumber()) {
            retval = other->isNumber() && 0==bits_.compare(other->bits_);
        } else {
            retval = !other->isNumber() && name_==other->name_;
        }
    }
    return retval;
}

Ptr
Leaf::substitute(const Ptr &from, const Ptr &to) {
    ASSERT_require(from!=NULL && to!=NULL && from->nBits()==to->nBits());
    if (isEquivalentTo(from))
        return to;
    return sharedFromThis();
}

VisitAction
Leaf::depthFirstTraversal(Visitor &v) {
    Ptr self = sharedFromThis();
    VisitAction retval = v.preVisit(self);
    if (TERMINATE!=retval)
        retval = v.postVisit(self);
    return retval;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Free functions of the API
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::ostream&
operator<<(std::ostream &o, Node &node) {
    Formatter fmt;
    node.print(o, fmt);
    return o;
}

std::ostream&
operator<<(std::ostream &o, const Node::WithFormatter &w)
{
    w.print(o);
    return o;
}

std::vector<Ptr>
findCommonSubexpressions(const std::vector<Ptr> &exprs) {
    return findCommonSubexpressions(exprs.begin(), exprs.end());
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      Factory functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ptr
makeVariable(size_t nbits, const std::string &comment, unsigned flags) {
    return Leaf::createVariable(nbits, comment, flags);
}

Ptr
makeExistingVariable(size_t nbits, uint64_t id, const std::string &comment, unsigned flags) {
    return Leaf::createExistingVariable(nbits, id, comment, flags);
}

Ptr
makeInteger(size_t nbits, uint64_t n, const std::string &comment, unsigned flags) {
    return Leaf::createInteger(nbits, n, comment, flags);
}

Ptr
makeConstant(const Sawyer::Container::BitVector &bits, const std::string &comment, unsigned flags) {
    return Leaf::createConstant(bits, comment, flags);
}

Ptr
makeBoolean(bool b, const std::string &comment, unsigned flags) {
    return Leaf::createBoolean(b, comment, flags);
}

Ptr
makeMemory(size_t addressWidth, size_t valueWidth, const std::string &comment, unsigned flags) {
    return Leaf::createMemory(addressWidth, valueWidth, comment, flags);
}

Ptr
makeExistingMemory(size_t addressWidth, size_t valueWidth, uint64_t id, const std::string &comment, unsigned flags) {
    return Leaf::createExistingMemory(addressWidth, valueWidth, id, comment, flags);
}

Ptr
makeAdd(const Ptr&a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_ADD, a, b, comment, flags);
}

Ptr
makeBooleanAnd(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_AND, a, b, comment, flags);
}

Ptr
makeAsr(const Ptr &sa, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_ASR, sa, a, comment, flags);
}

Ptr
makeAnd(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_BV_AND, a, b, comment, flags);
}

Ptr
makeOr(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_BV_OR, a, b, comment, flags);
}

Ptr
makeXor(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_BV_XOR, a, b, comment, flags);
}
    
Ptr
makeConcat(const Ptr &hi, const Ptr &lo, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_CONCAT, hi, lo, comment, flags);
}

Ptr
makeEq(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_EQ, a, b, comment, flags);
}

Ptr
makeExtract(const Ptr &begin, const Ptr &end, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_EXTRACT, begin, end, a, comment, flags);
}

Ptr
makeInvert(const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_INVERT, a, comment, flags);
}

Ptr
makeIte(const Ptr &cond, const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_ITE, cond, a, b, comment, flags);
}

Ptr
makeLssb(const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_LSSB, a, comment, flags);
}

Ptr
makeMssb(const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_MSSB, a, comment, flags);
}

Ptr
makeNe(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_NE, a, b, comment, flags);
}

Ptr
makeNegate(const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_NEGATE, a, comment, flags);
}

Ptr
makeBooleanOr(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_OR, a, b, comment, flags);
}

Ptr
makeRead(const Ptr &mem, const Ptr &addr, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_READ, mem, addr, comment, flags);
}

Ptr
makeRol(const Ptr &sa, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_ROL, sa, a, comment, flags);
}

Ptr
makeRor(const Ptr &sa, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_ROR, sa, a, comment, flags);
}

Ptr
makeSet(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SET, a, b, comment, flags);
}

Ptr
makeSet(const Ptr &a, const Ptr &b, const Ptr &c, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SET, a, b, c, comment, flags);
}

Ptr
makeSignedDiv(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SDIV, a, b, comment, flags);
}

Ptr
makeSignExtend(const Ptr &newSize, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SEXTEND, newSize, a, comment, flags);
}

Ptr
makeSignedGe(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SGE, a, b, comment, flags);
}

Ptr
makeSignedGt(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SGT, a, b, comment, flags);
}

Ptr
makeShl0(const Ptr &sa, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SHL0, sa, a, comment, flags);
}

Ptr
makeShl1(const Ptr &sa, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SHL1, sa, a, comment, flags);
}

Ptr
makeShr0(const Ptr &sa, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SHR0, sa, a, comment, flags);
}

Ptr
makeShr1(const Ptr &sa, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SHR1, sa, a, comment, flags);
}

Ptr
makeSignedLe(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SLE, a, b, comment, flags);
}

Ptr
makeSignedLt(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SLT, a, b, comment, flags);
}

Ptr
makeSignedMod(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SMOD, a, b, comment, flags);
}

Ptr
makeSignedMul(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_SMUL, a, b, comment, flags);
}

Ptr
makeDiv(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_UDIV, a, b, comment, flags);
}

Ptr
makeExtend(const Ptr &newSize, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_UEXTEND, newSize, a, comment, flags);
}

Ptr
makeGe(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_UGE, a, b, comment, flags);
}

Ptr
makeGt(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_UGT, a, b, comment, flags);
}

Ptr
makeLe(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_ULE, a, b, comment, flags);
}

Ptr
makeLt(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_ULT, a, b, comment, flags);
}

Ptr
makeMod(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_UMOD, a, b, comment, flags);
}

Ptr
makeMul(const Ptr &a, const Ptr &b, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_UMUL, a, b, comment, flags);
}

Ptr
makeWrite(const Ptr &mem, const Ptr &addr, const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_WRITE, mem, addr, a, comment, flags);
}

Ptr
makeZerop(const Ptr &a, const std::string &comment, unsigned flags) {
    return Interior::create(0, OP_ZEROP, a, comment, flags);
}

} // namespace
} // namespace
} // namespace
