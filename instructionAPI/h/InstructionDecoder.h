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

#if !defined(INSTRUCTION_DECODER_H)
#define INSTRUCTION_DECODER_H

#include "Instruction.h"
#include <stddef.h>

namespace Dyninst
{
  namespace InstructionAPI
  {
    /// The %InstructionDecoder class decodes instructions, given a buffer of bytes and a length, and
    /// the architecture for which to decode instructions,
    /// and constructs shared pointers to %Instruction objects representing those instructions.
    /// %InstructionDecoder objects are given a buffer from which to decode at construction.
    /// Calls to \c decode will proceed to decode instructions sequentially from that buffer until its
    /// end is reached.  At that point, all subsequent calls to \c decode will return a null %Instruction pointer.
    ///
      class InstructionDecoderImpl;

    class INSTRUCTION_EXPORT InstructionDecoder
    {
      friend class Instruction;
        public:
        static const unsigned int maxInstructionLength = 16;
      /// Construct an %InstructionDecoder object that decodes \c arch from \c buffer, up to \c size bytes.
      /// Valid values for \c arch are \c Arch_x86, \c Arch_x86_64, \c Arch_ppc32, and \c Arch_ppc64.
      InstructionDecoder(const unsigned char* buffer, size_t size, Architecture arch);
      InstructionDecoder(const void* buffer, size_t size, Architecture arch);

      INSTRUCTION_EXPORT ~InstructionDecoder() = default;
      INSTRUCTION_EXPORT InstructionDecoder(const InstructionDecoder& o) = default;
      INSTRUCTION_EXPORT InstructionDecoder& operator=(const InstructionDecoder  & o) = default;
      /// Decode the current instruction in this %InstructionDecoder object's buffer, interpreting it as
      /// machine language of the type understood by this %InstructionDecoder.
      /// If the buffer does not contain a valid instruction stream, a null %Instruction pointer
      /// will be returned.  The %Instruction's \c size field will contain the size of the instruction decoded.
      Instruction decode();
      /// Decode the instruction at \c buffer, interpreting it as machine language of the type
      /// understood by this %InstructionDecoder.  If the buffer does not contain a valid instruction stream,
      /// a null %Instruction pointer will be returned.  The %Instruction's \c size field will contain
      /// the size of the instruction decoded.
      Instruction decode(const unsigned char *buffer);
      void doDelayedDecode(const Instruction* insn_to_complete);
      struct INSTRUCTION_EXPORT buffer
      {
          const unsigned char* start;
          const unsigned char* end;
          buffer(const unsigned char* b, unsigned int len) :
                  start(b), end(b+len) {}
          buffer(const void* b, unsigned int len) :
	start(reinterpret_cast<const unsigned char*>(b)), end(start+len) {}
          buffer(const unsigned char* b, const unsigned char* e) :
                  start(b), end(e) {}
      };

      // This interface allows registering a callback function to be invoked
      // when the InstructionDecoder encounters a byte sequence it is not able
      // to successfully convert into a known instruction
      struct unknown_instruction {
		  using callback_t = Instruction(*)(buffer);
		  static void register_callback(callback_t);
		  static callback_t unregister_callback();
		  unknown_instruction() = delete;
		  ~unknown_instruction() = delete;
      };

        private:
            buffer m_buf;
      boost::shared_ptr<InstructionDecoderImpl> m_Impl;
    };

  }
}

#endif //!defined(INSTRUCTION_DECODER_H)
