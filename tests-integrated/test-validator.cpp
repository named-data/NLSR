/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
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
 **/

#include "validator.hpp"

#include <ndn-cxx/security/certificate-cache-ttl.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/util/scheduler.hpp>

#include "boost-test.hpp"
#include "common.hpp"
#include "security/certificate-store.hpp"

namespace nlsr {

namespace test {

BOOST_AUTO_TEST_SUITE(TestValidator)

struct ValidatorFixture
{
  ValidatorFixture()
    : m_face2(m_face.getIoService())
    , m_scheduler(m_face.getIoService())
    , m_keyPrefix("/ndn/broadcast/KEYS")
    , m_certificateCache(new ndn::CertificateCacheTtl(m_face.getIoService()))
    , m_validator(m_face2, ndn::Name("/ndn/broadcast"), m_certificateCache, m_certStore)
    , m_identity("/TestValidator/NLSR")
    , m_wasValidated(false)
  {
    m_face.setInterestFilter(m_keyPrefix,
                             ndn::bind(&ValidatorFixture::onKeyInterest, this, _1, _2),
                             ndn::bind(&ValidatorFixture::onKeyPrefixRegSuccess, this, _1),
                             ndn::bind(&ValidatorFixture::registrationFailed, this, _1, _2));

    m_keyChain.createIdentity(m_identity);
    ndn::Name certName = m_keyChain.getDefaultCertificateNameForIdentity(m_identity);
    m_cert = m_keyChain.getCertificate(certName);
    ndn::io::save(*m_cert, "trust-anchor.cert");

    const std::string CONFIG =
      "rule\n"
      "{\n"
      "  id \"NSLR Hello Rule\"\n"
      "  for data\n"
      "  filter\n"
      "  {\n"
      "    type name\n"
      "    regex ^[^<NLSR><INFO>]*<NLSR><INFO><><>$\n"
      "  }\n"
      "  checker\n"
      "  {\n"
      "    type customized\n"
      "    sig-type rsa-sha256\n"
      "    key-locator\n"
      "    {\n"
      "      type name\n"
      "      hyper-relation\n"
      "      {\n"
      "        k-regex ^([^<KEY><NLSR>]*)<NLSR><KEY><ksk-.*><ID-CERT>$\n"
      "        k-expand \\\\1\n"
      "        h-relation equal\n"
      "        p-regex ^([^<NLSR><INFO>]*)<NLSR><INFO><><>$\n"
      "        p-expand \\\\1\n"
      "      }\n"
      "    }\n"
      "  }\n"
      "}\n"
      "rule\n"
      "{\n"
      "  id \"Single Rule\"\n"
      "  for data\n"
      "  filter\n"
      "  {\n"
      "    type name\n"
      "    regex ^<TestValidator>([^<KEY><NLSR>]*)<NLSR><KEY><ksk-.*><><>$\n"
      "  }\n"
      "  checker\n"
      "  {\n"
      "    type fixed-signer\n"
      "    sig-type rsa-sha256\n"
      "    signer\n"
      "    {\n"
      "      type file\n"
      "      file-name \"trust-anchor.cert\"\n"
      "    }\n"
      "  }\n"
      "}\n";

    const boost::filesystem::path CONFIG_PATH =
      (boost::filesystem::current_path() / std::string("unit-test.conf"));

    m_validator.load(CONFIG, CONFIG_PATH.native());
  }

  ~ValidatorFixture()
  {
    m_keyChain.deleteIdentity(m_identity);

    const boost::filesystem::path CERT_PATH =
      (boost::filesystem::current_path() / std::string("trust-anchor.cert"));
    boost::filesystem::remove(CERT_PATH);
  }

  void
  onKeyInterest(const ndn::Name& name, const ndn::Interest& interest)
  {
    const ndn::Name& interestName = interest.getName();

    ndn::Name certName = interestName.getSubName(name.size());

    if (certName[-2].toUri() == "ID-CERT")
      {
        certName = certName.getPrefix(-1);
      }
    else if (certName[-1].toUri() != "ID-CERT")
      return; //Wrong key interest.

    if (certName != m_cert->getName().getPrefix(-1))
      return; //No such a cert

    shared_ptr<ndn::Data> data = make_shared<ndn::Data>(interestName);
    data->setContent(m_cert->wireEncode());
    m_keyChain.signWithSha256(*data);

    m_face.put(*data);
  }

  void
  onKeyPrefixRegSuccess(const ndn::Name& name)
  {
    BOOST_REQUIRE(true);
  }

  void
  registrationFailed(const ndn::Name& name, const std::string& msg)
  {
    std::cerr << "Failure Info: " << msg << std::endl;
    BOOST_REQUIRE(false);
  }

  void
  onValidated(const ndn::shared_ptr<const ndn::Data>& data)
  {
    m_wasValidated = true;
  }

  void
  onValidationFailed(const ndn::shared_ptr<const ndn::Data>& data,
                     const std::string& failureInfo)
  {
    std::cerr << "Failure Info: " << failureInfo << std::endl;
    m_wasValidated = false;
  }

  void
  validate(const ndn::shared_ptr<const ndn::Data>& data)
  {
    m_validator.validate(*data,
                         bind(&ValidatorFixture::onValidated, this, _1),
                         bind(&ValidatorFixture::onValidationFailed, this, _1, _2));
  }

  void
  terminate()
  {
    m_face.getIoService().stop();
  }

protected:
  ndn::Face m_face;
  ndn::Face m_face2;
  ndn::Scheduler m_scheduler;
  const ndn::Name m_keyPrefix;
  ndn::shared_ptr<ndn::CertificateCacheTtl> m_certificateCache;
  security::CertificateStore m_certStore;
  nlsr::Validator m_validator;

  ndn::KeyChain m_keyChain;
  ndn::Name m_identity;
  ndn::shared_ptr<ndn::IdentityCertificate> m_cert;

  bool m_wasValidated;
};

BOOST_FIXTURE_TEST_CASE(InfoCertFetch, ValidatorFixture)
{
  ndn::Name dataName = m_identity;
  dataName.append("INFO").append("neighbor").append("version");
  ndn::shared_ptr<ndn::Data> data = ndn::make_shared<ndn::Data>(dataName);
  m_keyChain.signByIdentity(*data, m_identity);

  m_scheduler.scheduleEvent(ndn::time::milliseconds(200),
                            ndn::bind(&ValidatorFixture::validate, this, data));
  m_scheduler.scheduleEvent(ndn::time::milliseconds(1000),
                            ndn::bind(&ValidatorFixture::terminate, this));
  BOOST_REQUIRE_NO_THROW(m_face.processEvents());

  BOOST_CHECK(m_wasValidated);
}

BOOST_FIXTURE_TEST_CASE(CertificateStorage, ValidatorFixture)
{
  std::vector<ndn::CertificateSubjectDescription> subjectDescription;

  // Create an operator identity
  ndn::Name opIdentity("/TestValidator/operator/NLSR");
  m_keyChain.createIdentity(opIdentity);

  // Create an operator cert signed by the trust anchor
  ndn::Name keyName = m_keyChain.generateRsaKeyPairAsDefault(opIdentity, true);
  shared_ptr<ndn::IdentityCertificate> opCert =
    m_keyChain.prepareUnsignedIdentityCertificate(keyName,
                                                  m_identity,
                                                  ndn::time::system_clock::now(),
                                                  ndn::time::system_clock::now()
                                                    + ndn::time::days(1),
                                                  subjectDescription);
  m_keyChain.signByIdentity(*opCert, m_identity);
  m_keyChain.addCertificateAsIdentityDefault(*opCert);

  // Sign data with operator cert
  ndn::Name dataName = opIdentity;
  dataName.append("INFO").append("neighbor").append("version");
  ndn::shared_ptr<ndn::Data> data = ndn::make_shared<ndn::Data>(dataName);
  m_keyChain.signByIdentity(*data, opIdentity);

  // Check without cert in CertificateStore
  m_scheduler.scheduleEvent(ndn::time::milliseconds(200),
                            ndn::bind(&ValidatorFixture::validate, this, data));
  m_scheduler.scheduleEvent(ndn::time::milliseconds(1000),
                            ndn::bind(&ValidatorFixture::terminate, this));

  BOOST_REQUIRE_NO_THROW(m_face.processEvents());
  BOOST_CHECK_EQUAL(m_wasValidated, false);

  // Check with cert in CertificateStore
  m_certStore.insert(opCert);

  m_scheduler.scheduleEvent(ndn::time::milliseconds(200),
                            ndn::bind(&ValidatorFixture::validate, this, data));
  m_scheduler.scheduleEvent(ndn::time::milliseconds(1000),
                            ndn::bind(&ValidatorFixture::terminate, this));

  BOOST_REQUIRE_NO_THROW(m_face.processEvents());
  BOOST_CHECK(m_wasValidated);

  // Cleanup
  m_keyChain.deleteIdentity(opIdentity);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace nlsr
