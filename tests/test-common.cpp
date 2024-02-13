/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2024,  The University of Memphis,
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
 */

#include "tests/test-common.hpp"

#include <ndn-cxx/mgmt/nfd/control-parameters.hpp>

namespace nlsr::tests {

std::shared_ptr<ndn::Data>
makeData(const ndn::Name& name)
{
  auto data = std::make_shared<ndn::Data>(name);
  return signData(data);
}

ndn::Data&
signData(ndn::Data& data)
{
  data.setSignatureInfo(ndn::SignatureInfo(ndn::tlv::NullSignature));
  data.setSignatureValue(std::make_shared<ndn::Buffer>());
  data.wireEncode();
  return data;
}

void
checkPrefixRegistered(const ndn::DummyClientFace& face, const ndn::Name& prefix)
{
  bool registerCommandEmitted = false;
  for (const auto& interest : face.sentInterests) {
    const auto& name = interest.getName();
    if (name.size() > 4 && name[3] == ndn::name::Component("register")) {
      ndn::nfd::ControlParameters params(name[4].blockFromValue());
      if (params.getName() == prefix) {
        registerCommandEmitted = true;
        break;
      }
    }
  }
  BOOST_CHECK(registerCommandEmitted);
}

} // namespace nlsr::tests
