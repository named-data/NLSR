/**
 * Copyright (C) 2014 Regents of the University of Memphis.
 * See COPYING for copyright and distribution information.
 */

#include "utility/tokenizer.hpp"
#include <boost/test/unit_test.hpp>

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestTokenizer)

BOOST_AUTO_TEST_CASE(TokenizerBasic)
{
  Tokenizer token1("/ndn/memphis.edu/cs", "/");
  Tokenizer token2("/ndn/memphis.edu/cs", "/");

  Tokenizer token3("/test/hello");

  BOOST_CHECK(token1.getFirstToken() == token2.getFirstToken());

  BOOST_CHECK_EQUAL(token1.getRestOfLine(), token2.getRestOfLine());

  BOOST_CHECK(token1.getFirstToken() != token3.getFirstToken());

  BOOST_CHECK(token1.getRestOfLine() != token3.getRestOfLine());
}

BOOST_AUTO_TEST_SUITE_END()

} //namespace test
} //namespace nlsr
