#include<iostream>
#include<string>

#include "adjacent.hpp"

using namespace std;

Adjacent::Adjacent(const string& an, int cf, double lc, int s, int iton){
	adjacentName=an;
	connectingFace=cf;
	linkCost=lc;
	status=s;
	interestTimedOutNo=iton;
}

std::ostream&
operator << (std::ostream &os, Adjacent &adj){
	cout<<"Adjacent : "<< adj.getAdjacentName()	<< endl;
	cout<<"Connecting Face: "<<adj.getConnectingFace()<<endl;
	cout<<"Link Cost: "<<adj.getLinkCost()<<endl;
	cout<<"Status: "<<adj.getStatus()<<endl;
	cout<<"Interest Timed out: "<<adj.getInterestTimedOutNo()<<endl;
	return os;
}
