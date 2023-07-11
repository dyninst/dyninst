// WARNING: Changes to this file must be contributed back to Sawyer or else they will
//          be clobbered by the next update from Sawyer.  The Sawyer repository is at
//          https://github.com/matzke1/sawyer.




#ifndef Sawyer_Stopwatch_H
#define Sawyer_Stopwatch_H

#include <iosfwd>
#include "Sawyer.h"

#ifdef SAWYER_HAVE_BOOST_CHRONO
# include <boost/chrono/duration.hpp>
# include <boost/chrono/system_clocks.hpp>
#endif

namespace Sawyer {

/** Simple elapsed time.
 *
 * @code
 *  Stopwatch stopwatch; // starts immediately unless false argument is given
 *  do_something();
 *  std::cerr <<"that took " <<stopwatch.stop() <<" seconds.\n";
 * @endcode
 *
 *  All times are returned as floating point number of seconds.  The underlying data structure has nanosecond resolution
 *  although accuracy is probably nowhere near that.
 *
 *  A stopwatch can be started and stopped as often as desired and it will accumulate the total amount of time it has been in
 *  the running state (the clear() method resets the stopwatch by stopping it and zeroing the total).  Starting an already
 *  running stopwatch does nothing, and neither does stopping an already stopped stopwatch.
 *
 *  When a stopwatch is copied the destination stopwatch inherits the accumulated time and running state of the source. Both
 *  instances are then completely independent of one another.
 *
 *  Thread safety: These functions are not thread-safe, although multiple threads can invoke the methods concurrently on
 *  different objects.  It is permissible for different threads to invoke methods on the same object provided that
 *  synchronization occurs above the method calls. */
class SAWYER_EXPORT Stopwatch {
public:
#ifdef SAWYER_HAVE_BOOST_CHRONO
    typedef boost::chrono::high_resolution_clock::time_point TimePoint;
    typedef boost::chrono::duration<double> Duration;
#else
    typedef double TimePoint;
    typedef double Duration;
#endif

private:
#include "../util/WarningsOff.h"
    mutable TimePoint begin_;                           // time that this stopwatch (re)started
    mutable Duration elapsed_;                          // sum of elapsed run time in seconds
    bool running_;                                      // state of the stopwatch: running or not
#include "../util/WarningsRestore.h"

public:
    /** Construct and optionally start a timer.
     *
     *  The timer is started immediately unless the constructor is invoked with a false argument. */
    explicit Stopwatch(bool start=true): elapsed_(0), running_(false) {
        start && this->start();
    }

    /** Stop and reset the timer to the specified value.
     *
     *  The timer is stopped if it is running.  Returns the accumulated time before resetting it to the specified value. */
    double clear(double value=0.0);

    /** Start the timer and report accumulated time.
     *
     *  Reports the time accumulated as of this call and makes sure the clock is running.  If a value is specified then the
     *  stopwatch starts with that much time accumulated.
     *
     * @{ */
    double start();
    double start(double value);
    double start(bool b) SAWYER_DEPRECATED("use start() or start(double)") { return b ? start() : start(0.0); }
    /** @} */

    /** Restart the timer.
     *
     *  Reports the accumulated time as of this call, then resets it to zero and starts or restarts the clock running. */
    double restart();

    /** Stop the timer and report accumulated time.
     *
     *  If the timer is running then it is stopped and the elapsed time is accumulated, otherwise the timer
     *  remains in the stopped state an no additional time is accumulated.  If @p clear is set, then the accumlated time is
     *  reset to zero.  In any case, the return value is the accumulated time before being optionally reset to zero. */
    double stop(bool clear=false);

    /** Report current accumulated time without stopping or starting.
     *
     *  If @p clear is set then the accumulated time is reset to zero without affecting the return value or the running state,
     *  and the stopwatch starts reaccumulating time as of this call. */
    double report(bool clear=false) const;

    /** Query state of stopwatch.
     *
     *  Returns true if and only if the stopwatch is running. */
    bool isRunning() const { return running_; }
};


SAWYER_EXPORT std::ostream& operator<<(std::ostream&, const Stopwatch&);

} // namespace
#endif
