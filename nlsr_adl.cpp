#include<iostream>
#include<algorithm>

#include "nlsr_adl.hpp"
#include "nlsr_adjacent.hpp"

Adl::Adl(){
}

Adl::~Adl(){

}

static bool
adjacent_compare(Adjacent& adj1, Adjacent& adj2){
	return adj1.getAdjacentName()==adj2.getAdjacentName();
}

int
Adl::insert(Adjacent& adj){
	std::list<Adjacent >::iterator it = std::find_if( adjList.begin(), 
								adjList.end(),	
   								bind(&adjacent_compare, _1, adj));
	if ( it != adjList.end() ){
		return -1;
	}
	adjList.push_back(adj);
	return 0;
}
int
Adl::updateAdjacentStatus(string adjName, int s){
	Adjacent adj(adjName);
	
	std::list<Adjacent >::iterator it = std::find_if( adjList.begin(), 
								adjList.end(),	
   								bind(&adjacent_compare, _1, adj));

	if( it == adjList.end()){
		return -1;
	}

	(*it).setStatus(s);
	return 0;
	

}

int 
Adl::updateAdjacentLinkCost(string adjName, double lc){
	Adjacent adj(adjName);
	
	std::list<Adjacent >::iterator it = std::find_if( adjList.begin(), 
								adjList.end(),	
   								bind(&adjacent_compare, _1, adj));

	if( it == adjList.end()){
		return -1;
	}

	(*it).setLinkCost(lc);
	return 0;

}

std::list<Adjacent> 
Adl::getAdjList(){
	return adjList;
}

// used for debugging purpose
void
Adl::printAdl(){
	for( std::list<Adjacent>::iterator it=adjList.begin(); it!= adjList.end() ; it++){
		cout<< (*it) <<endl;
	}
}
