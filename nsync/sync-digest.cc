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

#include "sync-digest.h"
#include <string.h>

#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info_str;
typedef boost::error_info<struct tag_errmsg, int> errmsg_info_int;

// for printing, may be disabled in optimized build

// #ifdef DIGEST_BASE64
// #include <boost/archive/iterators/base64_from_binary.hpp>
// #include <boost/archive/iterators/binary_from_base64.hpp>
// #endif

#include "boost-archive.h"

using namespace boost;
using namespace boost::archive::iterators;
using namespace std;

// Other options: VP_md2, EVP_md5, EVP_sha, EVP_sha1, EVP_sha256, EVP_dss, EVP_dss1, EVP_mdc2, EVP_ripemd160
#define HASH_FUNCTION EVP_sha256
#define HASH_FUNCTION_LEN 32


// #ifndef DIGEST_BASE64

template<class CharType>
struct hex_from_4_bit
{
  typedef CharType result_type;
  CharType operator () (CharType ch) const
  {
    const char *lookup_table = "0123456789abcdef";
    // cout << "New character: " << (int) ch << " (" << (char) ch << ")" << "\n";
    BOOST_ASSERT (ch < 16);
    return lookup_table[static_cast<size_t>(ch)];
  }
};

typedef transform_iterator<hex_from_4_bit<std::vector<uint8_t>::const_iterator::value_type>,
                           transform_width<std::vector<uint8_t>::const_iterator, 4, 8, std::vector<uint8_t>::const_iterator::value_type> > string_from_binary;


template<class CharType>
struct hex_to_4_bit
{
  typedef CharType result_type;
  CharType operator () (CharType ch) const
  {
    const signed char lookup_table [] = {
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
      -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };

    // cout << "New character: " << hex << (int) ch << " (" << (char) ch << ")" << "\n";
    signed char value = -1;
    if ((unsigned)ch < 128)
      value = lookup_table [(unsigned)ch];
    if (value == -1)
      BOOST_THROW_EXCEPTION (Sync::Error::DigestCalculationError () << errmsg_info_int ((int)ch));

    return value;
  }
};

typedef transform_width<transform_iterator<hex_to_4_bit<string::const_iterator::value_type>, string::const_iterator>, 8, 4> string_to_binary;

namespace Sync {

Digest::Digest ()
{
  m_context = EVP_MD_CTX_create ();

  reset ();
}

Digest::~Digest ()
{
  EVP_MD_CTX_destroy (m_context);
}

bool
Digest::empty () const
{
  return m_buffer.empty ();
}

bool
Digest::isZero () const
{
  if (m_buffer.empty ())
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest has not been yet finalized"));

  return (m_buffer.size () == 1 && m_buffer[0] == 0);
}


void
Digest::reset ()
{
  m_buffer.clear ();

  int ok = EVP_DigestInit_ex (m_context, HASH_FUNCTION (), 0);
  if (!ok)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("EVP_DigestInit_ex returned error")
                           << errmsg_info_int (ok));
}


void
Digest::finalize ()
{
  if (!m_buffer.empty ()) return;

  m_buffer.resize (HASH_FUNCTION_LEN);

  unsigned int tmp;
  int ok = EVP_DigestFinal_ex (m_context,
			       &m_buffer[0], &tmp);
  if (!ok)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("EVP_DigestFinal_ex returned error")
                           << errmsg_info_int (ok));
}

std::size_t
Digest::getHash () const
{
  if (isZero ()) return 0;

  if (sizeof (std::size_t) > m_buffer.size ())
    {
      BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                             << errmsg_info_str ("Hash is not zero and length is less than size_t")
                             << errmsg_info_int (m_buffer.size ()));
    }

  // just getting first sizeof(std::size_t) bytes
  // not ideal, but should work pretty well
  return *(reinterpret_cast<const std::size_t*> (&m_buffer[0]));
}

bool
Digest::operator == (const Digest &digest) const
{
  if (m_buffer.empty ())
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest1 is empty"));

  if (digest.m_buffer.empty ())
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest2 is empty"));

  return m_buffer == digest.m_buffer;
}


void
Digest::update (const uint8_t *buffer, size_t size)
{
  // cout << "Update: " << (void*)buffer << " / size: " << size << "\n";

  // cannot update Digest when it has been finalized
  if (!m_buffer.empty ())
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest has been already finalized"));

  bool ok = EVP_DigestUpdate (m_context, buffer, size);
  if (!ok)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("EVP_DigestUpdate returned error")
                           << errmsg_info_int (ok));
}


Digest &
Digest::operator << (const Digest &src)
{
  if (src.m_buffer.empty ())
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest has not been yet finalized"));

  update (&src.m_buffer[0], src.m_buffer.size ());

  return *this;
}

std::ostream &
operator << (std::ostream &os, const Digest &digest)
{
  BOOST_ASSERT (!digest.m_buffer.empty ());

  ostreambuf_iterator<char> out_it (os); // ostream iterator
  // need to encode to base64
  copy (string_from_binary (digest.m_buffer.begin ()),
        string_from_binary (digest.m_buffer.end ()),
        out_it);

  return os;
}

std::istream &
operator >> (std::istream &is, Digest &digest)
{
  string str;
  is >> str; // read string first

  if (str.size () == 0)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Input is empty"));

  // uint8_t padding = (3 - str.size () % 3) % 3;
  // for (uint8_t i = 0; i < padding; i++) str.push_back ('=');

  // only empty digest object can be used for reading
  if (!digest.m_buffer.empty ())
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest has been already finalized"));

  digest.m_buffer.clear ();

  copy (string_to_binary (str.begin ()),
        string_to_binary (str.end ()),
        std::back_inserter (digest.m_buffer));

  return is;
}


} // Sync
