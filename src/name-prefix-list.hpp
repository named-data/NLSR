#ifndef NPL_HPP
#define NPL_HPP

#include <list>
#include <string>


namespace nlsr {
class NamePrefixList
{

public:
  NamePrefixList();

  ~NamePrefixList();

  int
  insert(std::string& name);

  int
  remove(std::string& name);

  void
  sort();

  int
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

#endif //NPL_HPP
