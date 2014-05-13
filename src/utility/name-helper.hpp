#ifndef NLSR_NAME_HELPER_HPP
#define NLSR_NAME_HELPER_HPP

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex_find_format.hpp>
#include <boost/regex.hpp>
#include <boost/cstdint.hpp>
#include <ndn-cxx/name-component.hpp>
#include <ndn-cxx/name.hpp>

namespace nlsr {
namespace util {
/**
 * @brief search a name component in ndn::Name and return the position of the component
 * @param name where to search the searchString
 * @param searchString, the string to search in name
 * @retrun int32_t -1 if searchString not found else return the position
 * starting from 0
 */

inline static int32_t
getNameComponentPosition(const ndn::Name& name, const std::string& searchString)
{
  ndn::name::Component component(searchString);
  size_t nameSize = name.size();
  for (uint32_t i = 0; i < nameSize; i++) {
    if (component == name[i]) {
      return (int32_t)i;
    }
  }
  return -1;
}

} //namespace util

} // namespace nlsr

#endif //NLSR_NAME_HELPER_HPP
