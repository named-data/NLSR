#ifndef NPL_HPP
#define NPL_HPP

#include<list>
#include<string>
#include <ndn-cpp-dev/face.hpp>

namespace nlsr {

using namespace std;

class Npl{
	
public:
	Npl();
	~Npl();

	int insertIntoNpl(string& name);
	int removeFromNpl(string& name);
	void sortNpl();
	int getNplSize()
	{
		return nameList.size();
	}
	std::list<string>& getNameList()
	{
		return nameList;
	}
	void printNpl();

private:
	std::list<string> nameList;

};

}//namespace nlsr

#endif
