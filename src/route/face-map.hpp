/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014  University of Memphis,
 *                     Regents of the University of California
 *
 * This file is part of NLSR (Named-data Link State Routing).
 * See AUTHORS.md for complete list of NLSR authors and contributors.
 *
 * NLSR is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 *
 **/
#ifndef NLSR_FACE_MAP_HPP
#define NLSR_FACE_MAP_HPP

#include <ndn-cxx/common.hpp>
#include <algorithm>

namespace nlsr {

class FaceMapEntry {

public:
  FaceMapEntry(const std::string& faceUri, uint32_t faceId)
    : m_faceUri(faceUri)
    , m_faceId(faceId)
  {
  }

  void
  setFaceUri(const std::string& faceUri)
  {
    m_faceUri = faceUri;
  }

  const std::string&
  getFaceUri() const
  {
    return m_faceUri;
  }

  void
  setFaceId(uint32_t faceId)
  {
    m_faceId = faceId;
  }

  uint32_t
  getFaceId() const
  {
    return m_faceId;
  }

  bool
  compare(const FaceMapEntry& fme)
  {
    return m_faceUri == fme.getFaceUri();
  }

private:
  std::string m_faceUri;
  uint32_t m_faceId;
};

class FaceMap {

public:
  FaceMap()
  {
  }

  ~FaceMap()
  {
  }

  inline void
  update(const std::string& faceUri, uint32_t faceId)
  {
    FaceMapEntry fme(faceUri, faceId);
    std::list<FaceMapEntry>::iterator it = std::find_if(m_table.begin(),
                                                        m_table.end(),
                                                        bind(&FaceMapEntry::compare,
                                                             &fme, _1));
    if (it == m_table.end()) {
      m_table.push_back(fme);
    }
    else {
      (*it).setFaceId(fme.getFaceId());
    }
  }

  inline uint32_t
  getFaceId(const std::string& faceUri)
  {
    FaceMapEntry fme(faceUri, 0);
    std::list<FaceMapEntry>::iterator it = std::find_if(m_table.begin(),
                                                        m_table.end(),
                                                        bind(&FaceMapEntry::compare,
                                                             &fme, _1));
    if (it != m_table.end()) {
      return (*it).getFaceId();
    }
    return 0;
  }

  void
  writeLog();

private:
  std::list<FaceMapEntry> m_table;
};

} //namespace nlsr

#endif //NLSR_FACE_MAP_HPP
