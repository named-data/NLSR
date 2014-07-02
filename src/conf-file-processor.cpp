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
 * \author A K M Mahmudul Hoque <ahoque1@memphis.edu>
 * \author Minsheng Zhang <mzhang4@memphis.edu>
 *
 **/
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>

#include <ndn-cxx/name.hpp>

#include "conf-parameter.hpp"
#include "conf-file-processor.hpp"
#include "adjacent.hpp"
#include "utility/name-helper.hpp"


namespace nlsr {

using namespace std;

bool
ConfFileProcessor::processConfFile()
{
  bool ret = true;
  ifstream inputFile;
  inputFile.open(m_confFileName.c_str());
  if (!inputFile.is_open()) {
    string msg = "Failed to read configuration file: ";
    msg += m_confFileName;
    cerr << msg << endl;
    return false;
  }
  ret = load(inputFile);
  inputFile.close();
  return ret;
}

bool
ConfFileProcessor::load(istream& input)
{
  boost::property_tree::ptree pt;
  bool ret = true;
  try {
    boost::property_tree::read_info(input, pt);
  }
  catch (const boost::property_tree::info_parser_error& error) {
    stringstream msg;
    std::cerr << "Failed to parse configuration file " << std::endl;
    std::cerr << m_confFileName << std::endl;
    return false;
  }
  for (boost::property_tree::ptree::const_iterator tn = pt.begin();
       tn != pt.end(); ++tn) {
    std::string section = tn->first;
    boost::property_tree::ptree SectionAttributeTree = tn ->second;
    ret = processSection(section, SectionAttributeTree);
    if (ret == false) {
      break;
    }
  }
  return ret;
}

bool
ConfFileProcessor::processSection(const std::string& section,
                                  boost::property_tree::ptree SectionAttributeTree)
{
  bool ret = true;
  if (section == "general")
  {
    ret = processConfSectionGeneral(SectionAttributeTree);
  }
  else if (section == "neighbors")
  {
    ret = processConfSectionNeighbors(SectionAttributeTree);
  }
  else if (section == "hyperbolic")
  {
    ret = processConfSectionHyperbolic(SectionAttributeTree);
  }
  else if (section == "fib")
  {
    ret = processConfSectionFib(SectionAttributeTree);
  }
  else if (section == "advertising")
  {
    ret = processConfSectionAdvertising(SectionAttributeTree);
  }
  else if (section == "security")
  {
    ret = processConfSectionSecurity(SectionAttributeTree);
  }
  else
  {
    std::cerr << "Wrong configuration Command: " << section << std::endl;
  }
  return ret;
}

bool
ConfFileProcessor::processConfSectionGeneral(boost::property_tree::ptree
                                             SectionAttributeTree)
{
  try {
    std::string network = SectionAttributeTree.get<string>("network");
    std::string site = SectionAttributeTree.get<string>("site");
    std::string router = SectionAttributeTree.get<string>("router");
    ndn::Name networkName(network);
    if (!networkName.empty()) {
      m_nlsr.getConfParameter().setNetwork(networkName);
    }
    else {
      cerr << " Network can not be null or empty or in bad URI format :(!" << endl;
      return false;
    }
    ndn::Name siteName(site);
    if (!siteName.empty()) {
      m_nlsr.getConfParameter().setSiteName(siteName);
    }
    else {
      cerr << "Site can not be null or empty or in bad URI format:( !" << endl;
      return false;
    }
    ndn::Name routerName(router);
    if (!routerName.empty()) {
      m_nlsr.getConfParameter().setRouterName(routerName);
    }
    else {
      cerr << " Router name can not be null or empty or in bad URI format:( !" << endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    cerr << ex.what() << endl;
    return false;
  }

  try {
    int32_t lsaRefreshTime = SectionAttributeTree.get<int32_t>("lsa-refresh-time");
    if (lsaRefreshTime >= LSA_REFRESH_TIME_MIN &&
        lsaRefreshTime <= LSA_REFRESH_TIME_MAX) {
      m_nlsr.getConfParameter().setLsaRefreshTime(lsaRefreshTime);
    }
    else {
      std::cerr << "Wrong value for lsa-refresh-time ";
      std::cerr << "Allowed value: " << LSA_REFRESH_TIME_MIN << "-";;
      std::cerr << LSA_REFRESH_TIME_MAX << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    return false;
  }

  try {
    std::string logLevel = SectionAttributeTree.get<string>("log-level");
    if ( boost::iequals(logLevel, "info") || boost::iequals(logLevel, "debug")) {
      m_nlsr.getConfParameter().setLogLevel(logLevel);
    }
    else {
      std::cerr << "Wrong value for log-level ";
      std::cerr << "Allowed value: INFO, DEBUG" << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    return false;
  }

  try {
    std::string logDir = SectionAttributeTree.get<string>("log-dir");
    if (boost::filesystem::exists(logDir)) {
      if (boost::filesystem::is_directory(logDir)) {
        std::string testFileName=logDir+"/test.log";
        ofstream testOutFile;
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
      std::cerr << "Log directory provided does not exists" << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << "You must configure log directory" << std::endl;
    std::cerr << ex.what() << std::endl;
    return false;
  }
  try {
    std::string seqDir = SectionAttributeTree.get<string>("seq-dir");
    if (boost::filesystem::exists(seqDir)) {
      if (boost::filesystem::is_directory(seqDir)) {
        std::string testFileName=seqDir+"/test.seq";
        ofstream testOutFile;
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
      std::cerr << "Seq directory provided does not exists" << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << "You must configure sequence directory" << std::endl;
    std::cerr << ex.what() << std::endl;
    return false;
  }

  return true;
}

bool
ConfFileProcessor::processConfSectionNeighbors(boost::property_tree::ptree
                                           SectionAttributeTree)
{
  try {
    int retrials = SectionAttributeTree.get<int>("hello-retries");
    if (retrials >= HELLO_RETRIES_MIN && retrials <= HELLO_RETRIES_MAX) {
      m_nlsr.getConfParameter().setInterestRetryNumber(retrials);
    }
    else {
      std::cerr << "Wrong value for hello-retries. ";
      std::cerr << "Allowed value:" << HELLO_RETRIES_MIN << "-";
      std::cerr << HELLO_RETRIES_MAX << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    return false;
  }
  try {
    int timeOut = SectionAttributeTree.get<int>("hello-timeout");
    if (timeOut >= HELLO_TIMEOUT_MIN && timeOut <= HELLO_TIMEOUT_MAX) {
      m_nlsr.getConfParameter().setInterestResendTime(timeOut);
    }
    else {
      std::cerr << "Wrong value for hello-timeout. ";
      std::cerr << "Allowed value:" << HELLO_TIMEOUT_MIN << "-";
      std::cerr << HELLO_TIMEOUT_MAX << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
  }
  try {
    int interval = SectionAttributeTree.get<int>("hello-interval");
    if (interval >= HELLO_INTERVAL_MIN && interval <= HELLO_INTERVAL_MAX) {
      m_nlsr.getConfParameter().setInfoInterestInterval(interval);
    }
    else {
      std::cerr << "Wrong value for hello-interval. ";
      std::cerr << "Allowed value:" << HELLO_INTERVAL_MIN << "-";
      std::cerr << HELLO_INTERVAL_MAX << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
  }
  for (boost::property_tree::ptree::const_iterator tn =
           SectionAttributeTree.begin(); tn != SectionAttributeTree.end(); ++tn) {

    if (tn->first == "neighbor")
    {
      try {
        boost::property_tree::ptree CommandAttriTree = tn->second;
        std::string name = CommandAttriTree.get<std::string>("name");
        std::string faceUri = CommandAttriTree.get<std::string>("face-uri");
        double linkCost = CommandAttriTree.get<double>("link-cost",
                                                       Adjacent::DEFAULT_LINK_COST);
        ndn::Name neighborName(name);
        if (!neighborName.empty()) {
          Adjacent adj(name, faceUri, linkCost, ADJACENT_STATUS_INACTIVE, 0, 0);
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
ConfFileProcessor::processConfSectionHyperbolic(boost::property_tree::ptree
                                                SectionAttributeTree)
{
  std::string state;
  try {
    state= SectionAttributeTree.get<string>("state","off");
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
  }
  catch (const std::exception& ex) {
    std::cerr << ex.what() << std::endl;
    return false;
  }

  try {
    /* Radius and angle is mandatory configuration parameter in hyperbolic section.
     * Even if router can have hyperbolic routing calculation off but other router
     * in the network may use hyperbolic routing calculation for FIB generation.
     * So each router need to advertise its hyperbolic coordinates in the network
     */
    double radius = SectionAttributeTree.get<double>("radius");
    double angle = SectionAttributeTree.get<double>("angle");
    if (!m_nlsr.getConfParameter().setCorR(radius)) {
      return false;
    }
    m_nlsr.getConfParameter().setCorTheta(angle);
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
ConfFileProcessor::processConfSectionFib(boost::property_tree::ptree
                                         SectionAttributeTree)
{
  try {
    int maxFacesPerPrefixNumber =
      SectionAttributeTree.get<int>("max-faces-per-prefix");
    if (maxFacesPerPrefixNumber >= MAX_FACES_PER_PREFIX_MIN &&
        maxFacesPerPrefixNumber <= MAX_FACES_PER_PREFIX_MAX)
    {
      m_nlsr.getConfParameter().setMaxFacesPerPrefix(maxFacesPerPrefixNumber);
    }
    else {
      std::cerr << "Wrong value for max-faces-per-prefix. ";
      std::cerr << MAX_FACES_PER_PREFIX_MIN << std::endl;
      return false;
    }
  }
  catch (const std::exception& ex) {
    cerr << ex.what() << endl;
    return false;
  }
  return true;
}

bool
ConfFileProcessor::processConfSectionAdvertising(boost::property_tree::ptree
                                                 SectionAttributeTree)
{
  for (boost::property_tree::ptree::const_iterator tn =
         SectionAttributeTree.begin(); tn != SectionAttributeTree.end(); ++tn) {
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
ConfFileProcessor::processConfSectionSecurity(boost::property_tree::ptree section)
{
  ConfigSection::const_iterator it = section.begin();

  if (it == section.end() || it->first != "validator")
    {
      std::cerr << "Error: Expect validator section!" << std::endl;
      return false;
    }

  m_nlsr.loadValidator(it->second, m_confFileName);
  it++;

  for (; it != section.end(); it++)
    {
      using namespace boost::filesystem;
      if (it->first != "cert-to-publish")
        {
          std::cerr << "Error: Expect cert-to-publish!" << std::endl;
          return false;
        }

      std::string file = it->second.data();
      path certfilePath = absolute(file, path(m_confFileName).parent_path());
      ndn::shared_ptr<ndn::IdentityCertificate> idCert =
        ndn::io::load<ndn::IdentityCertificate>(certfilePath.string());

      if (!static_cast<bool>(idCert))
        {
          std::cerr << "Error: Cannot load cert-to-publish: " << file << "!" << std::endl;
          return false;
        }

      m_nlsr.loadCertToPublish(idCert);
    }

  return true;
}

}//namespace NLSR
