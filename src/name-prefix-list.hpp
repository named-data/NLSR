#ifndef NLSR_NAME_PREFIX_LIST_HPP
#define NLSR_NAME_PREFIX_LIST_HPP

#include <list>
#include <string>
#include <boost/cstdint.hpp>
#include <ndn-cxx/name.hpp>


namespace nlsr {
class NamePrefixList
{

public:
  NamePrefixList();

  ~NamePrefixList();

  int32_t
  insert(const ndn::Name& name);

  int32_t
  remove(const ndn::Name& name);

  void
  sort();

  size_t
  getSize()
  {
    return m_nameList.size();
  }

  std::list<ndn::Name>&
  getNameList()
  {
    return m_nameList;
  }

  void
  print();

private:
  std::list<ndn::Name> m_nameList;

};

}//namespace nlsr

#endif //NLSR_NAME_PREFIX_LIST_HPP
