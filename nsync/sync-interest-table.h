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

#ifndef SYNC_INTEREST_TABLE_H
#define SYNC_INTEREST_TABLE_H

#include <ndn-cxx/util/scheduler.hpp>

#include <string>
#include <vector>

#include "sync-digest.h"
#include "sync-interest-container.h"

namespace Sync {

/**
 * \ingroup sync
 * @brief A table to keep unanswered Sync Interest
 * all access operation to the table should grab the
 * mutex first
 */
class SyncInterestTable
{
public:
  SyncInterestTable (boost::asio::io_service& io, ndn::time::system_clock::Duration lifetime);
  ~SyncInterestTable ();

  /**
   * @brief Insert an interest, if interest already exists, update the
   * timestamp
   */
  bool
  insert (DigestConstPtr interest, const std::string &name, bool unknownState=false);

  /**
   * @brief Remove interest by digest (e.g., when it was satisfied)
   */
  bool
  remove (DigestConstPtr interest);

  /**
   * @brief Remove interest by name (e.g., when it was satisfied)
   */
  bool
  remove (const std::string &name);

  /**
   * @brief pop a non-expired Interest from PIT
   */
  Interest
  pop ();

  uint32_t
  size () const;

private:
  /**
   * @brief periodically called to expire Interest
   */
  void
  expireInterests ();

private:
  ndn::time::system_clock::Duration m_entryLifetime;
  InterestContainer m_table;

  ndn::Scheduler m_scheduler;
};

namespace Error {
struct InterestTableIsEmpty : virtual boost::exception, virtual std::exception { };
}

} // Sync

#endif // SYNC_INTEREST_TABLE_H
