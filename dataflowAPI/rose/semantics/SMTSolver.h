#ifndef Rose_SMTSolver_H
#define Rose_SMTSolver_H

#include <iosfwd>
#include <set>
#include <stddef.h>
#include <string>
#include <vector>
#include "BinarySymbolicExpr.h"
#include <boost/thread/mutex.hpp>

/* Enable type formatting macros, not enabled by default in C++ */
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>

namespace rose {
    namespace BinaryAnalysis {

/** Interface to Satisfiability Modulo Theory (SMT) solvers.
 *
 *  The purpose of an SMT solver is to determine if an expression is satisfiable. */
        class SMTSolver {
        public:
            struct Exception {
                Exception(const std::string &mesg_) : mesg(mesg_) { }

                friend std::ostream &operator<<(std::ostream &, const SMTSolver::Exception &);

                std::string mesg;
            };

            /** Satisfiability constants. */
            enum Satisfiable {
                SAT_NO = 0, /**< Provably unsatisfiable. */
                        SAT_YES, /**< Satisfiable and evidence of satisfiability may be available. */
                        SAT_UNKNOWN              /**< Could not be proved satisfiable or unsatisfiable. */
            };

            /** SMT solver statistics. */
            struct Stats {
                Stats() : ncalls(0), input_size(0), output_size(0) { }

                size_t ncalls;
                /**< Number of times satisfiable() was called. */
                size_t input_size;
                /**< Bytes of input generated for satisfiable(). */
                size_t output_size;                     /**< Amount of output produced by the SMT solver. */
            };

            typedef std::set <uint64_t> Definitions;

            /**< Free variables that have been defined. */

            SMTSolver() : debug(NULL) { init(); }

            virtual ~SMTSolver() { }

            /** Determines if expressions are trivially satisfiable or unsatisfiable.  If all expressions are known 1-bit values that
             *  are true, then this function returns SAT_YES.  If any expression is a known 1-bit value that is false, then this
             *  function returns SAT_NO.  Otherwise this function returns SAT_UNKNOWN. */
            virtual Satisfiable trivially_satisfiable(const std::vector <SymbolicExpr::Ptr> &exprs);

            /** Determines if the specified expressions are all satisfiable, unsatisfiable, or unknown.
             * @{ */
            virtual Satisfiable satisfiable(const SymbolicExpr::Ptr &);

            virtual Satisfiable satisfiable(const std::vector <SymbolicExpr::Ptr> &);

            virtual Satisfiable satisfiable(std::vector <SymbolicExpr::Ptr>, const SymbolicExpr::Ptr &);
            /** @} */



            /** Evidence of satisfiability for a bitvector variable.  If an expression is satisfiable, this function will return
             *  a value for the specified bitvector variable that satisfies the expression in conjunction with the other evidence. Not
             *  all SMT solvers can return this information.  Returns the null pointer if no evidence is available for the variable.
             * @{ */
            virtual SymbolicExpr::Ptr evidence_for_variable(uint64_t varno) {
                char buf[64];
                //FIXME
                snprintf(buf, sizeof(buf), "v%" PRIu64, varno);
                return evidence_for_name(buf);
            }

            virtual SymbolicExpr::Ptr evidence_for_variable(const SymbolicExpr::Ptr &var) {
                SymbolicExpr::LeafPtr ln = var->isLeafNode();
                ASSERT_require(ln && !ln->isNumber());
                return evidence_for_variable(ln->nameId());
            }
            /** @} */

            /** Evidence of satisfiability for a memory address.  If an expression is satisfiable, this function will return
             *  a value for the specified memory address that satisfies the expression in conjunction with the other evidence. Not
             *  all SMT solvers can return this information. Returns the null pointer if no evidence is available for the memory
             *  address. */
            virtual SymbolicExpr::Ptr evidence_for_address(uint64_t addr);

            /** Evidence of satisfiability for a variable or memory address.  If the string starts with the letter 'v' then variable
             *  evidence is returned, otherwise the string must be an address.  The strings are those values returned by the
             *  evidence_names() method.  Not all SMT solvers can return this information.  Returns the null pointer if no evidence is
             *  available for the named item. */
            virtual SymbolicExpr::Ptr evidence_for_name(const std::string &) {
                return SymbolicExpr::Ptr();
            }

            /** Names of items for which satisfiability evidence exists.  Returns a vector of strings (variable names or memory
             * addresses) that can be passed to evidence_for_name().  Not all SMT solvers can return this information. */
            virtual std::vector <std::string> evidence_names() {
                return std::vector<std::string>();
            }

            /** Clears evidence information. */
            virtual void clear_evidence() { }

            /** Turns debugging on or off. */
            void set_debug(FILE *f) { debug = f; }

            /** Obtain current debugging setting. */
            FILE *get_debug() const { return debug; }

            /** Returns statistics for this solver. The statistics are not reset by this call, but continue to accumulate. */
            const Stats &get_stats() const { return stats; }

            /** Returns statistics for all solvers. The statistics are not reset by this call, but continue to accumulate. */
            static Stats get_class_stats();

            /** Resets statistics for this solver. */
            void reset_stats() { stats = Stats(); }

            /** Resets statistics for the class.  Statistics are reset to initial values for the class as a whole.  Resetting
             * statistics for the class does not affect statistics of any particular SMT object. */
            void reset_class_stats();

        protected:
            /** Generates an input file for for the solver. Usually the input file will be SMT-LIB format, but subclasses might
             *  override this to generate some other kind of input. Throws Excecption if the solver does not support an operation that
             *  is necessary to determine the satisfiability. */
            virtual void generate_file(std::ostream &, const std::vector <SymbolicExpr::Ptr> &exprs,
                                       Definitions *) = 0;

            /** Given the name of a configuration file, return the command that is needed to run the solver. The first line
             *  of stdout emitted by the solver should be the word "sat" or "unsat". */
            virtual std::string get_command(const std::string &config_name) = 0;

            /** Parses evidence of satisfiability.  Some solvers can emit information about what variable bindings satisfy the
             *  expression.  This information is parsed by this function and added to a mapping of variable to value. */
            virtual void parse_evidence() { }

            /** Additional output obtained by satisfiable(). */
            std::string output_text;

            // Statistics
            static boost::mutex class_stats_mutex;
            static Stats class_stats;                   // all access must be protected by class_stats_mutex
            Stats stats;

        private:
            FILE *debug;

            void init();
        };

    } // namespace
} // namespace

#endif
