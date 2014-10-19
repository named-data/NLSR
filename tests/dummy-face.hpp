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
 **/

#ifndef NLSR_TEST_DUMMY_FACE_HPP
#define NLSR_TEST_DUMMY_FACE_HPP

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/transport/transport.hpp>

namespace ndn {

class DummyTransport : public Transport
{
public:
  void
  receive(const Block& block)
  {
    m_receiveCallback(block);
  }

  virtual void
  close()
  {
  }

  virtual void
  pause()
  {
  }

  virtual void
  resume()
  {
  }

  virtual void
  send(const Block& wire)
  {
    if (wire.type() == tlv::Interest) {
      m_sentInterests->push_back(Interest(wire));
    }
    else if (wire.type() == tlv::Data) {
      m_sentDatas->push_back(Data(wire));
    }
  }

  virtual void
  send(const Block& header, const Block& payload)
  {
    this->send(payload);
  }

public:
  std::vector<Interest>* m_sentInterests;
  std::vector<Data>*     m_sentDatas;
};


/** \brief a Face for unit testing
 */
class DummyFace : public Face
{
public:
  explicit
  DummyFace(shared_ptr<DummyTransport> transport)
    : Face(transport)
    , m_transport(transport)
  {
    m_transport->m_sentInterests = &m_sentInterests;
    m_transport->m_sentDatas     = &m_sentDatas;
  }

  /** \brief cause the Face to receive a packet
   */
  template<typename Packet>
  void
  receive(const Packet& packet)
  {
    m_transport->receive(packet.wireEncode());
  }

public:
  std::vector<Interest> m_sentInterests;
  std::vector<Data>     m_sentDatas;

private:
  shared_ptr<DummyTransport> m_transport;
};

inline shared_ptr<DummyFace>
makeDummyFace()
{
  return make_shared<DummyFace>(make_shared<DummyTransport>());
}

} // namespace ndn

#endif // NLSR_TEST_DUMMY_FACE_HPP
