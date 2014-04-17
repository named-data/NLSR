/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef SYNC_STATE_H
#define SYNC_STATE_H

#include "sync-state-leaf-container.h"
#include <boost/exception/all.hpp>
#include "boost/tuple/tuple.hpp"
#include "sync-state.pb.h"

/**
 * \defgroup sync SYNC protocol
 *
 * Implementation of SYNC protocol
 */
namespace Sync {

/**
 * \ingroup sync
 * @brief this prefix will be used for the dummy node which increases its sequence number whenever
 * a remove operation happens; this is to prevent the reversion of root digest when we prune 
 * a branch, i.e. help the root digest to be forward only
 * No corresponding data msg would be published and no attempt would be made to retrieve the 
 * data msg
 */
const std::string forwarderPrefix = "/d0n0t18ak/t0ps8cr8t";

class State;
typedef boost::shared_ptr<State> StatePtr;
typedef boost::shared_ptr<State> StateConstPtr;

/**
 * \ingroup sync
 * @brief Container for state leaves and definition of the abstract interface to work with State objects
 */
class State
{
public:
  virtual ~State () { };
  
  /**
   * @brief Add or update leaf to the state tree
   *
   * @param info name of the leaf
   * @param seq  sequence number of the leaf
   */
  virtual boost::tuple<bool/*inserted*/, bool/*updated*/, SeqNo/*oldSeqNo*/>
  update (NameInfoConstPtr info, const SeqNo &seq) = 0;

  /**
   * @brief Remove leaf from the state tree
   * @param info name of the leaf
   */
  virtual bool
  remove (NameInfoConstPtr info) = 0;

  /**
   * @brief Get state leaves
   */
  const LeafContainer &
  getLeaves () const 
  { return m_leaves; }
  
protected:
  LeafContainer m_leaves;
};


/**
 * @brief Formats a protobuf SyncStateMsg msg
 * @param oss output SyncStateMsg msg
 * @param state state
 * @returns output SyncStateMsg msg
 */
SyncStateMsg &
operator << (SyncStateMsg &ossm, const State &state);


/**
 * @brief Parse a protobuf SyncStateMsg msg
 * @param iss input SyncStateMsg msg
 * @param state state
 * @returns SyncStateMsg msg
 */
SyncStateMsg &
operator >> (SyncStateMsg &issm, State &state);

namespace Error {
/**
 * @brief Will be thrown when data cannot be properly decoded to SyncStateMsg
 */
struct SyncStateMsgDecodingFailure : virtual boost::exception, virtual std::exception { };
}

} // Sync

#endif // SYNC_STATE_H
