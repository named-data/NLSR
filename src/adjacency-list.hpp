#ifndef NLSR_ADJACENCY_LIST_HPP
#define NLSR_ADJACENCY_LIST_HPP

#include <list>
#include <boost/cstdint.hpp>
#include <ndn-cxx/common.hpp>

#include "adjacent.hpp"

namespace nlsr {
class Nlsr;

class AdjacencyList
{

public:
  AdjacencyList();
  ~AdjacencyList();

  int32_t
  insert(Adjacent& adjacent);

  int32_t
  updateAdjacentStatus(const std::string& adjName, int32_t s);

  int32_t
  updateAdjacentLinkCost(const std::string& adjName, double lc);

  std::list<Adjacent>&
  getAdjList();

  bool
  isNeighbor(const std::string& adjName);

  void
  incrementTimedOutInterestCount(const std::string& neighbor);

  int32_t
  getTimedOutInterestCount(const std::string& neighbor);

  uint32_t
  getStatusOfNeighbor(const std::string& neighbor);

  void
  setStatusOfNeighbor(const std::string& neighbor, int32_t status);

  void
  setTimedOutInterestCount(const std::string& neighbor, uint32_t count);

  void
  addAdjacents(AdjacencyList& adl);

  bool
  isAdjLsaBuildable(Nlsr& pnlsr);

  int32_t
  getNumOfActiveNeighbor();

  Adjacent
  getAdjacent(const std::string& adjName);

  bool
  operator==(AdjacencyList& adl);

  size_t
  getSize()
  {
    return m_adjList.size();
  }

  void
  reset()
  {
    if (m_adjList.size() > 0)
    {
      m_adjList.clear();
    }
  }

  void
  print();

private:
  std::list<Adjacent>::iterator
  find(std::string adjName);

private:
  std::list<Adjacent> m_adjList;
};

} //namespace nlsr
#endif //NLSR_ADJACENCY_LIST_HPP
