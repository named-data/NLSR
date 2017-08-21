/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2017,  The University of Memphis,
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

#include "conf-file-processor.hpp"
#include "conf-parameter.hpp"
#include "adjacent.hpp"
#include "utility/name-helper.hpp"
#include "update/prefix-update-processor.hpp"

#include <boost/cstdint.hpp>

#include <ndn-cxx/name.hpp>
#include <ndn-cxx/net/face-uri.hpp>

#include <iostream>
#include <fstream>

namespace nlsr {

template <class T>
class ConfigurationVariable
{
public:
  typedef std::function<void(T)> ConfParameterCallback;
  typedef boost::property_tree::ptree ConfigSection;

  ConfigurationVariable(const std::string& key, const ConfParameterCallback& setter)
    : m_key(key)
    , m_setterCallback(setter)
    , m_minValue(0)
    , m_maxValue(0)
    , m_shouldCheckRange(false)
    , m_isRequired(true)
  {
  }

  bool
  parseFromConfigSection(const ConfigSection& section)
  {
    try {
      T value = section.get<T>(m_key);

      if (!isValidValue(value)) {
        return false;
      }

      m_setterCallback(value);
      return true;
    }
    catch (const std::exception& ex) {

      if (m_isRequired) {
        std::cerr << ex.what() << std::endl;
        std::cerr << "Missing required configuration variable" << std::endl;
        return false;
      }
      else {
        m_setterCallback(m_defaultValue);
        return true;
      }
    }

    return false;
  }

  void
  setMinAndMaxValue(T min, T max)
  {
    m_minValue = min;
    m_maxValue = max;
    m_shouldCheckRange = true;
  }

  void
  setOptional(T defaultValue)
  {
    m_isRequired = false;
    m_defaultValue = defaultValue;
  }

private:
  void
  printOutOfRangeError(T value)
  {
    std::cerr << "Invalid value for " << m_key << ": "
              << value << ". "
              << "Valid values: "
              << m_minValue << " - "
              << m_maxValue << std::endl;
  }

  bool
  isValidValue(T value)
  {
    if (!m_shouldCheckRange) {
      return true;
    }
    else if (value < m_minValue || value > m_maxValue)
    {
      printOutOfRangeError(value);
      return false;
    }

    return true;
  }

private:
  const std::string m_key;
  const ConfParameterCallback m_setterCallback;
  T m_defaultValue;

  T m_minValue;
  T m_maxValue;

  bool m_shouldCheckRange;
  bool m_isRequired;
};

bool
ConfFileProcessor::processConfFile()
{
  bool ret = true;
  std::ifstream inputFile;
  inputFile.open(m_confFileName.c_str());
  if (!inputFile.is_open()) {
    std::string msg = "Failed to read configuration file: ";
    msg += m_confFileName;
    std::cerr << msg << std::endl;
    return false;
  }
  ret = load(inputFile);
  inputFile.close();
  return ret;
}

bool
ConfFileProcessor::load(std::istream& input)
{
  ConfigSection pt;
  bool ret = true;
  try {
    boost::property_tree::read_info(input, pt);
  }
  catch (const boost::property_tree::info_parser_error& error) {
    std::stringstream msg;
    std::cerr << "Failed to parse configuration file " << std::endl;
    std::cerr << m_confFileName << std::endl;
    return false;
  }

  for (ConfigSection::const_iterator tn = pt.begin();
       tn != pt.end(); ++tn) {
    ret = processSection(tn->first, tn->second);
    if (ret == false) {
      break;
    }
  }
  return ret;
}

bool
ConfFileProcessor::processSection(const std::string& sectionName, const ConfigSection& section)
{
  bool ret = true;
  if (sectionName == "general")
  {
    ret = processConfSectionGeneral(section);
  }
  else if (sectionName == "neighbors")
  {
    ret = processConfSectionNeighbors(section);
  }
  else if (sectionName == "hyperbolic")
  {
    ret = processConfSectionHyperbolic(section);
  }
  else if (sectionName == "fib")
  {
    ret = processConfSectionFib(section);
  }
  else if (sectionName == "advertising")
  {
    ret = processConfSectionAdvertising(section);
  }
  else if (sectionName == "security")
  {
    ret = processConfSectionSecurity(section);
  }
  else
  {
    std::cerr << "Wrong configuration section: " << sectionName << std::endl;
  }
  return ret;
}

bool
ConfFileProcessor::processConfSectionGeneral(const ConfigSection& section)
{
  try {
    std::string network = section.get<std::string>("network");
    std::string site = section.get<std::string>("site");
    std::string router = section.get<std::string>("router");
    ndn::Name networkName(network);
    if (!networkName.empty()) {
      m_nlsr.getConfParameter().setNetwork(networkName);
    }
    else {
      std::cerr << " Network can not be null or empty or in bad URI format :(!" << std::endl;
      return false;
    }
    ndn::Name siteName(site);
    if (!siteName.empty()) {
      m_nlsr.getConfParameter().setSiteName(siteName);
    }
    else {
      std::cerr << "Site can not be null or empty or in bad URI format:( !" << std::endl;
      return false;
    }
    ndn::Name routerName(router);
    if (!routerName.empty()) {
      m_nlsr.getConfParameter().setRouterName(routerName);
    }
    else {
      std::cerr << " Router name can not be null or empty or in bad URI format:( !" << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    return false;
  }

  // lsa-refresh-time
  uint32_t lsaRefreshTime = section.get<uint32_t>("lsa-refresh-time", LSA_REFRESH_TIME_DEFAULT);

  if (lsaRefreshTime >= LSA_REFRESH_TIME_MIN && lsaRefreshTime <= LSA_REFRESH_TIME_MAX) {
    m_nlsr.getConfParameter().setLsaRefreshTime(lsaRefreshTime);
  }
  else {
    std::cerr << "Wrong value for lsa-refresh-time ";
    std::cerr << "Allowed value: " << LSA_REFRESH_TIME_MIN << "-";;
    std::cerr << LSA_REFRESH_TIME_MAX << std::endl;

    return false;
  }

  // router-dead-interval
  uint32_t routerDeadInterval = section.get<uint32_t>("router-dead-interval", (2*lsaRefreshTime));

  if (routerDeadInterval > m_nlsr.getConfParameter().getLsaRefreshTime()) {
    m_nlsr.getConfParameter().setRouterDeadInterval(routerDeadInterval);
  }
  else {
    std::cerr << "Value of router-dead-interval must be larger than lsa-refresh-time" << std::endl;
    return false;
  }

  // lsa-interest-lifetime
  int lifetime = section.get<int>("lsa-interest-lifetime", LSA_INTEREST_LIFETIME_DEFAULT);

  if (lifetime >= LSA_INTEREST_LIFETIME_MIN && lifetime <= LSA_INTEREST_LIFETIME_MAX) {
    m_nlsr.getConfParameter().setLsaInterestLifetime(ndn::time::seconds(lifetime));
  }
  else {
    std::cerr << "Wrong value for lsa-interest-timeout. "
              << "Allowed value:" << LSA_INTEREST_LIFETIME_MIN << "-"
              << LSA_INTEREST_LIFETIME_MAX << std::endl;

    return false;
  }

  // log-level
  std::string logLevel = section.get<std::string>("log-level", "INFO");

  if (isValidLogLevel(logLevel)) {
    m_nlsr.getConfParameter().setLogLevel(logLevel);
  }
  else {
    std::cerr << "Invalid value for log-level ";
    std::cerr << "Valid values: ALL, TRACE, DEBUG, INFO, WARN, ERROR, NONE" << std::endl;
    return false;
  }

  try {
    std::string logDir = section.get<std::string>("log-dir");
    if (boost::filesystem::exists(logDir)) {
      if (boost::filesystem::is_directory(logDir)) {
        std::string testFileName=logDir+"/test.log";
        std::ofstream testOutFile;
        testOutFile.open(testFileName.c_str());
        if (testOutFile.is_open() && testOutFile.good()) {
          m_nlsr.getConfParameter().setLogDir(logDir);
        }
        else {
          std::cerr << "User does not have read and write permission on the directory";
          std::cerr << std::endl;
          return false;
        }
        testOutFile.close();
        remove(testFileName.c_str());
      }
      else {
        std::cerr << "Provided path is not a directory" << std::endl;
        return false;
      }
    }
    else {
      std::cerr << "Provided log directory <" << logDir << "> does not exist" << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << "You must configure log directory" << std::endl;
    std::cerr << ex.what() << std::endl;
    return false;
  }

  try {
    std::string seqDir = section.get<std::string>("seq-dir");
    if (boost::filesystem::exists(seqDir)) {
      if (boost::filesystem::is_directory(seqDir)) {
        std::string testFileName=seqDir+"/test.seq";
        std::ofstream testOutFile;
        testOutFile.open(testFileName.c_str());
        if (testOutFile.is_open() && testOutFile.good()) {
          m_nlsr.getConfParameter().setSeqFileDir(seqDir);
        }
        else {
          std::cerr << "User does not have read and write permission on the directory";
          std::cerr << std::endl;
          return false;
        }
        testOutFile.close();
        remove(testFileName.c_str());
      }
      else {
        std::cerr << "Provided path is not a directory" << std::endl;
        return false;
      }
    }
    else {
      std::cerr << "Provided sequence directory <" << seqDir << "> does not exist" << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << "You must configure sequence directory" << std::endl;
    std::cerr << ex.what() << std::endl;
    return false;
  }

  try {
    std::string log4cxxPath = section.get<std::string>("log4cxx-conf");

    if (log4cxxPath == "") {
      std::cerr << "No value provided for log4cxx-conf" << std::endl;
      return false;
    }

    if (boost::filesystem::exists(log4cxxPath)) {
      m_nlsr.getConfParameter().setLog4CxxConfPath(log4cxxPath);
    }
    else {
      std::cerr << "Provided path for log4cxx-conf <" << log4cxxPath
                << "> does not exist" << std::endl;

      return false;
    }
  }
  catch (const std::exception& ex) {
    // Variable is optional so default configuration will be used; continue processing file
  }

  return true;
}

bool
ConfFileProcessor::processConfSectionNeighbors(const ConfigSection& section)
{
  // hello-retries
  int retrials = section.get<int>("hello-retries", HELLO_RETRIES_DEFAULT);

  if (retrials >= HELLO_RETRIES_MIN && retrials <= HELLO_RETRIES_MAX) {
    m_nlsr.getConfParameter().setInterestRetryNumber(retrials);
  }
  else {
    std::cerr << "Wrong value for hello-retries." << std::endl;
    std::cerr << "Allowed value:" << HELLO_RETRIES_MIN << "-";
    std::cerr << HELLO_RETRIES_MAX << std::endl;

    return false;
  }

  // hello-timeout
  uint32_t timeOut = section.get<uint32_t>("hello-timeout", HELLO_TIMEOUT_DEFAULT);

  if (timeOut >= HELLO_TIMEOUT_MIN && timeOut <= HELLO_TIMEOUT_MAX) {
    m_nlsr.getConfParameter().setInterestResendTime(timeOut);
  }
  else {
    std::cerr << "Wrong value for hello-timeout. ";
    std::cerr << "Allowed value:" << HELLO_TIMEOUT_MIN << "-";
    std::cerr << HELLO_TIMEOUT_MAX << std::endl;

    return false;
  }

  // hello-interval
  uint32_t interval = section.get<uint32_t>("hello-interval", HELLO_INTERVAL_DEFAULT);

  if (interval >= HELLO_INTERVAL_MIN && interval <= HELLO_INTERVAL_MAX) {
    m_nlsr.getConfParameter().setInfoInterestInterval(interval);
  }
  else {
    std::cerr << "Wrong value for hello-interval. ";
    std::cerr << "Allowed value:" << HELLO_INTERVAL_MIN << "-";
    std::cerr << HELLO_INTERVAL_MAX << std::endl;

    return false;
  }

  // Event intervals
  // adj-lsa-build-interval
  ConfigurationVariable<uint32_t> adjLsaBuildInterval("adj-lsa-build-interval",
                                                      std::bind(&ConfParameter::setAdjLsaBuildInterval,
                                                      &m_nlsr.getConfParameter(), _1));
  adjLsaBuildInterval.setMinAndMaxValue(ADJ_LSA_BUILD_INTERVAL_MIN, ADJ_LSA_BUILD_INTERVAL_MAX);
  adjLsaBuildInterval.setOptional(ADJ_LSA_BUILD_INTERVAL_DEFAULT);

  if (!adjLsaBuildInterval.parseFromConfigSection(section)) {
    return false;
  }
  // Set the retry count for fetching the FaceStatus dataset
  ConfigurationVariable<uint32_t> faceDatasetFetchTries("face-dataset-fetch-tries",
                                                        std::bind(&ConfParameter::setFaceDatasetFetchTries,
                                                                  &m_nlsr.getConfParameter(),
                                                                  _1));

  faceDatasetFetchTries.setMinAndMaxValue(FACE_DATASET_FETCH_TRIES_MIN,
                                          FACE_DATASET_FETCH_TRIES_MAX);
  faceDatasetFetchTries.setOptional(FACE_DATASET_FETCH_TRIES_DEFAULT);

  if (!faceDatasetFetchTries.parseFromConfigSection(section)) {
    return false;
  }

  // Set the interval between FaceStatus dataset fetch attempts.
  ConfigurationVariable<uint32_t> faceDatasetFetchInterval("face-dataset-fetch-interval",
                                                           bind(&ConfParameter::setFaceDatasetFetchInterval,
                                                                &m_nlsr.getConfParameter(),
                                                                _1));

  faceDatasetFetchInterval.setMinAndMaxValue(FACE_DATASET_FETCH_INTERVAL_MIN,
                                             FACE_DATASET_FETCH_INTERVAL_MAX);
  faceDatasetFetchInterval.setOptional(FACE_DATASET_FETCH_INTERVAL_DEFAULT);

  if (!faceDatasetFetchInterval.parseFromConfigSection(section)) {
    return false;
  }

  // first-hello-interval
  ConfigurationVariable<uint32_t> firstHelloInterval("first-hello-interval",
                                                     std::bind(&ConfParameter::setFirstHelloInterval,
                                                     &m_nlsr.getConfParameter(), _1));
  firstHelloInterval.setMinAndMaxValue(FIRST_HELLO_INTERVAL_MIN, FIRST_HELLO_INTERVAL_MAX);
  firstHelloInterval.setOptional(FIRST_HELLO_INTERVAL_DEFAULT);

  if (!firstHelloInterval.parseFromConfigSection(section)) {
    return false;
  }

  for (ConfigSection::const_iterator tn =
           section.begin(); tn != section.end(); ++tn) {

    if (tn->first == "neighbor") {
      try {
        ConfigSection CommandAttriTree = tn->second;
        std::string name = CommandAttriTree.get<std::string>("name");
        std::string uriString = CommandAttriTree.get<std::string>("face-uri");

        ndn::FaceUri faceUri;
        if (! faceUri.parse(uriString)) {
          std::cerr << "parsing failed!" << std::endl;
          return false;
        }

        double linkCost = CommandAttriTree.get<double>("link-cost",
                                                       Adjacent::DEFAULT_LINK_COST);
        ndn::Name neighborName(name);
        if (!neighborName.empty()) {
          Adjacent adj(name, faceUri, linkCost, Adjacent::STATUS_INACTIVE, 0, 0);
          m_nlsr.getAdjacencyList().insert(adj);
        }
        else {
          std::cerr << " Wrong command format ! [name /nbr/name/ \n face-uri /uri\n]";
          std::cerr << " or bad URI format" << std::endl;
        }
      }
      catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return false;
      }
    }
  }
  return true;
}

bool
ConfFileProcessor::processConfSectionHyperbolic(const ConfigSection& section)
{
  // state
  std::string state = section.get<std::string>("state", "off");

  if (boost::iequals(state, "off")) {
    m_nlsr.getConfParameter().setHyperbolicState(HYPERBOLIC_STATE_OFF);
  }
  else if (boost::iequals(state, "on")) {
    m_nlsr.getConfParameter().setHyperbolicState(HYPERBOLIC_STATE_ON);
  }
  else if (state == "dry-run") {
    m_nlsr.getConfParameter().setHyperbolicState(HYPERBOLIC_STATE_DRY_RUN);
  }
  else {
    std::cerr << "Wrong format for hyperbolic state." << std::endl;
    std::cerr << "Allowed value: off, on, dry-run" << std::endl;

    return false;
  }

  try {
    // Radius and angle(s) are mandatory configuration parameters in hyperbolic section.
    // Even if router can have hyperbolic routing calculation off but other router
    // in the network may use hyperbolic routing calculation for FIB generation.
    // So each router need to advertise its hyperbolic coordinates in the network
    double radius = section.get<double>("radius");
    std::string angleString = section.get<std::string>("angle");

    std::stringstream ss(angleString);
    std::vector<double> angles;

    double angle;

    while (ss >> angle) {
      angles.push_back(angle);
      if (ss.peek() == ',' || ss.peek() == ' ') {
        ss.ignore();
      }
    }

    if (!m_nlsr.getConfParameter().setCorR(radius)) {
      return false;
    }
    m_nlsr.getConfParameter().setCorTheta(angles);
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    if (state == "on" || state == "dry-run") {
      return false;
    }
  }

  return true;
}

bool
ConfFileProcessor::processConfSectionFib(const ConfigSection& section)
{
  // max-faces-per-prefix
  int maxFacesPerPrefix = section.get<int>("max-faces-per-prefix", MAX_FACES_PER_PREFIX_DEFAULT);

  if (maxFacesPerPrefix >= MAX_FACES_PER_PREFIX_MIN &&
      maxFacesPerPrefix <= MAX_FACES_PER_PREFIX_MAX)
  {
    m_nlsr.getConfParameter().setMaxFacesPerPrefix(maxFacesPerPrefix);
  }
  else {
    std::cerr << "Wrong value for max-faces-per-prefix. ";
    std::cerr << MAX_FACES_PER_PREFIX_MIN << std::endl;

    return false;
  }

  // routing-calc-interval
  ConfigurationVariable<uint32_t> routingCalcInterval("routing-calc-interval",
                                                      std::bind(&ConfParameter::setRoutingCalcInterval,
                                                      &m_nlsr.getConfParameter(), _1));
  routingCalcInterval.setMinAndMaxValue(ROUTING_CALC_INTERVAL_MIN, ROUTING_CALC_INTERVAL_MAX);
  routingCalcInterval.setOptional(ROUTING_CALC_INTERVAL_DEFAULT);

  if (!routingCalcInterval.parseFromConfigSection(section)) {
    return false;
  }

  return true;
}

bool
ConfFileProcessor::processConfSectionAdvertising(const ConfigSection& section)
{
  for (ConfigSection::const_iterator tn =
         section.begin(); tn != section.end(); ++tn) {
   if (tn->first == "prefix") {
     try {
       std::string prefix = tn->second.data();
       ndn::Name namePrefix(prefix);
       if (!namePrefix.empty()) {
         m_nlsr.getNamePrefixList().insert(namePrefix);
       }
       else {
         std::cerr << " Wrong command format ! [prefix /name/prefix] or bad URI" << std::endl;
         return false;
       }
     }
     catch (const std::exception& ex) {
       std::cerr << ex.what() << std::endl;
       return false;
     }
    }
  }
  return true;
}

bool
ConfFileProcessor::processConfSectionSecurity(const ConfigSection& section)
{
  ConfigSection::const_iterator it = section.begin();

  if (it == section.end() || it->first != "validator") {
    std::cerr << "Error: Expect validator section!" << std::endl;
    return false;
  }

  m_nlsr.loadValidator(it->second, m_confFileName);

  it++;
  if (it != section.end() && it->first == "prefix-update-validator") {
    m_nlsr.getPrefixUpdateProcessor().loadValidator(it->second, m_confFileName);

    it++;
    for (; it != section.end(); it++) {
      using namespace boost::filesystem;

      if (it->first != "cert-to-publish") {
        std::cerr << "Error: Expect cert-to-publish!" << std::endl;
        return false;
      }

      std::string file = it->second.data();
      path certfilePath = absolute(file, path(m_confFileName).parent_path());
      std::shared_ptr<ndn::security::v2::Certificate> idCert =
        ndn::io::load<ndn::security::v2::Certificate>(certfilePath.string());

      if (idCert == nullptr) {
        std::cerr << "Error: Cannot load cert-to-publish: " << file << "!" << std::endl;
        return false;
      }

      m_nlsr.loadCertToPublish(*idCert);
    }
  }

  return true;
}

} // namespace nlsr
