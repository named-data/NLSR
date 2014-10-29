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

#include "sync-state.h"
#include "sync-diff-leaf.h"
#include "sync-std-name-info.h"

#include <boost/assert.hpp>
#include <boost/foreach.hpp>
#include <boost/throw_exception.hpp>
#include <boost/lexical_cast.hpp>

typedef boost::error_info<struct tag_errmsg, std::string> info_str;

using namespace Sync::Error;
using boost::lexical_cast;

namespace Sync {

/*
std::ostream &
operator << (std::ostream &os, const State &state)
{
  os << "<state>"; DEBUG_ENDL;

  BOOST_FOREACH (shared_ptr<const Leaf> leaf, state.getLeaves ().get<ordered> ())
    {
      shared_ptr<const DiffLeaf> diffLeaf = dynamic_pointer_cast<const DiffLeaf> (leaf);
      if (diffLeaf != 0)
        {
          os << "<item action=\"" << diffLeaf->getOperation () << "\">"; DEBUG_ENDL;
        }
      else
        {
          os << "<item>"; DEBUG_ENDL;
        }
      os << "<name>" << *leaf->getInfo () << "</name>"; DEBUG_ENDL;
      if (diffLeaf == 0 || (diffLeaf != 0 && diffLeaf->getOperation () == UPDATE))
        {
          os << "<seq>" << leaf->getSeq () << "</seq>"; DEBUG_ENDL;
        }
      os << "</item>"; DEBUG_ENDL;
    }
  os << "</state>";
}
*/

SyncStateMsg &
operator << (SyncStateMsg &ossm, const State &state)
{
  BOOST_FOREACH (shared_ptr<const Leaf> leaf, state.getLeaves ().get<ordered> ())
  {
    SyncState *oss = ossm.add_ss();
    shared_ptr<const DiffLeaf> diffLeaf = dynamic_pointer_cast<const DiffLeaf> (leaf);
    if (diffLeaf != 0 && diffLeaf->getOperation() != UPDATE)
    {
      oss->set_type(SyncState::DELETE);
    }
    else
    {
      oss->set_type(SyncState::UPDATE);
    }

    std::ostringstream os;
    os << *leaf->getInfo();
    oss->set_name(os.str());

    if (diffLeaf == 0 || (diffLeaf != 0 && diffLeaf->getOperation () == UPDATE))
    {
      SyncState::SeqNo *seqNo = oss->mutable_seqno();
      seqNo->set_session(leaf->getSeq().getSession());
      seqNo->set_seq(leaf->getSeq().getSeq());
    }
  }
  return ossm;
}

/*
std::istream &
operator >> (std::istream &in, State &state)
{
  TiXmlDocument doc;
  in >> doc;

  if (doc.RootElement() == 0)
        BOOST_THROW_EXCEPTION (SyncXmlDecodingFailure () << info_str ("Empty XML"));

  for (TiXmlElement *iterator = doc.RootElement()->FirstChildElement ("item");
       iterator != 0;
       iterator = iterator->NextSiblingElement("item"))
    {
      TiXmlElement *name = iterator->FirstChildElement ("name");
      if (name == 0 || name->GetText() == 0)
        BOOST_THROW_EXCEPTION (SyncXmlDecodingFailure () << info_str ("<name> element is missing"));

      NameInfoConstPtr info = StdNameInfo::FindOrCreate (name->GetText());

      if (iterator->Attribute("action") == 0 || strcmp(iterator->Attribute("action"), "update") == 0)
        {
          TiXmlElement *seq = iterator->FirstChildElement ("seq");
          if (seq == 0)
            BOOST_THROW_EXCEPTION (SyncXmlDecodingFailure () << info_str ("<seq> element is missing"));

          TiXmlElement *session = seq->FirstChildElement ("session");
          TiXmlElement *seqno = seq->FirstChildElement ("seqno");

          if (session == 0 || session->GetText() == 0)
            BOOST_THROW_EXCEPTION (SyncXmlDecodingFailure () << info_str ("<session> element is missing"));
          if (seqno == 0 || seqno->GetText() == 0)
            BOOST_THROW_EXCEPTION (SyncXmlDecodingFailure () << info_str ("<seqno> element is missing"));

          state.update (info, SeqNo (
                                     lexical_cast<uint32_t> (session->GetText()),
                                     lexical_cast<uint32_t> (seqno->GetText())
                                     ));
        }
      else
        {
          state.remove (info);
        }
    }

  return in;
}
*/

SyncStateMsg &
operator >> (SyncStateMsg &issm, State &state)
{
  int n = issm.ss_size();
  for (int i = 0; i < n; i++)
  {
    const SyncState &ss = issm.ss(i);
    NameInfoConstPtr info = StdNameInfo::FindOrCreate (ss.name());
    if (ss.type() == SyncState::UPDATE)
    {
      uint64_t session = lexical_cast<uint64_t>(ss.seqno().session());
      uint64_t seq = lexical_cast<uint64_t>(ss.seqno().seq());
      SeqNo seqNo(session, seq);
      state.update(info, seqNo);
    }
    else
    {
      state.remove(info);
    }
  }
  return issm;
}

}
