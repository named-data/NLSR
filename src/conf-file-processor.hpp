#ifndef CONF_PROCESSOR_HPP
#define CONF_PROCESSOR_HPP

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/cstdint.hpp>

#include "nlsr.hpp"

namespace nlsr {

class ConfFileProcessor
{
public:
  ConfFileProcessor(Nlsr& nlsr, const std::string& cfile)
    : m_confFileName(cfile)
    , m_nlsr(nlsr)
  {
  }

  bool
  processConfFile();

private:
  bool
  load(std::istream& input);

  bool
  processSection(const std::string& section,
                 boost::property_tree::ptree SectionAttributeTree);

  bool
  processConfSectionGeneral(boost::property_tree::ptree SectionAttributeTree);

  bool
  processConfSectionNeighbors(boost::property_tree::ptree SectionAttributeTree);

  bool
  processConfSectionHyperbolic(boost::property_tree::ptree SectionAttributeTree);

  bool
  processConfSectionFib(boost::property_tree::ptree SectionAttributeTree);

  bool
  processConfSectionAdvertising(boost::property_tree::ptree SectionAttributeTree);

private:
  std::string m_confFileName;
  Nlsr& m_nlsr;
};

} //namespace nlsr
#endif //CONF_PROCESSOR_HPP
