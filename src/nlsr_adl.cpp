#include<iostream>
#include<algorithm>

#include "nlsr_adl.hpp"
#include "nlsr_adjacent.hpp"
#include "nlsr.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_adl.cpp"

namespace nlsr
{

  Adl::Adl()
  {
  }

  Adl::~Adl()
  {
  }

  static bool
  adjacent_compare(Adjacent& adj1, Adjacent& adj2)
  {
    return adj1.getName()==adj2.getName();
  }

  int
  Adl::insert(Adjacent& adj)
  {
    std::list<Adjacent >::iterator it = std::find_if( m_adjList.begin(),
                                        m_adjList.end(),
                                        bind(&adjacent_compare, _1, adj));
    if ( it != m_adjList.end() )
    {
      return -1;
    }
    m_adjList.push_back(adj);
    return 0;
  }

  void
  Adl::addAdjacentsFromAdl(Adl& adl)
  {
    for(std::list<Adjacent >::iterator it=adl.getAdjList().begin();
        it!=adl.getAdjList().end(); ++it)
    {
      insert((*it));
    }
  }

  int
  Adl::updateAdjacentStatus(string adjName, int s)
  {
    Adjacent adj(adjName);
    std::list<Adjacent >::iterator it = std::find_if( m_adjList.begin(),
                                        m_adjList.end(),
                                        bind(&adjacent_compare, _1, adj));
    if( it == m_adjList.end())
    {
      return -1;
    }
    (*it).setStatus(s);
    return 0;
  }

  Adjacent
  Adl::getAdjacent(string adjName)
  {
    Adjacent adj(adjName);
    std::list<Adjacent >::iterator it = std::find_if( m_adjList.begin(),
                                        m_adjList.end(),
                                        bind(&adjacent_compare, _1, adj));
    if( it != m_adjList.end())
    {
      return (*it);
    }
    return adj;
  }


  bool
  Adl::isEqual(Adl& adl)
  {
    if ( getSize() != adl.getSize() )
    {
      return false;
    }
    m_adjList.sort(adjacent_compare);
    adl.getAdjList().sort(adjacent_compare);
    int equalAdjCount=0;
    std::list< Adjacent > adjList2=adl.getAdjList();
    std::list<Adjacent>::iterator it1;
    std::list<Adjacent>::iterator it2;
    for(it1=m_adjList.begin() , it2=adjList2.begin() ;
        it1!=m_adjList.end(); it1++,it2++)
    {
      if ( !(*it1).isEqual((*it2)) )
      {
        break;
      }
      equalAdjCount++;
    }
    return equalAdjCount==getSize();
  }


  int
  Adl::updateAdjacentLinkCost(string adjName, double lc)
  {
    Adjacent adj(adjName);
    std::list<Adjacent >::iterator it = std::find_if( m_adjList.begin(),
                                        m_adjList.end(),
                                        bind(&adjacent_compare, _1, adj));
    if( it == m_adjList.end())
    {
      return -1;
    }
    (*it).setLinkCost(lc);
    return 0;
  }

  bool
  Adl::isNeighbor(string adjName)
  {
    Adjacent adj(adjName);
    std::list<Adjacent >::iterator it = std::find_if( m_adjList.begin(),
                                        m_adjList.end(),
                                        bind(&adjacent_compare, _1, adj));
    if( it == m_adjList.end())
    {
      return false;
    }
    return true;
  }

  void
  Adl::incrementTimedOutInterestCount(string& neighbor)
  {
    Adjacent adj(neighbor);
    std::list<Adjacent >::iterator it = std::find_if( m_adjList.begin(),
                                        m_adjList.end(),
                                        bind(&adjacent_compare, _1, adj));
    if( it == m_adjList.end())
    {
      return ;
    }
    (*it).setInterestTimedOutNo((*it).getInterestTimedOutNo()+1);
  }

  void
  Adl::setTimedOutInterestCount(string& neighbor, int count)
  {
    Adjacent adj(neighbor);
    std::list<Adjacent >::iterator it = std::find_if( m_adjList.begin(),
                                        m_adjList.end(),
                                        bind(&adjacent_compare, _1, adj));
    if( it != m_adjList.end())
    {
      (*it).setInterestTimedOutNo(count);
    }
  }

  int
  Adl::getTimedOutInterestCount(string& neighbor)
  {
    Adjacent adj(neighbor);
    std::list<Adjacent >::iterator it = std::find_if( m_adjList.begin(),
                                        m_adjList.end(),
                                        bind(&adjacent_compare, _1, adj));
    if( it == m_adjList.end())
    {
      return -1;
    }
    return (*it).getInterestTimedOutNo();
  }

  int
  Adl::getStatusOfNeighbor(string& neighbor)
  {
    Adjacent adj(neighbor);
    std::list<Adjacent >::iterator it = std::find_if( m_adjList.begin(),
                                        m_adjList.end(),
                                        bind(&adjacent_compare, _1, adj));
    if( it == m_adjList.end())
    {
      return -1;
    }
    return (*it).getStatus();
  }

  void
  Adl::setStatusOfNeighbor(string& neighbor, int status)
  {
    Adjacent adj(neighbor);
    std::list<Adjacent >::iterator it = std::find_if( m_adjList.begin(),
                                        m_adjList.end(),
                                        bind(&adjacent_compare, _1, adj));
    if( it != m_adjList.end())
    {
      (*it).setStatus(status);
    }
  }

  std::list<Adjacent>&
  Adl::getAdjList()
  {
    return m_adjList;
  }

  bool
  Adl::isAdjLsaBuildable(Nlsr& pnlsr)
  {
    int nbrCount=0;
    for( std::list<Adjacent>::iterator it=m_adjList.begin();
         it!= m_adjList.end() ; it++)
    {
      if ( ((*it).getStatus() == 1 ) )
      {
        nbrCount++;
      }
      else
      {
        if ( (*it).getInterestTimedOutNo() >=
             pnlsr.getConfParameter().getInterestRetryNumber())
        {
          nbrCount++;
        }
      }
    }
    if( nbrCount == m_adjList.size())
    {
      return true;
    }
    return false;
  }

  int
  Adl::getNumOfActiveNeighbor()
  {
    int actNbrCount=0;
    for( std::list<Adjacent>::iterator it=m_adjList.begin();
         it!= m_adjList.end() ; it++)
    {
      if ( ((*it).getStatus() == 1 ) )
      {
        actNbrCount++;
      }
    }
    return actNbrCount;
  }

// used for debugging purpose
  void
  Adl::printAdl()
  {
    for( std::list<Adjacent>::iterator it=m_adjList.begin(); it!= m_adjList.end() ;
         it++)
    {
      cout<< (*it) <<endl;
    }
  }

} //namespace nlsr
