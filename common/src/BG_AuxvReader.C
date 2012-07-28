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
#include "BG_AuxvReader.h"
#include "auxvtypes.h"
using namespace DebuggerInterface;

#include <cstring>
#include <sstream>
using namespace std;

// these defines are for readability 
#define AUX(msg) (msg).dataArea.GET_AUX_VECTORS
#define ACK(msg) (msg).dataArea.GET_AUX_VECTORS_ACK

BG_AuxvReader::BG_AuxvReader(int _pid)
  : buffer_size(BG_Debugger_AUX_VECS_BUFFER), 
    pid(_pid), 
    fetch_offset(0), 
    read_offset(0), 
    ack_msg(GET_AUX_VECTORS_ACK, pid, 0, 0, 0),
    error(NULL)
{
  // set this up to trigger an initial fetch of some data.
  ACK(ack_msg).auxVecBufferLength = 0;
  ACK(ack_msg).endOfVecData = false;
  elt.type = AT_IGNORE;
}


void BG_AuxvReader::check_buffer() {
  if (read_offset < ACK(ack_msg).auxVecBufferLength 
      || ACK(ack_msg).endOfVecData) {
    return;
  }
  
  // get a single auxv element
  BG_Debugger_Msg get_aux_msg(GET_AUX_VECTORS, pid, 0, 0, 0);
  get_aux_msg.header.dataLength = sizeof(get_aux_msg.dataArea.GET_AUX_VECTORS);
  get_aux_msg.dataArea.GET_AUX_VECTORS.auxVecBufferOffset = fetch_offset;
  get_aux_msg.dataArea.GET_AUX_VECTORS.auxVecBufferLength = buffer_size;

  if (!BG_Debugger_Msg::writeOnFd(BG_DEBUGGER_WRITE_PIPE, get_aux_msg)) {
    error = "Error writing GET_AUX_VECTORS to BG debug stream.";
    return;
  }

  // wait for the ack (and the buffer that goes with it)
  // make sure we got the right kind of ack, too.
  ack_msg = BG_Debugger_Msg(GET_AUX_VECTORS_ACK, pid, 0, 0, 0);
  ack_msg.header.dataLength = sizeof(ack_msg.dataArea.GET_AUX_VECTORS_ACK);
  if (!BG_Debugger_Msg::readFromFd(BG_DEBUGGER_READ_PIPE, ack_msg)) {
    error = "Error reading from BG debug stream.";
    return;
  }

  if (ack_msg.header.messageType != GET_AUX_VECTORS_ACK) {
    ostringstream err;
    err << "Got invalid response from BG debug stream.  Expected GET_AUX_VECTORS_ACK, but got ";
    err << BG_Debugger_Msg::getMessageName(ack_msg.header.messageType);
    err << ".";
    string errstr = err.str();
    error = strdup(errstr.c_str());
    return;
  }

  // sanity check to make sure the bounds are ok in the response.
  if (ACK(ack_msg).auxVecBufferLength  > AUX(get_aux_msg).auxVecBufferLength ||
      ACK(ack_msg).auxVecBufferOffset != AUX(get_aux_msg).auxVecBufferOffset) 
  {
    error = "Sanity check on GET_AUX_VECTORS_ACK failed.";
    return;
  }

  // set offsets so we can read what we just fetched.
  fetch_offset += buffer_size;
  read_offset = 0;
}


BG_AuxvReader::~BG_AuxvReader() { }


bool BG_AuxvReader::good() {
  return !error;//.length();
}
  

const char *BG_AuxvReader::error_msg() {
  return error;//.c_str();
}
  

bool BG_AuxvReader::has_next() {
  check_buffer();
  if (error/*.length()*/) return false;

  return elt.type != AT_NULL 
    &&   read_offset < ACK(ack_msg).auxVecBufferLength;
}
  

auxv_element BG_AuxvReader::next() {
  // check here too, just in case someone decides they *know* we have a next element.
  check_buffer();

  elt.type  = ACK(ack_msg).auxVecData[read_offset++];
  elt.value = ACK(ack_msg).auxVecData[read_offset++];

  return elt;
}

