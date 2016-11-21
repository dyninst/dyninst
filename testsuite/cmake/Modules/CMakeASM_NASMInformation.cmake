
#=============================================================================
# Copyright 2010 Kitware, Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

# support for the nasm assembler

set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS nasm asm)

if(NOT CMAKE_ASM_NASM_OBJECT_FORMAT)
  if(WIN32)
    if(CMAKE_C_SIZEOF_DATA_PTR EQUAL 8)
      set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)
    else()
      set(CMAKE_ASM_NASM_OBJECT_FORMAT win32)
    endif()
  elseif(APPLE)
    if(CMAKE_C_SIZEOF_DATA_PTR EQUAL 8)
      set(CMAKE_ASM_NASM_OBJECT_FORMAT macho64)
    else()
      set(CMAKE_ASM_NASM_OBJECT_FORMAT macho)
    endif()
  else()
    if(CMAKE_C_SIZEOF_DATA_PTR EQUAL 8)
      set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)
    else()
      set(CMAKE_ASM_NASM_OBJECT_FORMAT elf)
    endif()
  endif()
endif()

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

# Load the generic ASMInformation file:
set(ASM_DIALECT "_NASM")
include(CMakeASMInformation)
set(ASM_DIALECT)
