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

#ifndef SYNC_INTEREST_CONTAINER_H
#define SYNC_INTEREST_CONTAINER_H

#include <ndn-cxx/util/time.hpp>

#include "sync-digest.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/tag.hpp>
// #include <boost/multi_index/ordered_index.hpp>
// #include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
// #include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace mi = boost::multi_index;

namespace Sync {

struct Interest
{
  Interest (DigestConstPtr digest, const std::string &name, bool unknown=false)
  : m_digest (digest)
  , m_name (name)
  , m_time (ndn::time::system_clock::now())
  , m_unknown (unknown)
  {
  }

  DigestConstPtr   m_digest;
  std::string      m_name;
  ndn::time::system_clock::TimePoint m_time;
  bool             m_unknown;
};

/// @cond include_hidden
struct named { };
struct hashed;
struct timed;
/// @endcond

/**
 * \ingroup sync
 * @brief Container for interests (application PIT)
 */
struct InterestContainer : public mi::multi_index_container<
  Interest,
  mi::indexed_by<
    mi::hashed_unique<
      mi::tag<named>,
      BOOST_MULTI_INDEX_MEMBER(Interest, std::string, m_name)
    >
    ,

    mi::hashed_non_unique<
      mi::tag<hashed>,
      BOOST_MULTI_INDEX_MEMBER(Interest, DigestConstPtr, m_digest),
      DigestPtrHash,
      DigestPtrEqual
      >
    ,

    mi::ordered_non_unique<
      mi::tag<timed>,
      BOOST_MULTI_INDEX_MEMBER(Interest, ndn::time::system_clock::TimePoint, m_time)
      >
    >
  >
{
};

} // Sync

#endif // SYNC_INTEREST_CONTAINER_H
