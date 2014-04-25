#include <iostream>
#include <algorithm>

#include <ndn-cxx/common.hpp>

#include "name-prefix-list.hpp"

namespace nlsr {

using namespace std;

NamePrefixList::NamePrefixList()
{
}

NamePrefixList::~NamePrefixList()
{
}

static bool
nameCompare(const string& s1, const string& s2)
{
  return s1 == s2;
}

int
NamePrefixList::insert(string& name)
{
  std::list<string>::iterator it = std::find_if(m_nameList.begin(),
                                                m_nameList.end(),
                                                ndn::bind(&nameCompare, _1 , name));
  if (it != m_nameList.end())
  {
    return -1;
  }
  m_nameList.push_back(name);
  return 0;
}

int
NamePrefixList::remove(string& name)
{
  std::list<string>::iterator it = std::find_if(m_nameList.begin(),
                                                m_nameList.end(),
                                                ndn::bind(&nameCompare, _1 , name));
  if (it != m_nameList.end())
  {
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
NamePrefixList::print()
{
  int i = 1;
  for (std::list<string>::iterator it = m_nameList.begin();
       it != m_nameList.end();
       it++)
  {
    cout << "Name " << i << " : " << (*it) << endl;
    i++;
  }
}

}//namespace nlsr
