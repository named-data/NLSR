#include <iostream>
#include <cmath>
#include "nlsr_lsdb.hpp"
#include "nlsr_rtc.hpp"
#include "nlsr_map.hpp"
#include "nlsr_lsa.hpp"
#include "nlsr_nexthop.hpp"
#include "nlsr.hpp"

namespace nlsr
{

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
    RoutingTableCalculator::initMatrix()
    {
        for(int i=0; i<numOfRouter; i++)
        {
            for(int j=0; j<numOfRouter; j++)
                adjMatrix[i][j]=0;
        }
    }

    void
    RoutingTableCalculator::makeAdjMatrix(Nlsr& pnlsr, Map pMap)
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
        for(int i=0; i<numOfRouter; i++)
        {
            for(int j=0; j<numOfRouter; j++)
                printf("%f ",adjMatrix[i][j]);
            printf("\n");
        }
    }

    void
    RoutingTableCalculator::adjustAdMatrix(int source, int link, double linkCost)
    {
        for ( int i = 0; i < numOfRouter; i++ )
        {
            if ( i == link )
            {
                adjMatrix[source][i]=linkCost;
            }
            else
            {
                adjMatrix[source][i]=0;
            }
        }
    }

    int
    RoutingTableCalculator::getNumOfLinkfromAdjMatrix(int sRouter)
    {
        int noLink=0;
        for(int i=0; i<numOfRouter; i++)
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
    RoutingTableCalculator::allocateLinks()
    {
        links=new int[vNoLink];
    }

    void RoutingTableCalculator::allocateLinkCosts()
    {
        linkCosts=new double[vNoLink];
    }


    void
    RoutingTableCalculator::freeLinks()
    {
        delete [] links;
    }
    void
    RoutingTableCalculator::freeLinksCosts()
    {
        delete [] linkCosts;
    }

    void
    LinkStateRoutingTableCalculator::calculatePath(Map& pMap,
            RoutingTable& rt, Nlsr& pnlsr)
    {
        cout<<"LinkStateRoutingTableCalculator::calculatePath Called"<<endl;
        allocateAdjMatrix();
        initMatrix();
        makeAdjMatrix(pnlsr,pMap);
        cout<<pMap;
        printAdjMatrix();
        string routerName=pnlsr.getConfParameter().getRouterPrefix();
        int sourceRouter=pMap.getMappingNoByRouterName(routerName);
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
            addAllLsNextHopsToRoutingTable(pnlsr, rt, pMap, sourceRouter);
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
                printAdjMatrix();
                doDijkstraPathCalculation(sourceRouter);
                // print all ls path -- debugging purpose
                printAllLsPath(sourceRouter);
                //update routing table
                addAllLsNextHopsToRoutingTable(pnlsr, rt, pMap, sourceRouter);
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
        for (i = 0 ; i < numOfRouter; i++)
        {
            parent[i]=EMPTY_PARENT;
            distance[i]=INF_DISTANCE;
            Q[i]=i;
        }

        if ( sourceRouter != NO_MAPPING_NUM )
        {
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
    LinkStateRoutingTableCalculator::addAllLsNextHopsToRoutingTable(Nlsr& pnlsr,
            RoutingTable& rt, Map& pMap, int sourceRouter)
    {
        cout<<"LinkStateRoutingTableCalculator::addAllNextHopsToRoutingTable Called";
        cout<<endl;
        for(int i=0; i < numOfRouter ; i++)
        {
            if ( i!= sourceRouter )
            {
                int nextHopRouter=getLsNextHop(i,sourceRouter);
                double routeCost=distance[i];
                string nextHopRouterName=pMap.getRouterNameByMappingNo(nextHopRouter);
                int nxtHopFace=
                    pnlsr.getAdl().getAdjacent(nextHopRouterName).getConnectingFace();
                cout<<"Dest Router: "<<pMap.getRouterNameByMappingNo(i)<<endl;
                cout<<"Next hop Router: "<<nextHopRouterName<<endl;
                cout<<"Next hop Face: "<<nxtHopFace<<endl;
                cout<<"Route Cost: "<<routeCost<<endl;
                cout<<endl;
                // Add next hop to routing table
                NextHop nh(nxtHopFace,routeCost);
                rt.addNextHop(pMap.getRouterNameByMappingNo(i),nh);

            }
        }
    }

    int
    LinkStateRoutingTableCalculator::getLsNextHop(int dest, int source)
    {
        int nextHop;
        while ( parent[dest] != EMPTY_PARENT )
        {
            nextHop=dest;
            dest=parent[dest];

        }

        if ( dest != source )
        {
            nextHop=NO_NEXT_HOP;
        }

        return nextHop;
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
    HypRoutingTableCalculator::calculatePath(Map& pMap,
            RoutingTable& rt, Nlsr& pnlsr)
    {
        makeAdjMatrix(pnlsr,pMap);
        string routerName=pnlsr.getConfParameter().getRouterPrefix();
        int sourceRouter=pMap.getMappingNoByRouterName(routerName);
        int noLink=getNumOfLinkfromAdjMatrix(sourceRouter);
        setNoLink(noLink);
        allocateLinks();
        allocateLinkCosts();

        getLinksFromAdjMatrix(links, linkCosts, sourceRouter);

        for(int i=0 ; i < numOfRouter ; ++i)
        {
            int k=0;
            if ( i != sourceRouter)
            {
                allocateLinkFaces();
                allocateDistanceToNeighbor();
                allocateDistFromNbrToDest();

                for(int j=0; j<vNoLink; j++)
                {
                    string nextHopRouterName=pMap.getRouterNameByMappingNo(links[j]);
                    int nextHopFace=
                        pnlsr.getAdl().getAdjacent(nextHopRouterName).getConnectingFace();
                    double distToNbr=getHyperbolicDistance(pnlsr,pMap,
                                                           sourceRouter,links[j]);
                    double distToDestFromNbr=getHyperbolicDistance(pnlsr,
                                             pMap,links[j],i);
                    if ( distToDestFromNbr >= 0 )
                    {
                        linkFaces[k] = nextHopFace;
                        distanceToNeighbor[k] = distToNbr;
                        distFromNbrToDest[k] = distToDestFromNbr;
                        k++;
                    }
                }

                addHypNextHopsToRoutingTable(pnlsr,pMap,rt,k,i);

                freeLinkFaces();
                freeDistanceToNeighbor();
                freeDistFromNbrToDest();
            }
        }

        freeLinks();
        freeLinksCosts();
        freeAdjMatrix();
    }

    void
    HypRoutingTableCalculator::addHypNextHopsToRoutingTable(Nlsr& pnlsr,Map& pMap,
            RoutingTable& rt, int noFaces, int dest)
    {
        for(int i=0 ; i < noFaces ; ++i)
        {
            string destRouter=pMap.getRouterNameByMappingNo(dest);
            NextHop nh(linkFaces[i],distFromNbrToDest[i]);
            rt.addNextHop(destRouter,nh);
            if( isDryRun == 1 )
            {
                rt.addNextHopToDryTable(destRouter,nh);
            }
        }

    }

    double
    HypRoutingTableCalculator::getHyperbolicDistance(Nlsr& pnlsr,
            Map& pMap, int src, int dest)
    {
        double distance=0.0;

        string srcRouterKey=pMap.getRouterNameByMappingNo(src)+"/3";
        string destRouterKey=pMap.getRouterNameByMappingNo(dest)+"/3";

        double srcRadius=(pnlsr.getLsdb().getCorLsa(srcRouterKey).first).getCorRadius();
        double srcTheta=(pnlsr.getLsdb().getCorLsa(srcRouterKey).first).getCorTheta();

        double destRadius=(pnlsr.getLsdb().getCorLsa(
                               destRouterKey).first).getCorRadius();
        double destTheta=(pnlsr.getLsdb().getCorLsa(destRouterKey).first).getCorTheta();


        double diffTheta = fabs (srcTheta - destTheta);

        if (diffTheta > MATH_PI)
        {
            diffTheta = 2 * MATH_PI - diffTheta;
        }

        if ( srcRadius != -1 && destRadius != -1 )
        {
            if (diffTheta == 0)
                distance = fabs (srcRadius - destRadius);
            else
                distance = acosh((cosh(srcRadius)*cosh(destRadius))-
                                 (sinh(srcRadius)*sinh(destRadius)*cos(diffTheta)));
        }
        else
        {
            distance = -1;
        }

        return distance;
    }

    void
    HypRoutingTableCalculator::allocateLinkFaces()
    {
        linkFaces=new int[vNoLink];
    }

    void
    HypRoutingTableCalculator::allocateDistanceToNeighbor()
    {
        distanceToNeighbor=new double[vNoLink];
    }

    void
    HypRoutingTableCalculator::allocateDistFromNbrToDest()
    {
        distFromNbrToDest=new double[vNoLink];
    }

    void
    HypRoutingTableCalculator::freeLinkFaces()
    {
        delete [] linkFaces;
    }

    void
    HypRoutingTableCalculator::freeDistanceToNeighbor()
    {
        delete [] distanceToNeighbor;
    }

    void
    HypRoutingTableCalculator::freeDistFromNbrToDest()
    {
        delete [] distFromNbrToDest;
    }

}//namespace nlsr
