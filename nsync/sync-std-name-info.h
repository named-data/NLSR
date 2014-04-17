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

#ifndef SYNC_STD_NAME_INFO_H
#define SYNC_STD_NAME_INFO_H

#include "sync-name-info.h"
#include <string>

namespace Sync {

class StdNameInfo : public NameInfo
{
public:
  /**
   * @brief Lookup existing or create new NameInfo object
   * @param name routable prefix
   */
  static NameInfoConstPtr
  FindOrCreate (const std::string &name);

  /**
   * @brief Destructor which will clean up m_names structure
   */
  virtual ~StdNameInfo ();
  
  // from NameInfo
  virtual bool
  operator == (const NameInfo &info) const;

  virtual bool
  operator < (const NameInfo &info) const;

  virtual std::string
  toString () const;

private:
  // implementing a singleton pattern. 
  /**
   * @brief Disabled default constructor. NameInfo object should be created through FindOrCreate static call.
   */

  /**
   * @brief Disabled default
   */
  StdNameInfo () {}
  StdNameInfo& operator = (const StdNameInfo &info) { (void)info; return *this; }
  StdNameInfo (const std::string &name);
  
  std::string m_name;
};

} // Sync

#endif // SYNC_CCNX_NAME_INFO_H
