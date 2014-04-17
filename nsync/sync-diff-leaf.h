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

#ifndef SYNC_DIFF_LEAF_H
#define SYNC_DIFF_LEAF_H

#include "sync-leaf.h"
#include <boost/exception/all.hpp>

namespace Sync {

/**
 * @ingroup sync
 * @brief Annotation for SYNC leaf
 */
enum Operation
  {
    UPDATE, ///< @brief Leaf was added or updated
    REMOVE  ///< @brief Leaf was removed
  };

/**
 * @ingroup sync
 * @brief Annotated SYNC leaf 
 */
class DiffLeaf : public Leaf
{
public:
  /**
   * @brief Constructor to create an UPDATE diff leaf
   * @param info Smart pointer to leaf's name
   * @param seq  Initial sequence number of the pointer
   */
  DiffLeaf (NameInfoConstPtr info, const SeqNo &seq);

  /**
   * @brief Constructor to create an REMOVE diff leaf
   * @param info Smart pointer to leaf's name
   *
   * This constructor creates a leaf with phony sequence number
   * with 0 session ID and 0 sequence number
   */
  DiffLeaf (NameInfoConstPtr info);

  virtual ~DiffLeaf () { }

  /**
   * @brief Get diff leaf type
   */
  Operation
  getOperation () const { return m_op; }

private:
  Operation m_op;
};

typedef boost::shared_ptr<DiffLeaf> DiffLeafPtr;
typedef boost::shared_ptr<const DiffLeaf> DiffLeafConstPtr;

std::ostream &
operator << (std::ostream &os, Operation op);

std::istream &
operator >> (std::istream &is, Operation &op);

namespace Error {
struct SyncDiffLeafOperationParseError : virtual boost::exception, virtual std::exception { };
} // Error

} // Sync

#endif // SYNC_DIFF_LEAF_H
