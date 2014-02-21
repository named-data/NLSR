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

#include "sync-diff-leaf.h"
#include <boost/throw_exception.hpp>
typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info; 

using namespace Sync::Error;

namespace Sync {

DiffLeaf::DiffLeaf (NameInfoConstPtr info, const SeqNo &seq)
  : Leaf (info, seq)
  , m_op (UPDATE)
{
}

DiffLeaf::DiffLeaf (NameInfoConstPtr info)
  : Leaf (info, SeqNo (0,0))
  , m_op (REMOVE)
{
}

std::ostream &
operator << (std::ostream &os, Operation op)
{
  switch (op)
    {
    case UPDATE:
      os << "update";
      break;
    case REMOVE:
      os << "remove";
      break;
    }
  return os;
}

std::istream &
operator >> (std::istream &is, Operation &op)
{
  std::string operation;
  is >> operation;
  if (operation == "update")
    op = UPDATE;
  else if (operation == "remove")
    op = REMOVE;
  else
    BOOST_THROW_EXCEPTION (SyncDiffLeafOperationParseError () << errmsg_info (operation));

  return is;
}


}
