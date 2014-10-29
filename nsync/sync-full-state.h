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
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef SYNC_FULL_STATE_H
#define SYNC_FULL_STATE_H

#include <ndn-cxx/util/time.hpp>
#include "sync-state.h"

namespace Sync {

class FullState;
typedef shared_ptr<FullState> FullStatePtr;
typedef shared_ptr<FullState> FullStateConstPtr;


/**
 * \ingroup sync
 * @brief Cumulative SYNC state
 */
class FullState : public State
{
public:
  /**
   * @brief Default constructor
   */
  FullState ();
  virtual ~FullState ();

  /**
   * @brief Get time period since last state update
   *
   * This value can be used to randomize reconciliation waiting time in SyncApp
   */
  ndn::time::system_clock::Duration
  getTimeFromLastUpdate () const;

  /**
   * @brief Obtain a read-only copy of the digest
   *
   * If m_digest is 0, then it is automatically created.  On every update and removal, m_digest is reset to 0
   */
  DigestConstPtr
  getDigest ();

  // from State
  virtual tuple<bool/*inserted*/, bool/*updated*/, SeqNo/*oldSeqNo*/>
  update (NameInfoConstPtr info, const SeqNo &seq);

  virtual bool
  remove (NameInfoConstPtr info);

private:
  ndn::time::system_clock::TimePoint m_lastUpdated; ///< @brief Time when state was updated last time
  DigestPtr m_digest;
};

} // Sync

#endif // SYNC_STATE_H
