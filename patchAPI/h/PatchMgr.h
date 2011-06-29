/* Public Interface */

#ifndef PATCHAPI_H_PATCHMGR_H_
#define PATCHAPI_H_PATCHMGR_H_

#include "common.h"
#include "Point.h"
#include "Instrumenter.h"
#include "PatchCFG.h"

namespace Dyninst {
namespace PatchAPI {

/* Interfaces for point query, snippet insertion and removal in batch
   mode */

class PatchObject;
class PatchMgr;
class PatchFunction;
class PatchBlock;
class PatchCallback;

struct Scope {
   PatchObject *obj;
   PatchFunction *func;
   PatchBlock *block;
   bool wholeProgram;
Scope(PatchBlock *b) : obj(NULL), func(NULL), block(b), wholeProgram(false) {};
Scope(PatchFunction *f, PatchBlock *b) : obj(NULL), func(f), block(b), wholeProgram(false) {};
Scope(PatchFunction *f) : obj(NULL), func(f), block(NULL), wholeProgram(false) {};
};


class PatchMgr : public dyn_detail::boost::enable_shared_from_this<PatchMgr> {
  friend class Point;
  friend class PatchObject; // for splitting blocks as that is _not_ public.
  typedef std::pair<PatchFunction *, PatchBlock *> BlockInstance;
  typedef std::pair<PatchFunction *, InsnLoc> InsnInstance;
  typedef std::vector<PatchFunction *> Functions;
  typedef std::vector<PatchBlock *> Blocks;
  typedef std::vector<PatchEdge *> Edges;
  typedef std::vector<BlockInstance> BlockInstances;
  typedef std::vector<InsnInstance> InsnInstances;
  typedef std::vector<CallSite> CallSites;
  typedef std::vector<ExitSite> ExitSites;
  typedef std::vector<InsnLoc> Insns;
  typedef std::vector<Point::Type> EnumeratedTypes;

  public:
  typedef std::pair<Location, Point::Type> Candidate;
  typedef std::vector<Candidate> Candidates;

  PatchMgr(AddrSpacePtr as, PointMakerPtr pf, InstrumenterPtr inst, PatchCallback *);
    PATCHAPI_EXPORT virtual ~PatchMgr();
    PATCHAPI_EXPORT static PatchMgrPtr create(AddrSpacePtr as,
                                              PointMakerPtr pf = PointMakerPtr(new PointMaker),
                                              InstrumenterPtr inst = InstrumenterPtr(),
                                              PatchCallback *cb = NULL);

    // Default implementation for filter function,
    // used in findPoins and removeSnippets
    template <class T>
    class IdentityFilterFunc {
      public:
       bool operator()(Point::Type, Location, T) { return true;}
    };

    // Query Points:

    // Direct interface; specify a Location and a unique Type, receive a Point.
    Point *findPoint(Location loc,
                     Point::Type type,
                     bool create = true);
    // And accumulation version
    template <class OutputIterator>
    bool findPoint(Location loc,
                   Point::Type type,
                   OutputIterator outputIter,
                   bool create = true);

    // Group interface with one degree of freedom: the Type can be a composition.
    template <class FilterFunc, class FilterArgument, class OutputIterator> 
    bool findPoints(Location loc,
                    Point::Type types,
                    FilterFunc filter_func,
                    FilterArgument filter_arg,
                    OutputIterator outputIter,
                    bool create = true);

    template <class OutputIterator> 
    bool findPoints(Location loc,
                    Point::Type types,
                    OutputIterator outputIter,
                    bool create = true);

    // Group interface with two degrees of freedom: Locations are wildcarded
    // and Type can be a composition. Instead, users provide a Scope and a 
    // FilterFunc that guide which Locations will be considered. 
    template <class FilterFunc, class FilterArgument, class OutputIterator>
    bool findPoints(Scope scope,
                    Point::Type types,
                    FilterFunc filter_func,
                    FilterArgument filter_arg,
                    OutputIterator output_iter, 
                    bool create = true);

    // Use default identity filter function
    template <class OutputIterator>
    bool findPoints(Scope scope,
                    Point::Type types,
                    OutputIterator output_iter, 
                    bool create = true);

    // Explicit batch mode for snippet insertion and removal.
    // TODO(wenbin): Transactional semantics -- all succeed, or all fail

    // Return false on failure:
    //   Nested batchStart/batchFinish
    PATCHAPI_EXPORT bool batchStart();

    // Return false on failure:
    //   1) Broken batchStart/batchFinish pair
    //   2) Some insertion/removal fails
    template <class OutputIterator>
    bool batchFinish(OutputIterator /*output_iter_for_failed_instances*/) {
      if (batch_mode_ != 1) return false;
      batch_mode_--;
      return patch();
    }

    // Snippet instance removal
    // Return false if no point if found
    PATCHAPI_EXPORT bool removeSnippet(InstancePtr);

    // Delete ALL snippets at certain points.
    // This uses the same filter-based interface as findPoints.
    template <class FilterFunc, class FilterArgument>
    bool removeSnippets(Scope scope,
                        Point::Type types,
                        FilterFunc filter_func,
                        FilterArgument filter_arg) {
      PointSet points;
      if (!findPoints(scope, types, filter_func, filter_arg,
                      inserter(points, points.begin()) ) ) return false;

       for (PointIter p = points.begin(); p != points.end(); p++) {
         (*p)->clear();
       }
       return true;
    }

    PATCHAPI_EXPORT void destroy(Point *);

    // Use default identity filter function.
    bool removeSnippets(Scope scope,
                        Point::Type types) {
      IdentityFilterFunc<char*> filter_func;
      char *foo = NULL;
      return removeSnippets<IdentityFilterFunc<char *>, char*>(scope, types, filter_func, foo);
    }

    // Getters
    PATCHAPI_EXPORT AddrSpacePtr as() const { return as_; }
    PATCHAPI_EXPORT PointMakerPtr pointMaker() const { return point_maker_; }
    PATCHAPI_EXPORT InstrumenterPtr instrumenter() const { return instor_; }
    PATCHAPI_EXPORT PatchCallback *cb() const { return cb_; }
    //----------------------------------------------------
    // Mapping order: Scope -> Type -> Point Set
    //   The Scope x Type provides us a list of matching locations;
    //   we then filter those locations. Points are stored in
    //   their contexts (e.g., Functions or Blocks). 
    //----------------------------------------------------

  private:
    bool getCandidates(Scope &, Point::Type types, Candidates &ret);

    bool findInsnPointsByType(Location *, Point::Type, PointSet&, bool create = true);
    bool findBlockPointsByType(Location *, Point::Type, PointSet&, bool create = true);
    bool findEdgePointsByType(Location *, Point::Type, PointSet&, bool create = true);
    bool findFuncPointsByType(Location *, Point::Type, PointSet&, bool create = true);
    bool findFuncSitePointsByType(Location *, Point::Type, PointSet&, bool create = true);

    bool wantFuncs(Scope &scope, Point::Type types);
    bool wantCallSites(Scope &scope, Point::Type types);
    bool wantExitSites(Scope &scope, Point::Type types);
    bool wantBlocks(Scope &scope, Point::Type types);
    bool wantEdges(Scope &scope, Point::Type types);
    bool wantInsns(Scope &scope, Point::Type types);
    bool wantBlockInstances(Scope &scope, Point::Type types);
    //bool wantEdgeInstances(Scope &scope, Point::Type types);
    bool wantInsnInstances(Scope &scope, Point::Type types);

    void getFuncCandidates(Scope &scope, Point::Type types, Candidates &ret);
    void getCallSiteCandidates(Scope &scope, Point::Type types, Candidates &ret);
    void getExitSiteCandidates(Scope &scope, Point::Type types, Candidates &ret);
    void getBlockCandidates(Scope &scope, Point::Type types, Candidates &ret);
    void getEdgeCandidates(Scope &scope, Point::Type types, Candidates &ret);
    void getInsnCandidates(Scope &scope, Point::Type types, Candidates &ret);
    void getBlockInstanceCandidates(Scope &scope, Point::Type types, Candidates &ret);
    //bool getEdgeInstanceCandidates(Scope &scope, Point::Type types, Candidates &ret);
    void getInsnInstanceCandidates(Scope &scope, Point::Type types, Candidates &ret);

    void getFuncs(Scope &scope, Functions &funcs);
    void getCallSites(Scope &scope, CallSites &Sites);
    void getExitSites(Scope &scope, ExitSites &Sites);
    void getBlocks(Scope &scope, Blocks &blocks);
    void getEdges(Scope &scope, Edges &edges);
    void getInsns(Scope &scope, Insns &insns);
    void getBlockInstances(Scope &scope, BlockInstances &blocks);
    //bool getEdgeInstances(Scope &scope, EdgeCandidates &edges);
    void getInsnInstances(Scope &scope, InsnInstances &insns);

    void enumerateTypes(Point::Type types, EnumeratedTypes &out);

    // Core instrumentation function!
    PATCHAPI_EXPORT bool patch();

    bool match(Point *, Location *);
    void updatePointsForBlockSplit(PatchBlock *oldBlock, PatchBlock *newBlock);
    PointMakerPtr point_maker_;
    InstrumenterPtr instor_;
    AddrSpacePtr as_;

    typedef int BatchMode;
    BatchMode batch_mode_;

    PatchCallback *cb_;
};


template <class FilterFunc, class FilterArgument, class OutputIterator>
bool PatchMgr::findPoints(Scope scope,
                          Point::Type types,
                          FilterFunc filter_func,
                          FilterArgument filter_arg,
                          OutputIterator output_iter, 
                          bool create) {
   Candidates candidates;
   if (!getCandidates(scope, types, candidates)) return false;
   for (Candidates::iterator c = candidates.begin();
        c != candidates.end(); ++c) {
      if (filter_func(c->second, c->first, filter_arg)) {
         if (!findPoint(c->first, c->second, output_iter, create)) {
            return false;
         }
      }
   }
   return true;
};

template <class FilterFunc, class FilterArgument, class OutputIterator> 
bool PatchMgr::findPoints(Location loc, Point::Type types,
                FilterFunc filter_func, FilterArgument filter_arg,
                OutputIterator outputIter, bool create) {
   EnumeratedTypes tmp;
   enumerateTypes(types, tmp);
   for (EnumeratedTypes::iterator iter = tmp.begin();
        iter != tmp.end(); ++iter) {
      if (filter_func(*iter, loc, filter_arg)) {
         if (!findPoint(loc, *iter, outputIter, create)) return false;
      }
   }
   return true;
}

template <class OutputIterator>
bool PatchMgr::findPoints(Location loc, Point::Type types,
                          OutputIterator outputIter, bool create) {
   IdentityFilterFunc<char *> filter_func;
   char *dummy = NULL;
   return findPoints(loc, types, filter_func, dummy, outputIter, create);
};

// Use default identity filter function
template <class OutputIterator>
bool PatchMgr::findPoints(Scope scope,
                          Point::Type types,
                          OutputIterator output_iter, 
                          bool create) {
   IdentityFilterFunc<char*> filter_func;
   char* dummy = NULL;
   return findPoints<IdentityFilterFunc<char*>, char*, OutputIterator>
      (scope, types, filter_func, dummy, output_iter, create);
};

template <class OutputIterator>
bool PatchMgr::findPoint(Location loc, Point::Type type,
               OutputIterator outputIter,
               bool create) {
   Point *p = findPoint(loc, type, create);
   if (p) {
      *outputIter = p;
      ++outputIter;
   }
   return (!create || p);
};
            

};
};



#endif  // PATCHAPI_H_PATCHMGR_H_

