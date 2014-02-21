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

#ifndef SYNC_STATE_LEAF_CONTAINER
#define SYNC_STATE_LEAF_CONTAINER

#include "sync-leaf.h"
#include "sync-name-info.h"

#include <boost/multi_index_container.hpp>
// #include <boost/multi_index/tag.hpp>
#include <boost/multi_index/ordered_index.hpp>
// #include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
// #include <boost/multi_index/random_access_index.hpp>
// #include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>

namespace mi = boost::multi_index;

namespace Sync {

struct NameInfoHash : public std::unary_function<NameInfo, std::size_t>
{
  std::size_t
  operator() (NameInfoConstPtr prefix) const
  {
    return prefix->getHashId ();
  }
};

struct NameInfoEqual : public std::unary_function<NameInfo, std::size_t>
{
  bool
  operator() (NameInfoConstPtr prefix1, NameInfoConstPtr prefix2) const
  {
    return *prefix1 == *prefix2;
  }
};

struct NameInfoCompare : public std::unary_function<NameInfo, std::size_t>
{
  bool
  operator() (NameInfoConstPtr prefix1, NameInfoConstPtr prefix2) const
  {
    return *prefix1 < *prefix2;
  }
};

/// @cond include_hidden 
struct hashed { };
struct ordered { };
/// @endcond

/**
 * \ingroup sync
 * @brief Container for SYNC leaves
 */
struct LeafContainer : public mi::multi_index_container<
  LeafPtr,
  mi::indexed_by<
    // For fast access to elements using NameInfo
    mi::hashed_unique<
      mi::tag<hashed>,
      mi::const_mem_fun<Leaf, NameInfoConstPtr, &Leaf::getInfo>,
      NameInfoHash,
      NameInfoEqual
      >,
        
    mi::ordered_unique<
      mi::tag<ordered>,
      mi::const_mem_fun<Leaf, NameInfoConstPtr, &Leaf::getInfo>,
      NameInfoCompare
      >
    >
  >
{
};

} // Sync

#endif // SYNC_STATE_LEAF_CONTAINER
