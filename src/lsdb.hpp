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

#ifndef NLSR_LSDB_HPP
#define NLSR_LSDB_HPP

#include "communication/sync-logic-handler.hpp"
#include "conf-parameter.hpp"
#include "lsa/lsa.hpp"
#include "lsa/name-lsa.hpp"
#include "lsa/coordinate-lsa.hpp"
#include "lsa/adj-lsa.hpp"
#include "sequencing-manager.hpp"
#include "statistics.hpp"
#include "test-access-control.hpp"

#include <ndn-cxx/ims/in-memory-storage-fifo.hpp>
#include <ndn-cxx/ims/in-memory-storage-persistent.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/segmenter.hpp>
#include <ndn-cxx/util/segment-fetcher.hpp>
#include <ndn-cxx/util/signal.hpp>
#include <ndn-cxx/util/time.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>

namespace nlsr {

namespace bmi = boost::multi_index;

inline constexpr ndn::time::seconds GRACE_PERIOD = 10_s;

enum class LsdbUpdate {
  INSTALLED,
  UPDATED,
  REMOVED
};

class Lsdb
{
public:
  Lsdb(ndn::Face& face, ndn::KeyChain& keyChain, ConfParameter& confParam);

  ~Lsdb();

  /*! \brief Returns whether the LSDB contains some LSA.
   */
  bool
  doesLsaExist(const ndn::Name& router, Lsa::Type lsaType)
  {
    return m_lsdb.get<byName>().find(std::make_tuple(router, lsaType)) != m_lsdb.end();
  }

  /*! \brief Builds a name LSA for this router and then installs it
      into the LSDB.
  */
  void
  buildAndInstallOwnNameLsa();

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  /*! \brief Builds a cor. LSA for this router and installs it into the LSDB. */
  void
  buildAndInstallOwnCoordinateLsa();

public:
  /*! \brief Schedules a build of this router's LSA. */
  void
  scheduleAdjLsaBuild();

  void
  writeLog() const;

  /* \brief Process interest which can be either:
   * 1) Discovery interest from segment fetcher:
   *    /localhop/<network>/nlsr/LSA/<site>/<router>/<lsaType>/<seqNo>
   * 2) Interest containing segment number:
   *    /localhop/<network>/nlsr/LSA/<site>/<router>/<lsaType>/<seqNo>/<version>/<segmentNo>
  */
  void
  processInterest(const ndn::Name& name, const ndn::Interest& interest);

  bool
  getIsBuildAdjLsaScheduled() const
  {
    return m_isBuildAdjLsaScheduled;
  }

  SyncLogicHandler&
  getSync()
  {
    return m_sync;
  }

  template<typename T>
  std::shared_ptr<T>
  findLsa(const ndn::Name& router) const
  {
    return std::static_pointer_cast<T>(findLsa(router, T::type()));
  }

  struct ExtractOriginRouter
  {
    using result_type = ndn::Name;

    ndn::Name
    operator()(const Lsa& lsa) const
    {
      return lsa.getOriginRouter();
    }
  };

  struct name_hash {
    int
    operator()(const ndn::Name& name) const {
      return std::hash<ndn::Name>{}(name);
    }
  };

  struct enum_class_hash {
    template<typename T>
    int
    operator()(T t) const {
      return static_cast<int>(t);
    }
  };

  struct byName{};
  struct byType{};

  using LsaContainer = boost::multi_index_container<
    std::shared_ptr<Lsa>,
    bmi::indexed_by<
      bmi::hashed_unique<
        bmi::tag<byName>,
        bmi::composite_key<
          Lsa,
          ExtractOriginRouter,
          bmi::const_mem_fun<Lsa, Lsa::Type, &Lsa::getType>
        >,
        bmi::composite_key_hash<name_hash, enum_class_hash>
      >,
      bmi::hashed_non_unique<
        bmi::tag<byType>,
        bmi::const_mem_fun<Lsa, Lsa::Type, &Lsa::getType>,
        enum_class_hash
      >
    >
  >;

  template<typename T>
  std::pair<LsaContainer::index<Lsdb::byType>::type::iterator,
            LsaContainer::index<Lsdb::byType>::type::iterator>
  getLsdbIterator() const
  {
    return m_lsdb.get<byType>().equal_range(T::type());
  }

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  std::shared_ptr<Lsa>
  findLsa(const ndn::Name& router, Lsa::Type lsaType) const
  {
    auto it = m_lsdb.get<byName>().find(std::make_tuple(router, lsaType));
    return it != m_lsdb.end() ? *it : nullptr;
  }

  void
  incrementDataSentStats(Lsa::Type lsaType)
  {
    if (lsaType == Lsa::Type::NAME) {
      lsaIncrementSignal(Statistics::PacketType::SENT_NAME_LSA_DATA);
    }
    else if (lsaType == Lsa::Type::ADJACENCY) {
      lsaIncrementSignal(Statistics::PacketType::SENT_ADJ_LSA_DATA);
    }
    else if (lsaType == Lsa::Type::COORDINATE) {
      lsaIncrementSignal(Statistics::PacketType::SENT_COORD_LSA_DATA);
    }
  }

  void
  incrementInterestRcvdStats(Lsa::Type lsaType)
  {
    if (lsaType == Lsa::Type::NAME) {
      lsaIncrementSignal(Statistics::PacketType::RCV_NAME_LSA_INTEREST);
    }
    else if (lsaType == Lsa::Type::ADJACENCY) {
      lsaIncrementSignal(Statistics::PacketType::RCV_ADJ_LSA_INTEREST);
    }
    else if (lsaType == Lsa::Type::COORDINATE) {
      lsaIncrementSignal(Statistics::PacketType::RCV_COORD_LSA_INTEREST);
    }
  }

  void
  incrementInterestSentStats(Lsa::Type lsaType)
  {
    if (lsaType == Lsa::Type::NAME) {
      lsaIncrementSignal(Statistics::PacketType::SENT_NAME_LSA_INTEREST);
    }
    else if (lsaType == Lsa::Type::ADJACENCY) {
      lsaIncrementSignal(Statistics::PacketType::SENT_ADJ_LSA_INTEREST);
    }
    else if (lsaType == Lsa::Type::COORDINATE) {
      lsaIncrementSignal(Statistics::PacketType::SENT_COORD_LSA_INTEREST);
    }
  }

  /*! Returns whether a seq. no. from a certain router signals a new LSA.
    \param originRouter The name of the originating router.
    \param lsaType The type of the LSA.
    \param seqNo The sequence number to check.
  */
  bool
  isLsaNew(const ndn::Name& originRouter, Lsa::Type lsaType, uint64_t seqNo) const
  {
    // Is the name in the LSDB and the supplied seq no is the highest so far
    auto lsaPtr = findLsa(originRouter, lsaType);
    return lsaPtr ? lsaPtr->getSeqNo() < seqNo : true;
  }

  void
  installLsa(std::shared_ptr<Lsa> lsa);

  /*! \brief Remove a name LSA from the LSDB.
    \param router The name of the router that published the LSA to remove.
    \param lsaType The type of the LSA.

    This function will remove a name LSA from the LSDB by finding an
    LSA whose name matches key. This removal also causes the NPT to
    remove those name prefixes if no more LSAs advertise them.
   */
  void
  removeLsa(const ndn::Name& router, Lsa::Type lsaType);

  void
  removeLsa(const LsaContainer::index<Lsdb::byName>::type::iterator& lsaIt);

  /*! \brief Attempts to construct an adj. LSA.

    This function will attempt to construct an adjacency LSA. An LSA
    can only be built when the status of all of the router's neighbors
    is known. That is, when we are not currently trying to contact any
    neighbor.
   */
  void
  buildAdjLsa();

  /*! \brief Wrapper event to build and install an adj. LSA for this router. */
  void
  buildAndInstallOwnAdjLsa();

  /*! \brief Schedules a refresh/expire event in the scheduler.
    \param lsa The LSA.
    \param expTime How many seconds to wait before triggering the event.
   */
  ndn::scheduler::EventId
  scheduleLsaExpiration(std::shared_ptr<Lsa> lsa, ndn::time::seconds expTime);

  /*! \brief Either allow to expire, or refresh a name LSA.
    \param lsa The LSA.
  */
  void
  expireOrRefreshLsa(std::shared_ptr<Lsa> lsa);

  bool
  processInterestForLsa(const ndn::Interest& interest, const ndn::Name& originRouter,
                        Lsa::Type lsaType, uint64_t seqNo);

  void
  expressInterest(const ndn::Name& interestName, uint32_t timeoutCount, uint64_t incomingFaceId,
                  ndn::time::steady_clock::time_point deadline = DEFAULT_LSA_RETRIEVAL_DEADLINE);

  /*!
     \brief Error callback when SegmentFetcher fails to return an LSA

     In all error cases, a reattempt to fetch the LSA will be made.

     Segment validation can fail either because the packet does not have a
     valid signature (fatal) or because some of the certificates in the trust chain
     could not be fetched (non-fatal).

     Currently, the library does not provide clear indication (besides a plain-text message
     in the error callback) of the reason for the failure nor the segment that failed
     to be validated, thus we will continue to try to fetch the LSA until the deadline
     is reached.
   */
  void
  onFetchLsaError(uint32_t errorCode, const std::string& msg,
                  const ndn::Name& interestName, uint32_t retransmitNo,
                  const ndn::time::steady_clock::time_point& deadline,
                  ndn::Name lsaName, uint64_t seqNo);

  /*!
     \brief Success callback when SegmentFetcher returns a valid LSA

     \param interestName The base Interest used to fetch the LSA in the format:
            /<network>/NLSR/LSA/<site>/%C1.Router/<router>/<lsa-type>/<seqNo>
   */
  void
  afterFetchLsa(const ndn::ConstBufferPtr& bufferPtr, const ndn::Name& interestName);

  void
  emitSegmentValidatedSignal(const ndn::Data& data)
  {
    afterSegmentValidatedSignal(data);
  }

  ndn::time::system_clock::time_point
  getLsaExpirationTimePoint() const
  {
    return ndn::time::system_clock::now() + ndn::time::seconds(m_confParam.getRouterDeadInterval());
  }

public:
  ndn::signal::Signal<Lsdb, Statistics::PacketType> lsaIncrementSignal;
  ndn::signal::Signal<Lsdb, ndn::Data> afterSegmentValidatedSignal;
  using AfterLsdbModified = ndn::signal::Signal<Lsdb, std::shared_ptr<Lsa>, LsdbUpdate,
                                                std::list<nlsr::PrefixInfo>, std::list<nlsr::PrefixInfo>>;
  AfterLsdbModified onLsdbModified;

PUBLIC_WITH_TESTS_ELSE_PRIVATE:
  ndn::Face& m_face;
  ndn::Scheduler m_scheduler;
  ConfParameter& m_confParam;

  SyncLogicHandler m_sync;

  LsaContainer m_lsdb;

  ndn::time::seconds m_lsaRefreshTime;
  ndn::time::seconds m_adjLsaBuildInterval;
  const ndn::Name& m_thisRouterPrefix;

  // Maps the name of an LSA to its highest known sequence number from sync;
  // Used to stop NLSR from trying to fetch outdated LSAs
  std::map<ndn::Name, uint64_t> m_highestSeqNo;

  SequencingManager m_sequencingManager;

  ndn::signal::ScopedConnection m_onNewLsaConnection;

  std::set<std::shared_ptr<ndn::SegmentFetcher>> m_fetchers;
  ndn::Segmenter m_segmenter;
  ndn::InMemoryStorageFifo m_segmentFifo;

  bool m_isBuildAdjLsaScheduled;
  int64_t m_adjBuildCount;
  ndn::scheduler::ScopedEventId m_scheduledAdjLsaBuild;

  ndn::InMemoryStoragePersistent m_lsaStorage;

  static inline const ndn::time::steady_clock::time_point DEFAULT_LSA_RETRIEVAL_DEADLINE =
    ndn::time::steady_clock::time_point::min();
};

} // namespace nlsr

#endif // NLSR_LSDB_HPP
