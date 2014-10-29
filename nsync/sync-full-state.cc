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

#include "sync-full-state.h"

#include <boost/assert.hpp>

#include "sync-full-leaf.h"

namespace Sync {


FullState::FullState ()
// m_lastUpdated is initialized to "not_a_date_time" in normal lib mode and to "0" time in NS-3 mode
{
}

FullState::~FullState ()
{
}

ndn::time::system_clock::Duration
FullState::getTimeFromLastUpdate () const
{
  return ndn::time::system_clock::now() - m_lastUpdated;
}

DigestConstPtr
FullState::getDigest ()
{
  if (!m_digest)
    {
      m_digest = make_shared<Digest> ();
      if (m_leaves.get<ordered> ().size () > 0)
        {
          for (LeafConstPtr leaf : m_leaves.get<ordered> ())
            {
              FullLeafConstPtr fullLeaf = dynamic_pointer_cast<const FullLeaf> (leaf);
              BOOST_ASSERT (fullLeaf != 0);
              *m_digest << fullLeaf->getDigest ();
            }
          m_digest->finalize ();
        }
      else
        {
          std::istringstream is ("00"); //zero state
          is >> *m_digest;
        }
    }

  return m_digest;
}

// from State
tuple<bool/*inserted*/, bool/*updated*/, SeqNo/*oldSeqNo*/>
FullState::update (NameInfoConstPtr info, const SeqNo &seq)
{
  m_lastUpdated = ndn::time::system_clock::now();


  m_digest.reset ();

  LeafContainer::iterator item = m_leaves.find (info);
  if (item == m_leaves.end ())
    {
      m_leaves.insert (make_shared<FullLeaf> (info, seq));
      return make_tuple (true, false, SeqNo ());
    }
  else
    {
      if ((*item)->getSeq () == seq || seq < (*item)->getSeq ())
        {
          return make_tuple (false, false, SeqNo ());
        }

      SeqNo old = (*item)->getSeq ();
      m_leaves.modify (item, [&seq](LeafPtr data){ data->setSeq(seq); });
      return make_tuple (false, true, old);
    }
}

bool
FullState::remove (NameInfoConstPtr info)
{
  m_lastUpdated = ndn::time::system_clock::now();

  m_digest.reset ();

  LeafContainer::iterator item = m_leaves.find (info);
  if (item != m_leaves.end ())
    {
      m_leaves.erase (item);
      return true;
    }
  else
    return false;
}

} // Sync
