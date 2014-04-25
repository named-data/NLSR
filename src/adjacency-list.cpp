#include <iostream>
#include <algorithm>

#include "adjacency-list.hpp"
#include "adjacent.hpp"
#include "nlsr.hpp"


namespace nlsr {

AdjacencyList::AdjacencyList()
{
}

AdjacencyList::~AdjacencyList()
{
}

static bool
adjacent_compare(Adjacent& adj1, Adjacent& adj2)
{
  return adj1.getName() == adj2.getName();
}

int
AdjacencyList::insert(Adjacent& adj)
{
  std::list<Adjacent>::iterator it = find(adj.getName());
  if (it != m_adjList.end())
  {
    return -1;
  }
  m_adjList.push_back(adj);
  return 0;
}

void
AdjacencyList::addAdjacentsFromAdl(AdjacencyList& adl)
{
  for (std::list<Adjacent>::iterator it = adl.getAdjList().begin();
       it != adl.getAdjList().end(); ++it)
  {
    insert((*it));
  }
}

int
AdjacencyList::updateAdjacentStatus(const string& adjName, int s)
{
  std::list<Adjacent>::iterator it = find(adjName);
  if (it == m_adjList.end())
  {
    return -1;
  }
  (*it).setStatus(s);
  return 0;
}

Adjacent
AdjacencyList::getAdjacent(const string& adjName)
{
  Adjacent adj(adjName);
  std::list<Adjacent>::iterator it = find(adjName);
  if (it != m_adjList.end())
  {
    return (*it);
  }
  return adj;
}


bool
AdjacencyList::isEqual(AdjacencyList& adl)
{
  if (getSize() != adl.getSize())
  {
    return false;
  }
  m_adjList.sort(adjacent_compare);
  adl.getAdjList().sort(adjacent_compare);
  int equalAdjCount = 0;
  std::list<Adjacent> adjList2 = adl.getAdjList();
  std::list<Adjacent>::iterator it1;
  std::list<Adjacent>::iterator it2;
  for (it1 = m_adjList.begin(), it2 = adjList2.begin();
       it1 != m_adjList.end(); it1++, it2++)
  {
    if (!(*it1).isEqual((*it2)))
    {
      break;
    }
    equalAdjCount++;
  }
  return equalAdjCount == getSize();
}


int
AdjacencyList::updateAdjacentLinkCost(const string& adjName, double lc)
{
  std::list<Adjacent>::iterator it = find(adjName);
  if (it == m_adjList.end())
  {
    return -1;
  }
  (*it).setLinkCost(lc);
  return 0;
}

bool
AdjacencyList::isNeighbor(const string& adjName)
{
  std::list<Adjacent>::iterator it = find(adjName);
  if (it == m_adjList.end())
  {
    return false;
  }
  return true;
}

void
AdjacencyList::incrementTimedOutInterestCount(const string& neighbor)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it == m_adjList.end())
  {
    return ;
  }
  (*it).setInterestTimedOutNo((*it).getInterestTimedOutNo() + 1);
}

void
AdjacencyList::setTimedOutInterestCount(const string& neighbor, int count)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it != m_adjList.end())
  {
    (*it).setInterestTimedOutNo(count);
  }
}

int
AdjacencyList::getTimedOutInterestCount(const string& neighbor)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it == m_adjList.end())
  {
    return -1;
  }
  return (*it).getInterestTimedOutNo();
}

int
AdjacencyList::getStatusOfNeighbor(const string& neighbor)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it == m_adjList.end())
  {
    return -1;
  }
  return (*it).getStatus();
}

void
AdjacencyList::setStatusOfNeighbor(const string& neighbor, int status)
{
  std::list<Adjacent>::iterator it = find(neighbor);
  if (it != m_adjList.end())
  {
    (*it).setStatus(status);
  }
}

std::list<Adjacent>&
AdjacencyList::getAdjList()
{
  return m_adjList;
}

bool
AdjacencyList::isAdjLsaBuildable(Nlsr& pnlsr)
{
  uint32_t nbrCount = 0;
  for (std::list<Adjacent>::iterator it = m_adjList.begin();
       it != m_adjList.end() ; it++)
  {
    if (((*it).getStatus() == 1))
    {
      nbrCount++;
    }
    else
    {
      if ((*it).getInterestTimedOutNo() >=
          pnlsr.getConfParameter().getInterestRetryNumber())
      {
        nbrCount++;
      }
    }
  }
  if (nbrCount == m_adjList.size())
  {
    return true;
  }
  return false;
}

int
AdjacencyList::getNumOfActiveNeighbor()
{
  int actNbrCount = 0;
  for (std::list<Adjacent>::iterator it = m_adjList.begin();
       it != m_adjList.end(); it++)
  {
    if (((*it).getStatus() == 1))
    {
      actNbrCount++;
    }
  }
  return actNbrCount;
}

std::list<Adjacent>::iterator
AdjacencyList::find(std::string adjName)
{
  Adjacent adj(adjName);
  std::list<Adjacent>::iterator it = std::find_if(m_adjList.begin(),
                                                  m_adjList.end(),
                                                  bind(&adjacent_compare, _1, adj));
  return it;
}

// used for debugging purpose
void
AdjacencyList::print()
{
  for (std::list<Adjacent>::iterator it = m_adjList.begin();
       it != m_adjList.end(); it++)
  {
    cout << (*it) << endl;
  }
}

} //namespace nlsr
