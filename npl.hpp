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
	void printNpl();

private:
	std::list<string> nameList;

};

#endif
