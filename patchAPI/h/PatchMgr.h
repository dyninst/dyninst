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

class PatchMgr : public dyn_detail::boost::enable_shared_from_this<PatchMgr> {
  friend class Point;

  public:
    PatchMgr(AddrSpacePtr as, PointMakerPtr pf, InstrumenterPtr inst);
    PATCHAPI_EXPORT virtual ~PatchMgr();
    PATCHAPI_EXPORT static PatchMgrPtr create(AddrSpacePtr as,
                              PointMakerPtr pf = PointMakerPtr(new PointMaker),
                              InstrumenterPtr inst = InstrumenterPtr());

    // Default implementation for filter function,
    // used in findPoins and removeSnippets
    template <class T>
    class IdentityFilterFunc {
      public:
        bool operator()(Point*, T) { return true;}
    };

    // Query Points:
    // users have the illusion that all points are already there, and
    // they just need to grab some by specifying a scope, one or more
    // point types, and a user-defined filter function
    // 1) Scope: a CFG structure that contains the desired points,
    //          e.g., Function, Block, ...
    // 2) Point types: We define several types, e.g., block entry, exit ...
    //                See also Point.h
    // 3) Filter function: User-defined function that further processes
    //                the set of condidate points
    //
    // return false if no any point found
    template <class Scope, class FilterFunc, class FilterArgument, class OutputIterator>
    PATCHAPI_EXPORT bool findPoints(Scope* scope,
                                    Point::Type types,
                                    FilterFunc filter_func,
                                    FilterArgument filter_arg,
                                    OutputIterator output_iter) {
      patch_cerr << ws2 << "Find points.\n";

      PointSet candidate_points;
      if (!findPointsByType(scope, types, candidate_points)) return false;
      patch_cerr << ws4 << "Get candidate points within Scope: "
                 << candidate_points.size() << " candidates found" << "\n";

      int pt_count = 0;
      for (PointIter p = candidate_points.begin();
           p != candidate_points.end(); p++) {
        if (filter_func(*p, filter_arg)) {
          *output_iter = *p;
          output_iter++;
          ++pt_count;
        }
      }
      patch_cerr << ws4 << "Apply the filter function on candidate points, "
                 <<  pt_count << " found\n";
      return true;
    }

    // Use default identity filter function
    template <class Scope, class OutputIterator>
    PATCHAPI_EXPORT bool findPoints(Scope* scope,
                                    Point::Type types,
                                    OutputIterator output_iter) {
      IdentityFilterFunc<char*> filter_func;
      char* dummy = NULL;
      return findPoints<Scope, IdentityFilterFunc<char*>, char*, OutputIterator>
             (scope, types, filter_func, dummy, output_iter);
    }

    // Explicit batch mode for snippet insertion and removal.
    // TODO(wenbin): Transactional semantics -- all succeed, or all fail

    // Return false on failure:
    //   Nested batchStart/batchFinish
    PATCHAPI_EXPORT bool batchStart();

    // Return false on failure:
    //   1) Broken batchStart/batchFinish pair
    //   2) Some insertion/removal fails
    template <class OutputIterator>
    PATCHAPI_EXPORT bool batchFinish(OutputIterator /*output_iter_for_failed_instances*/) {
      if (batch_mode_ != 1) return false;
      batch_mode_--;
      return patch();
    }

    // Snippet instance removal
    // Return false if no point if found
    PATCHAPI_EXPORT bool removeSnippet(InstancePtr);

    // Delete ALL snippets at certain points.
    // This uses the same filter-based interface as findPoints.
    template <class Scope, class FilterFunc, class FilterArgument>
    PATCHAPI_EXPORT bool removeSnippets(Scope* scope,
                                        Point::Type types,
                                        FilterFunc filter_func,
                                        FilterArgument filter_arg) {
      PointSet points;
      if (!findPoints(scope, types, filter_func, filter_arg,
          back_inserter(points) ) ) return false;

       for (PointIter p = points.begin(); p != points.end(); p++) {
         (*p)->clear();
       }
       return true;
    }

    // Use default identity filter function.
    template <class Scope>
    PATCHAPI_EXPORT bool removeSnippets(Scope* scope,
                                        Point::Type types) {
      IdentityFilterFunc<char*> filter_func;
      return removeSnippets<Scope, IdentityFilterFunc, char*>
                (scope, types, filter_func);
    }

    // Getters
    PATCHAPI_EXPORT AddrSpacePtr as() const { return as_; }
    PATCHAPI_EXPORT InstanceSet& getCurInstances() { return current_instances_; }
    PATCHAPI_EXPORT PointMakerPtr pointMaker() const { return point_maker_; }
    PATCHAPI_EXPORT InstrumenterPtr instrumenter() const { return instor_; } 
    //----------------------------------------------------
    // Mapping order: Scope -> Type -> Point Set
    // This order matches out filter sequence:
    // Apply Scope filter first, Type filter second,
    // finally filter function.
    //----------------------------------------------------

    // Type -> Point mapping
    // In a particular scope, a type may have multiple points,
    // e.g., function exits
    typedef std::map<Point::Type, PointSet> TypePtMap;
    typedef std::map<PatchFunction*, TypePtMap> FuncTypePtMap;
    typedef std::map<PatchBlock*, TypePtMap> BlkTypePtMap;
    typedef std::map<PatchEdge*, TypePtMap> EdgeTypePtMap;
    typedef std::map<Address, TypePtMap> AddrTypePtMap;

  private:
    // Return false if no point is found
    template<class Scope>
    friend bool findPointsByScopeType(PatchMgrPtr mgr, Scope* scope,
                                 Point::Type types, PointSet& points);
    template<class Scope>
    void getPointsByType(TypePtMap& type_pt_map, Point::Type types,
                         Point::Type type, Address addr,
                         Scope* scope, PointSet& points);
    bool findPointsByType(Address*, Point::Type, PointSet&);
    bool findPointsByType(PatchBlock*, Point::Type, PointSet&);
    bool findPointsByType(PatchEdge*, Point::Type, PointSet&);
    bool findPointsByType(PatchFunction*, Point::Type, PointSet&);

    // Core instrumentation function!
    bool patch();

    PointMakerPtr point_maker_;

    BlkTypePtMap blk_type_pt_map_;
    EdgeTypePtMap edge_type_pt_map_;
    FuncTypePtMap func_type_pt_map_;
    AddrTypePtMap addr_type_pt_map_;
    PointSet del_pt_set_;

    // The lifetime of it is the runtime of mutator
    InstanceSet current_instances_;

    // The lifetime of them is one batch
    /*
    InstanceSet insertion_set_;
    InstanceSet deletion_set_;
    FuncRepMap funcReplacement_;
    CallRepMap callReplacement_;
    CallRemoval callRemoval_;
    */
    InstrumenterPtr instor_;
    AddrSpacePtr as_;

    typedef int BatchMode;
    BatchMode batch_mode_;
};

}
}
#endif  // PATCHAPI_H_PATCHMGR_H_
