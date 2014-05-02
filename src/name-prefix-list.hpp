#ifndef NLSR_NAME_PREFIX_LIST_HPP
#define NLSR_NAME_PREFIX_LIST_HPP

#include <list>
#include <string>
#include <boost/cstdint.hpp>


namespace nlsr {
class NamePrefixList
{

public:
  NamePrefixList();

  ~NamePrefixList();

  int32_t
  insert(const std::string& name);

  int32_t
  remove(const std::string& name);

  void
  sort();

  int32_t
  getSize()
  {
    return m_nameList.size();
  }

  std::list<std::string>&
  getNameList()
  {
    return m_nameList;
  }

  void
  print();

private:
  std::list<std::string> m_nameList;

};

}//namespace nlsr

#endif //NLSR_NAME_PREFIX_LIST_HPP
