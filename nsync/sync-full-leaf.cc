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

#include "sync-full-leaf.h"
#include <boost/ref.hpp>

using namespace boost;

namespace Sync {

FullLeaf::FullLeaf (NameInfoConstPtr info, const SeqNo &seq)
  : Leaf (info, seq)
{
  updateDigest ();
}

void
FullLeaf::updateDigest ()
{
  m_digest.reset ();
  m_digest << getInfo ()->getDigest () << *getSeq ().getDigest ();
  m_digest.finalize ();
}

// from Leaf
void
FullLeaf::setSeq (const SeqNo &seq)
{
  Leaf::setSeq (seq);
  updateDigest ();
}

} // Sync
