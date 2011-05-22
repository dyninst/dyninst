/* Public Interface */

#ifndef PATCHAPI_H_PATCHMGR_H_
#define PATCHAPI_H_PATCHMGR_H_

#include "common.h"
#include "Point.h"
#include "Instrumenter.h"

namespace Dyninst {
namespace PatchAPI {

/* Interfaces for point query, snippet insertion and removal in batch
   mode */
class PatchMgr : public dyn_detail::boost::enable_shared_from_this<PatchMgr> {
  friend class Point;

  public:
    PatchMgr(AddrSpacePtr as, PointFactoryPtr pf);
    virtual ~PatchMgr() {}
    static PatchMgrPtr create(AddrSpacePtr as,
               PointFactoryPtr pf = PointFactoryPtr(new PointFactory));

    // Default implementation for filter function,
    // used in findPoins and removeSnippets
    class IdentityFilterFunc {
      public:
        bool operator()(PointPtr) { return true;}
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
    template <class Scope, class FilterFunc, class OutputIterator>
    bool findPoints(Scope* scope,
                    Point::PointType types,
                    FilterFunc filter_func,
                    OutputIterator output_iter) {
      patch_cerr << ws2 << "Find points.\n";

      PointSet candidate_points;
      if (!findPointsByType(scope, types, candidate_points)) return false;
      patch_cerr << ws4 << "Get candidate points within Scope: "
                 << candidate_points.size() << " candidates found" << "\n";

      int pt_count = 0;
      for (PointIter p = candidate_points.begin();
           p != candidate_points.end(); p++) {
        if (filter_func(*p)) {
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
    bool findPoints(Scope* scope,
                    Point::PointType types,
                    OutputIterator output_iter) {
      IdentityFilterFunc filter_func;
      return findPoints<Scope, IdentityFilterFunc, OutputIterator>
                    (scope, types, filter_func, output_iter);
    }

    // Explicit batch mode for snippet insertion and removal.
    // TODO(wenbin): Transactional semantics -- all succeed, or all fail

    // Return false on failure:
    //   Nested batchStart/batchFinish
    bool batchStart();

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
    bool removeSnippet(InstancePtr);

    // Delete ALL snippets at certain points.
    // This uses the same filter-based interface as findPoints.
    template <class Scope, class FilterFunc>
    bool removeSnippets(Scope* scope,
                    Point::PointType types,
                    FilterFunc filter_func) {
      PointSet points;
      if (!findPoints(scope, types, filter_func,
          inserter(points, points.begin()))) return false;

       for (PointIter p = points.begin(); p != points.end(); p++) {
         (*p)->clear();
       }
       return true;
    }

    // Use default identity filter function.
    template <class Scope>
    bool removeSnippets(Scope* scope,
                    Point::PointType types) {
      IdentityFilterFunc filter_func;
      return removeSnippets<Scope, IdentityFilterFunc>
                (scope, types, filter_func);
    }

    // Code modification interfaces
    bool removeFuncCall(PointPtr point);
    bool replaceFuncCall(PointPtr point, PatchFunction* func);
    bool replaceFunction(PatchFunction* old_func,
                         PatchFunction* new_func);

    // Getters
    AddrSpacePtr as() const { return as_; }
    InstanceSet& getCurInstances() { return current_instances_; }


    //----------------------------------------------------
    // Mapping order: Scope -> PointType -> Point Set
    // This order matches out filter sequence:
    // Apply Scope filter first, PointType filter second,
    // finally filter function.
    //----------------------------------------------------

    // PointType -> Point mapping
    // In a particular scope, a type may have multiple points,
    // e.g., function exits
    typedef std::map<Point::PointType, PointSet> TypePtMap;
    typedef std::map<PatchFunction*, TypePtMap> FuncTypePtMap;
    typedef std::map<PatchBlock*, TypePtMap> BlkTypePtMap;
    typedef std::map<PatchEdge*, TypePtMap> EdgeTypePtMap;
    typedef std::map<Address, TypePtMap> AddrTypePtMap;

  private:
    // Return false if no point is found
    template<class Scope>
    friend bool findPointsByScopeType(PatchMgrPtr mgr, Scope* scope,
                                 Point::PointType types, PointSet& points);
    template<class Scope>
    void getPointsByType(TypePtMap& type_pt_map, Point::PointType types,
                         Point::PointType type, Address addr,
                         Scope* scope, PointSet& points);
    bool findPointsByType(Address*, Point::PointType, PointSet&);
    bool findPointsByType(PatchBlock*, Point::PointType, PointSet&);
    bool findPointsByType(PatchEdge*, Point::PointType, PointSet&);
    bool findPointsByType(PatchFunction*, Point::PointType, PointSet&);

    // Core instrumentation function!
    bool patch();

    PointFactoryPtr point_factory_;

    BlkTypePtMap blk_type_pt_map_;
    EdgeTypePtMap edge_type_pt_map_;
    FuncTypePtMap func_type_pt_map_;
    AddrTypePtMap addr_type_pt_map_;

    // The lifetime of it is the runtime of mutator
    InstanceSet current_instances_;

    // The lifetime of them is one batch
    InstanceSet insertion_set_;
    InstanceSet deletion_set_;
    FuncRepMap funcReplacement_;
    CallRepMap callReplacement_;
    CallRemoval callRemoval_;

    InstrumenterPtr instor_;
    AddrSpacePtr as_;

    typedef int BatchMode;
    BatchMode batch_mode_;
};

}
}
#endif  // PATCHAPI_H_PATCHMGR_H_
