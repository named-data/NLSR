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
 *         Yingdi Yu <yingdi@cs.ucla.edu>
 */

#ifndef SYNC_SEQ_NO_H
#define SYNC_SEQ_NO_H

#include <boost/cstdint.hpp>
#include "sync-digest.h"

namespace Sync {

/**
 * @ingroup sync
 * @brief Sequence number abstraction
 */
class SeqNo
{
public:
  /**
   * @brief Default constructor.  Creates an zero sequence number with zero session ID (basically is an invalid object)
   */
  SeqNo ()
    : m_valid (false)
    , m_session (0)
    , m_seq (0)
  {
  }

  /**
   * @brief Copy constructor
   * @param seq sequence number object to copy from
   */
  SeqNo (const SeqNo &seq)
  {
    *this = seq;
  }

  /**
   * @brief Assignment operator
   * @param seq sequence number object to copy from
   */
  SeqNo &
  operator = (const SeqNo &seq)
  {
    m_valid = seq.m_valid;
    m_session = seq.m_session;
    m_seq = seq.m_seq;

    return *this;
  }

  /**
   * @brief Constructor with just sequence number. Session assumed to be zero
   * @param seq Sequence number
   */
  SeqNo (uint64_t seq)
    : m_valid (true)
    , m_session (0)
    , m_seq (seq)
  { }

  /**
   * @brief Constructor with session and sequence id
   * @param session Session ID
   * @param seq Sequence number
   */
  SeqNo (uint64_t session, uint64_t seq)
    : m_valid (true)
    , m_session (session)
    , m_seq (seq)
  { }

  /**
   * @brief Get sequence number digest
   *
   * Digest will be calculated every time it is requested
   */
  DigestConstPtr
  getDigest () const;

  /**
   * @brief Compare if one sequence number is lower
   * @param seq Another sequence number to compare with
   *
   * tuple (session1, seq1) is less than (session2, seq2) in two cases:
   * 1. session1 < session2
   * 2. session1 == session2 and seq1 < seq2
   */
  bool
  operator < (const SeqNo &seq) const
  {
    return m_session < seq.m_session || (m_session == seq.m_session && m_seq < seq.m_seq);
  }

  /**
   * @brief Compare if two sequence numbers are equal
   * @param seq Another sequence number to compare with
   */
  bool
  operator == (const SeqNo &seq) const
  {
    return m_session == seq.m_session && m_seq == seq.m_seq;
  }

  bool
  operator <= (const SeqNo &seq) const
  {
    return m_session == seq.m_session && m_seq <= seq.m_seq;
  }

  SeqNo &
  operator ++ ()
  {
    if (m_valid) {
      m_seq ++;
    }
    else {
      m_valid = true;
    }
    return *this;
  }

  bool
  isValid () const
  {
    return m_valid;
  }

  /**
   * @brief Get session id
   */
  uint64_t getSession () const
  { return m_session; }

  /**
   * @brief Get sequence number
   */
  uint64_t getSeq () const
  { return m_seq; }

  /**
   * @brief Set sequence number
   */
   void
   setSeq(uint64_t seq)
   { m_seq = seq; }

private:
  bool m_valid;

  /**
   * @brief Session ID (e.g., after crash, application will choose new session ID.
   *
   * Note that session IDs for the same name should always increase. So, the good choice
   * for the session ID is client's timestamp
   */
  uint64_t m_session;

  /**
   * @brief Sequence number
   *
   * Sequence number for a session always starts with 0 and goes to max value.
   *
   * For now, wrapping sequence number after max to zero is not supported
   */
  uint64_t m_seq;
};

inline std::ostream &
operator << (std::ostream &os, const SeqNo &seqno)
{
  os << "<session>" << seqno.getSession () << "</session><seqno>" << seqno.getSeq () << "</seqno>";
  return os;
}

} // Sync

#endif // SYNC_SEQ_NO_H
