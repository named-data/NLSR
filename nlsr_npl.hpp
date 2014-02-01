#ifndef NPL_HPP
#define NPL_HPP

#include<list>
#include<string>
#include <ndn-cpp-dev/face.hpp>

using namespace std;

class Npl{
	
public:
	Npl();
	~Npl();

	int insertIntoNpl(string& name);
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

#endif
