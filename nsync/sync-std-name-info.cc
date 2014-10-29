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

#include "sync-std-name-info.h"

namespace Sync {


NameInfoConstPtr
StdNameInfo::FindOrCreate (const std::string &key)
{
  // std::cout << "FindOrCreate: " << m_names.size () << "\n";

  NameInfoConstPtr ret;

  NameMap::iterator item = m_names.find (key);
  if (item != m_names.end ())
    {
      ret = item->second.lock ();
      BOOST_ASSERT (ret != 0);
    }
  else
    {
      ret = NameInfoPtr (new StdNameInfo (key));
      weak_ptr<const NameInfo> value (ret);
      std::pair<NameMap::iterator,bool> inserted =
        m_names.insert (make_pair (key, value));

      BOOST_ASSERT (inserted.second); // previous call has to insert value
      item = inserted.first;
    }

  return ret;
}

StdNameInfo::StdNameInfo (const std::string &name)
  : m_name (name)
{
  m_id = m_ids ++; // set ID for a newly inserted element
  m_digest << name;
  m_digest.finalize ();

  // std::cout << "StdNameInfo: " << name << " = " << m_id << "\n";
}

StdNameInfo::~StdNameInfo ()
{
  // cout << "Destructor for " << m_name << "\n";
  m_names.erase (toString ());
}

std::string
StdNameInfo::toString () const
{
  return m_name;
}

bool
StdNameInfo::operator == (const NameInfo &info) const
{
  return m_name == dynamic_cast<const StdNameInfo&> (info).m_name;
}

bool
StdNameInfo::operator < (const NameInfo &info) const
{
  return m_name < dynamic_cast<const StdNameInfo&> (info).m_name;
}

} // Sync
