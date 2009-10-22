#ifndef BG_AUXV_READER_H
#define BG_AUXV_READER_H

#include <string>
#include "auxvtypes.h"

#if defined(os_bg_ion) && defined(os_bgp)
#include "external/bluegene/bgp-debugger-interface.h"
#else
#error "BUILD ERROR: BG_AuxvReader.h compiled in non-bgp ion build."
#endif // defined(os_bg_ion) && defined(os_bgp)

// Type for elements in the auxiliary vector.
struct auxv_element {
  uint32_t type;
  uint32_t value;
  const char *name() { return auxv_type_to_string(type); }
};

///
/// Class for using CIO debugger interface to read auxv vectors.  This will read
/// all the vectors in order until there's nothing left.  Sample usage:
///
/// BG_AuxvReader reader(mypid);
/// while (reader.hasNext()) {
///   if (!reader.good()) {
///     // handle error
///   }
///   auxv_element elt = reader.next();
///   // do stuff with elt
/// }
///
/// Note that the process on the compute node *must be stopped* for this to work
/// properly.  User of this class is responsible for stopping/restarting the process.
///
class BG_AuxvReader {
private:
  const size_t buffer_size;                    /// buffer size for chunks read from CN
  const int pid;                               /// process to read from
  size_t fetch_offset;                         /// offset within aux vectors to fetch
  size_t read_offset;                          /// local offset in buffer to read from
  DebuggerInterface::BG_Debugger_Msg ack_msg;  /// BG msg containing fetched auxv buffer.

  const char *error;                                /// set if we encounter an error while reading.
  auxv_element elt;                            /// Last element read
  
  void check_buffer();                         // checks whether buffer needs refilling.

public:
  /// Construct a new reader for the specified process. PID is according to MPIR proctable.
  BG_AuxvReader(int _pid);
  ~BG_AuxvReader();

  bool good();              /// Whether we encountered an error communicating wtih CIOD
  const char *error_msg();  /// Message associated with the last error.
  bool has_next();          /// Whether there are more auxv_elements to read.
  auxv_element next();      /// Gets the next auxv element
};

#endif //BG_AUXV_READER_H
