#include <iostream>
#include <algorithm>

#include <ndn-cxx/common.hpp>

#include "name-prefix-list.hpp"
#include "logger.hpp"

namespace nlsr {

INIT_LOGGER("NamePrefixList");

using namespace std;

NamePrefixList::NamePrefixList()
{
}

NamePrefixList::~NamePrefixList()
{
}

static bool
nameCompare(const ndn::Name& name1, const ndn::Name& name2)
{
  return name1 == name2;
}

int32_t
NamePrefixList::insert(const ndn::Name& name)
{
  std::list<ndn::Name>::iterator it = std::find_if(m_nameList.begin(),
                                                   m_nameList.end(),
                                                   ndn::bind(&nameCompare, _1 ,
                                                             ndn::cref(name)));
  if (it != m_nameList.end()) {
    return -1;
  }
  m_nameList.push_back(name);
  return 0;
}

int32_t
NamePrefixList::remove(const ndn::Name& name)
{
  std::list<ndn::Name>::iterator it = std::find_if(m_nameList.begin(),
                                                   m_nameList.end(),
                                                   ndn::bind(&nameCompare, _1 ,
                                                   ndn::cref(name)));
  if (it != m_nameList.end()) {
    m_nameList.erase(it);
  }
  return -1;
}

void
NamePrefixList::sort()
{
  m_nameList.sort();
}

void
NamePrefixList::writeLog()
{
  _LOG_DEBUG("-------Name Prefix List--------");
  int i = 1;
  for (std::list<ndn::Name>::iterator it = m_nameList.begin();
       it != m_nameList.end(); it++) {
    _LOG_DEBUG("Name " << i << " : " << (*it));
    i++;
  }
}

void
NamePrefixList::print()
{
  int i = 1;
  for (std::list<ndn::Name>::iterator it = m_nameList.begin();
       it != m_nameList.end(); it++) {
    cout << "Name " << i << " : " << (*it) << endl;
    i++;
  }
}

}//namespace nlsr
