/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
 *                           Regents of the University of California
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
 **/

#ifndef NLSR_FACE_MAP_HPP
#define NLSR_FACE_MAP_HPP

#include "common.hpp"

#include <map>

namespace nlsr {

class FaceMapEntry
{

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

class FaceMap
{

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
    std::map<std::string, FaceMapEntry>::iterator it = m_table.find(faceUri);
    if (it == m_table.end()) {
      m_table.emplace(faceUri, fme);
    }
    else {
      (it->second).setFaceId(fme.getFaceId());
    }
  }

  inline uint32_t
  getFaceId(const std::string& faceUri)
  {
    FaceMapEntry fme(faceUri, 0);
    std::map<std::string, FaceMapEntry>::iterator it = m_table.find(faceUri);
    if (it != m_table.end()) {
      return (it->second).getFaceId();
    }
    return 0;
  }

  void
  writeLog();

private:
  std::map<std::string, FaceMapEntry> m_table;
};

} // namespace nlsr

#endif // NLSR_FACE_MAP_HPP
