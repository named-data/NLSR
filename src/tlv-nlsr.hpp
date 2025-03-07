/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2025,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
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
 */

#ifndef NLSR_TLV_NLSR_HPP
#define NLSR_TLV_NLSR_HPP

namespace nlsr::tlv {

/*! The TLV block types that NLSR uses to encode/decode LSA types. The
 *  way NLSR encodes LSAs to TLV is by encoding each element of the
 *  LSA as a separate TLV block. So, block types are needed. These are
 *  used in the LSDB Status Dataset.
 */
enum {
  Lsa                         = 128,
  SequenceNumber              = 130,
  AdjacencyLsa                = 131,
  Adjacency                   = 132,
  CoordinateLsa               = 133,
  CostDouble                  = 134,
  HyperbolicRadius            = 135,
  HyperbolicAngle             = 136,
  NameLsa                     = 137,
  LsdbStatus                  = 138,
  ExpirationTime              = 139,
  Cost                        = 140,
  Uri                         = 141,
  NextHop                     = 143,
  RoutingTable                = 144,
  RoutingTableEntry           = 145,
  PrefixInfo                  = 146
};

} // namespace nlsr::tlv

#endif // NLSR_TLV_NLSR_HPP
