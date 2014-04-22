#ifndef NLSR_ADL_HPP
#define NLSR_ADL_HPP

#include <ndn-cpp-dev/face.hpp>
#include "adjacent.hpp"
#include <list>

namespace nlsr {
class Nlsr;

class Adl
{

public:
  Adl();
  ~Adl();

  int
  insert(Adjacent& adj);

  int
  updateAdjacentStatus(std::string adjName, int s);

  int
  updateAdjacentLinkCost(std::string adjName, double lc);

  std::list<Adjacent>&
  getAdjList();

  bool
  isNeighbor(std::string adjName);

  void
  incrementTimedOutInterestCount(std::string& neighbor);

  int
  getTimedOutInterestCount(std::string& neighbor);

  int
  getStatusOfNeighbor(std::string& neighbor);

  void
  setStatusOfNeighbor(std::string& neighbor, int status);

  void
  setTimedOutInterestCount(std::string& neighbor, int count);

  void
  addAdjacentsFromAdl(Adl& adl);

  bool
  isAdjLsaBuildable(Nlsr& pnlsr);

  int
  getNumOfActiveNeighbor();

  Adjacent
  getAdjacent(std::string adjName);

  bool
  isEqual(Adl& adl);

  int
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
  printAdl();

private:
  std::list<Adjacent>::iterator
  find(std::string adjName);

private:
  std::list<Adjacent> m_adjList;
};

} //namespace nlsr
#endif //NLSR_ADL_HPP
