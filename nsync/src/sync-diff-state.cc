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

#include "sync-diff-state.h"
#include "sync-diff-leaf.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/assert.hpp>

using namespace boost;

namespace Sync {

DiffState::DiffState ()
{
}

DiffState::~DiffState ()
{
}

DiffStatePtr
DiffState::diff () const
{
  DiffStatePtr ret = make_shared<DiffState> ();
  
  DiffStatePtr state = m_next;
  while (state != 0)
    {
      *ret += *state;
      state = state->m_next;
    }

  return ret;
}

DiffState &
DiffState::operator += (const DiffState &state)
{
  BOOST_FOREACH (LeafConstPtr _leaf, state.getLeaves ())
    {
      DiffLeafConstPtr leaf = dynamic_pointer_cast<const DiffLeaf> (_leaf);
      BOOST_ASSERT (leaf != 0);

      if (leaf->getOperation () == UPDATE)
        update (leaf->getInfo (), leaf->getSeq ());
      else if (leaf->getOperation () == REMOVE)
        remove (leaf->getInfo ());
      else
        {
          BOOST_ASSERT (false);
        }
    }
  
  return *this;
}
  
// from State
boost::tuple<bool/*inserted*/, bool/*updated*/, SeqNo/*oldSeqNo*/>
DiffState::update (NameInfoConstPtr info, const SeqNo &seq)
{
  m_leaves.erase (info);

  DiffLeafPtr leaf = make_shared<DiffLeaf> (info, cref (seq));
  m_leaves.insert (leaf);

  return make_tuple (true, false, SeqNo ());
}

bool
DiffState::remove (NameInfoConstPtr info)
{
  m_leaves.erase (info);

  DiffLeafPtr leaf = make_shared<DiffLeaf> (info);
  m_leaves.insert (leaf);

  return true;
}

} // ns3
