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
 * Author: Yingdi Yu <yingdi@cs.ucla.edu>
 */

#ifndef _SYNC_SOCKET_H
#define _SYNC_SOCKET_H

#include "sync-common.h"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator.hpp>
#include <ndn-cxx/security/key-chain.hpp>

#include "sync-logic.h"
#include "sync-seq-no.h"

#include <utility>
#include <map>
#include <vector>
#include <sstream>

namespace Sync {

/**
 * \ingroup sync
 * @brief A simple interface to interact with client code
 */
class SyncSocket
{
public:
  typedef ndn::function<void(const std::vector<MissingDataInfo> &, SyncSocket *)> NewDataCallback;
  typedef ndn::function<void(const std::string &/*prefix*/)> RemoveCallback;

  /**
   * @brief the constructor for SyncAppSocket; the parameter syncPrefix
   * should be passed to the constructor of m_syncAppWrapper; the other
   * parameter should be passed to the constructor of m_fetcher; furthermore,
   * the fetch function of m_fetcher should be a second paramter passed to
   * the constructor of m_syncAppWrapper, so that m_syncAppWrapper can tell
   * m_fetcher to fetch the actual app data after it learns the names
   *
   * @param syncPrefix the name prefix for Sync Interest
   * @param dataCallback the callback to process data
   */
  SyncSocket (const ndn::Name &syncPrefix,
              ndn::shared_ptr<ndn::Validator> validator,
              ndn::shared_ptr<ndn::Face> face,
              NewDataCallback dataCallback,
              RemoveCallback rmCallback);

  ~SyncSocket ();

  bool
  publishData(const ndn::Name &prefix, uint64_t session, const char *buf, size_t len,
              int freshness,uint64_t seq);

  void
  remove (const ndn::Name &prefix)
  { m_syncLogic.remove(prefix); }

  void
  fetchData(const ndn::Name &prefix, const SeqNo &seq,
            const ndn::OnDataValidated& onValidated, int retry = 0);

  std::string
  getRootDigest()
  { return m_syncLogic.getRootDigest(); }

  uint64_t
  getNextSeq (const ndn::Name &prefix, uint64_t session);

  SyncLogic &
  getLogic ()
  { return m_syncLogic; }

  // make this a static function so we don't have to create socket instance without
  // knowing the local prefix. it's a wrong place for this function anyway
  static std::string
  GetLocalPrefix ();

private:
  void
  publishDataInternal(ndn::shared_ptr<ndn::Data> data,
                      const ndn::Name &prefix, uint64_t session,uint64_t seq);

  void
  passCallback(const std::vector<MissingDataInfo> &v)
  {
    m_newDataCallback(v, this);
  }

  void
  onData(const ndn::Interest& interest, ndn::Data& data,
         const ndn::OnDataValidated& onValidated,
         const ndn::OnDataValidationFailed& onValidationFailed);

  void
  onDataTimeout(const ndn::Interest& interest,
                int retry,
                const ndn::OnDataValidated& onValidated,
                const ndn::OnDataValidationFailed& onValidationFailed);

  void
  onDataValidationFailed(const ndn::shared_ptr<const ndn::Data>& data);

private:
  typedef std::map<ndn::Name, SeqNo> SequenceLog;
  NewDataCallback m_newDataCallback;
  SequenceLog m_sequenceLog;
  ndn::shared_ptr<ndn::Validator> m_validator;
  ndn::shared_ptr<ndn::KeyChain> m_keyChain;
  ndn::shared_ptr<ndn::Face> m_face;
  SyncLogic      m_syncLogic;
};

} // Sync

#endif // SYNC_SOCKET_H
