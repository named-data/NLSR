#ifndef NPL_HPP
#define NPL_HPP

#include<list>
#include<string>
#include <ndn-cpp-dev/face.hpp>

namespace nlsr
{

  using namespace std;

  class Npl
  {

  public:
    Npl();
    ~Npl();

    int insert(string& name);
    int remove(string& name);
    void sort();
    int getSize()
    {
      return m_nameList.size();
    }
    std::list<string>& getNameList()
    {
      return m_nameList;
    }
    void print();

  private:
    std::list<string> m_nameList;

  };

}//namespace nlsr

#endif
