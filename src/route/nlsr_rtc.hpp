#ifndef NLSR_RTC_HPP
#define NLSR_RTC_HPP

#include <list>
#include <iostream>

namespace nlsr
{

    class Map;
    class RoutingTable;
    class Nlsr;


    using namespace std;

    class RoutingTableCalculator
    {
    public:
        RoutingTableCalculator()
        {
        }
        RoutingTableCalculator(int rn)
        {
            numOfRouter=rn;
        }
    protected:
        void allocateAdjMatrix();
        void initMatrix();
        void makeAdjMatrix(Nlsr& pnlsr,Map pMap);
        void printAdjMatrix();
        int getNumOfLinkfromAdjMatrix(int sRouter);
        void freeAdjMatrix();
        void adjustAdMatrix(int source, int link, double linkCost);
        void getLinksFromAdjMatrix(int *links, double *linkCosts, int source);

        void allocateLinks();
        void allocateLinkCosts();
        void freeLinks();
        void freeLinksCosts();

        void setNoLink(int nl)
        {
            vNoLink=nl;
        }

    protected:
        double ** adjMatrix;
        int numOfRouter;

        int vNoLink;
        int *links;
        double *linkCosts;
    };

    class LinkStateRoutingTableCalculator: public RoutingTableCalculator
    {
    public:
        LinkStateRoutingTableCalculator(int rn)
            : EMPTY_PARENT(-12345)
            , INF_DISTANCE(2147483647)
            , NO_MAPPING_NUM(-1)
            , NO_NEXT_HOP(-12345)
        {
            numOfRouter=rn;
        }


        void calculatePath(Map& pMap, RoutingTable& rt, Nlsr& pnlsr);


    private:
        void doDijkstraPathCalculation(int sourceRouter);
        void sortQueueByDistance(int *Q, double *dist,int start,int element);
        int isNotExplored(int *Q, int u,int start, int element);
        void printAllLsPath(int sourceRouter);
        void printLsPath(int destRouter);
        void addAllLsNextHopsToRoutingTable(Nlsr& pnlsr, RoutingTable& rt,
                                            Map& pMap,int sourceRouter);
        int getLsNextHop(int dest, int source);

        void allocateParent();
        void allocateDistance();
        void freeParent();
        void freeDistance();




    private:
        int *parent;
        double *distance;


        const int EMPTY_PARENT;
        const double INF_DISTANCE;
        const int NO_MAPPING_NUM;
        const int NO_NEXT_HOP;

    };

    class HypRoutingTableCalculator: public RoutingTableCalculator
    {
    public:
        HypRoutingTableCalculator(int rn)
            :  MATH_PI(3.141592654)
        {
            numOfRouter=rn;
            isDryRun=0;
        }
        HypRoutingTableCalculator(int rn, int idr)
            :  MATH_PI(3.141592654)
        {
            numOfRouter=rn;
            isDryRun=idr;
        }

        void calculatePath(Map& pMap, RoutingTable& rt, Nlsr& pnlsr);

    private:
        void allocateLinkFaces();
        void allocateDistanceToNeighbor();
        void allocateDistFromNbrToDest();
        void freeLinkFaces();
        void freeDistanceToNeighbor();
        void freeDistFromNbrToDest();

        double getHyperbolicDistance(Nlsr& pnlsr,Map& pMap, int src, int dest);
        void addHypNextHopsToRoutingTable(Nlsr& pnlsr,Map& pMap,
                                          RoutingTable& rt, int noFaces,int dest);

    private:
        int isDryRun;

        int *linkFaces;
        double *distanceToNeighbor;
        double *distFromNbrToDest;

        const double MATH_PI;

    };

}//namespace nlsr

#endif
