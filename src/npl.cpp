#include <iostream>
#include <algorithm>

#include "npl.hpp"

namespace nlsr {

using namespace std;

Npl::Npl()
{
}

Npl::~Npl()
{
}

static bool
nameCompare(string& s1, string& s2)
{
  return s1 == s2;
}

int
Npl::insert(string& name)
{
  std::list<string>::iterator it = std::find_if(m_nameList.begin(),
                                                m_nameList.end(),
                                                bind(&nameCompare, _1 , name));
  if (it != m_nameList.end())
  {
    return -1;
  }
  m_nameList.push_back(name);
  return 0;
}

int
Npl::remove(string& name)
{
  std::list<string>::iterator it = std::find_if(m_nameList.begin(),
                                                m_nameList.end(),
                                                bind(&nameCompare, _1 , name));
  if (it != m_nameList.end())
  {
    m_nameList.erase(it);
  }
  return -1;
}

void
Npl::sort()
{
  m_nameList.sort();
}

void
Npl::print()
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
