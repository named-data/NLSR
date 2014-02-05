#include <iostream>
#include "nlsr_lsdb.hpp"
#include "nlsr_rtc.hpp"
#include "nlsr_map.hpp"
#include "nlsr_lsa.hpp"
#include "nlsr.hpp"

using namespace std;

void 
RoutingTableCalculator::allocateAdjMatrix()
{
	adjMatrix = new double*[numOfRouter];
	for(int i = 0; i < numOfRouter; ++i) 
	{
    adjMatrix[i] = new double[numOfRouter];
	}
}

void
RoutingTableCalculator::makeAdjMatrix(nlsr& pnlsr, Map pMap)
{
	std::list<AdjLsa> adjLsdb=pnlsr.getLsdb().getAdjLsdb();
	for( std::list<AdjLsa>::iterator it=adjLsdb.begin(); 
	                                                 it!= adjLsdb.end() ; it++)
	{
		string linkStartRouter=(*it).getOrigRouter();
		int row=pMap.getMappingNoByRouterName(linkStartRouter);
		std::list<Adjacent> adl=(*it).getAdl().getAdjList();
		for( std::list<Adjacent>::iterator itAdl=adl.begin(); 
																								itAdl!= adl.end() ; itAdl++)
		{
			string linkEndRouter=(*itAdl).getAdjacentName();
			int col=pMap.getMappingNoByRouterName(linkEndRouter);
			double cost=(*itAdl).getLinkCost();
			if ( (row >= 0 && row<numOfRouter) && (col >= 0 && col<numOfRouter) )
			{
				adjMatrix[row][col]=cost;
			}
		}
		
	}
}

void
RoutingTableCalculator::printAdjMatrix()
{
	for(int i=0;i<numOfRouter;i++)
	{
		for(int j=0;j<numOfRouter;j++)
			printf("%f ",adjMatrix[i][j]);
		printf("\n");
	}
}

void 
RoutingTableCalculator::adjustAdMatrix(int source, int link, double linkCost)
{
	for ( int i = 0; i < numOfRouter; i++ ){
		if ( i == link ){
			adjMatrix[source][i]=linkCost;
		}
		else{
			adjMatrix[source][i]=0;
		}
	}
}

int 
RoutingTableCalculator::getNumOfLinkfromAdjMatrix(int sRouter)
{
	int noLink=0;
	for(int i=0;i<numOfRouter;i++)
	{	
		if ( adjMatrix[sRouter][i] > 0 )
		{
			noLink++;
		}
	}
	return noLink;
}

void 
RoutingTableCalculator::getLinksFromAdjMatrix(int *links, 
                                                 double *linkCosts, int source)
{
	int j=0;
	for (int i=0; i <numOfRouter; i++)
	{
		if ( adjMatrix[source][i] > 0 )
		{
			links[j]=i;
			linkCosts[j]=adjMatrix[source][i];
			j++;
		}
	}
}

void 
RoutingTableCalculator::freeAdjMatrix()
{
	for(int i = 0; i < numOfRouter; ++i) 
	{
    	delete [] adjMatrix[i];
	}
	delete [] adjMatrix;
}

void
LinkStateRoutingTableCalculator::calculatePath(Map& pMap, 
                                                  RoutingTable& rt, nlsr& pnlsr)
{
	cout<<"LinkStateRoutingTableCalculator::calculatePath Called"<<endl;
	allocateAdjMatrix();
	makeAdjMatrix(pnlsr,pMap);
	cout<<pMap;
	printAdjMatrix();
	string routerName=pnlsr.getConfParameter().getRouterPrefix();
	int sourceRouter=pMap.getMappingNoByRouterName(routerName);
	cout<<"Calculating Router: "<< routerName <<" Mapping no: "<<sourceRouter<<endl;
	int noLink=getNumOfLinkfromAdjMatrix(sourceRouter);
	allocateParent();
	allocateDistance();

	if ( pnlsr.getConfParameter().getMaxFacesPerPrefix() == 1 )
	{
		// Single Path
		doDijkstraPathCalculation(sourceRouter);
		// print all ls path -- debugging purpose
		printAllLsPath(sourceRouter);
		// update routing table
		// update NPT ( FIB )
	}
	else
	{
		// Multi Path
		setNoLink(getNumOfLinkfromAdjMatrix(sourceRouter));
		allocateLinks();
		allocateLinkCosts();

		getLinksFromAdjMatrix(links, linkCosts, sourceRouter);
		for (int i=0 ; i < vNoLink; i++)
		{
			adjustAdMatrix(sourceRouter,links[i], linkCosts[i]);
			doDijkstraPathCalculation(sourceRouter);
			//update routing table
			//update NPT ( FIB )
		}
		
		freeLinks();
	  freeLinksCosts();
	}

	freeParent();
	freeDistance();
	freeAdjMatrix();
}

void 
LinkStateRoutingTableCalculator::doDijkstraPathCalculation(int sourceRouter)
{
	int i;
	int v,u;
	int *Q=new int[numOfRouter];
	int head=0;
	/* Initiate the Parent */
	for (i = 0 ; i < numOfRouter; i++){
		parent[i]=EMPTY_PARENT;
		distance[i]=INF_DISTANCE;
		Q[i]=i;
	} 

	if ( sourceRouter != NO_MAPPING_NUM ){
		distance[sourceRouter]=0;
		sortQueueByDistance(Q,distance,head,numOfRouter);

		while (head < numOfRouter )
		{
			u=Q[head];
			if(distance[u] == INF_DISTANCE)
			{
				break;
			}

			for(v=0 ; v <numOfRouter; v++)
			{
				if( adjMatrix[u][v] > 0 )
				{
					if ( isNotExplored(Q,v,head+1,numOfRouter) )
					{
						if( distance[u] + adjMatrix[u][v] <  distance[v])
						{
							distance[v]=distance[u] + adjMatrix[u][v] ;
							parent[v]=u;
						}	

					}

				}

			}

			head++;
			sortQueueByDistance(Q,distance,head,numOfRouter);
		}
	}
	delete [] Q;
}

void
LinkStateRoutingTableCalculator::printAllLsPath(int sourceRouter)
{
	cout<<"LinkStateRoutingTableCalculator::printAllLsPath Called"<<endl;
	cout<<"Source Router: "<<sourceRouter<<endl;
	for(int i=0; i < numOfRouter ; i++)
	{
		if ( i!= sourceRouter )
		{
			printLsPath(i);
			cout<<endl;
		}
	}
}

void
LinkStateRoutingTableCalculator::printLsPath(int destRouter)
{
	if (parent[destRouter] != EMPTY_PARENT )
	{
		printLsPath(parent[destRouter]);
	}

	cout<<" "<<destRouter;
}

void 
LinkStateRoutingTableCalculator::sortQueueByDistance(int *Q,
                                             double *dist,int start,int element)
{
	for ( int i=start ; i < element ; i ++) 
	{
		for( int j=i+1; j<element; j ++)
		{
			if (dist[Q[j]] < dist[Q[i]])
			{
				int tempU=Q[j];
				Q[j]=Q[i];
				Q[i]=tempU;
			}
		}
	}

}

int 
LinkStateRoutingTableCalculator::isNotExplored(int *Q, 
                                                   int u,int start, int element)
{
	int ret=0;
	for(int i=start; i< element; i++)
	{
		if ( Q[i] == u )
		{
			ret=1;
			break;
		}
	}
	return ret;
}

void 
LinkStateRoutingTableCalculator::allocateParent()
{
	parent=new int[numOfRouter];
}

void 
LinkStateRoutingTableCalculator::allocateDistance()
{
	distance= new double[numOfRouter];
}

void 
LinkStateRoutingTableCalculator::freeParent()
{
	delete [] parent;
}

void LinkStateRoutingTableCalculator::freeDistance()
{
	delete [] distance;
}

void 
LinkStateRoutingTableCalculator::allocateLinks()
{
	links=new int[vNoLink];
}

void LinkStateRoutingTableCalculator::allocateLinkCosts()
{
	linkCosts=new double[vNoLink];
}

void 
LinkStateRoutingTableCalculator::freeLinks()
{
	delete [] links;
}
void 
LinkStateRoutingTableCalculator::freeLinksCosts()
{
	delete [] linkCosts;
}

void
HypRoutingTableCalculator::calculatePath(Map& pMap, 
                                                  RoutingTable& rt, nlsr& pnlsr)
{
}

void
HypDryRoutingTableCalculator::calculatePath(Map& pMap, 
                                                  RoutingTable& rt, nlsr& pnlsr)
{
}

