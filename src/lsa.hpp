#ifndef NLSR_LSA_HPP
#define NLSR_LSA_HPP

#include <ndn-cpp-dev/util/scheduler.hpp>
#include "adjacent.hpp"
#include "npl.hpp"
#include "adl.hpp"

namespace nlsr {
class Lsa
{
public:
  Lsa()
    : m_origRouter()
    , m_lsSeqNo()
    , m_lifeTime()
    , m_expiringEventId()
  {
  }


  void
  setLsType(uint8_t lst)
  {
    m_lsType = lst;
  }

  uint8_t
  getLsType() const
  {
    return m_lsType;
  }

  void
  setLsSeqNo(uint32_t lsn)
  {
    m_lsSeqNo = lsn;
  }

  uint32_t
  getLsSeqNo() const
  {
    return m_lsSeqNo;
  }

  std::string
  getOrigRouter() const
  {
    return m_origRouter;
  }

  void
  setOrigRouter(const std::string& org)
  {
    m_origRouter = org;
  }

  uint32_t
  getLifeTime() const
  {
    return m_lifeTime;
  }

  void
  setLifeTime(uint32_t lt)
  {
    m_lifeTime = lt;
  }

  void
  setExpiringEventId(const ndn::EventId leei)
  {
    m_expiringEventId = leei;
  }

  ndn::EventId
  getExpiringEventId() const
  {
    return m_expiringEventId;
  }

protected:
  std::string m_origRouter;
  uint8_t m_lsType;
  uint32_t m_lsSeqNo;
  uint32_t m_lifeTime;
  ndn::EventId m_expiringEventId;
};

class NameLsa: public Lsa
{
public:
  NameLsa()
    : Lsa()
    , m_npl()
  {
    setLsType(1);
  }

  NameLsa(std::string origR, uint8_t lst, uint32_t lsn, uint32_t lt, Npl npl);

  Npl&
  getNpl()
  {
    return m_npl;
  }

  void
  addName(std::string& name)
  {
    m_npl.insert(name);
  }

  void
  removeName(std::string& name)
  {
    m_npl.remove(name);
  }

  std::string
  getKey();

  std::string
  getData();

  bool
  initializeFromContent(std::string content);

  void
  writeLog();

private:
  Npl m_npl;

};

std::ostream&
operator<<(std::ostream& os, NameLsa& nLsa);

class AdjLsa: public Lsa
{
public:
  AdjLsa()
    : Lsa()
    , m_adl()
  {
    setLsType(2);
  }

  AdjLsa(std::string origR, uint8_t lst, uint32_t lsn, uint32_t lt,
         uint32_t nl , Adl padl);

  Adl&
  getAdl()
  {
    return m_adl;
  }

  void
  addAdjacent(Adjacent adj)
  {
    m_adl.insert(adj);
  }

  std::string
  getKey();

  std::string
  getData();

  bool
  initializeFromContent(std::string content);

  uint32_t
  getNoLink()
  {
    return m_noLink;
  }

  bool
  isEqual(AdjLsa& alsa);

  void
  addNptEntries(Nlsr& pnlsr);

  void
  removeNptEntries(Nlsr& pnlsr);

private:
  uint32_t m_noLink;
  Adl m_adl;
};

std::ostream&
operator<<(std::ostream& os, AdjLsa& aLsa);

class CorLsa: public Lsa
{
public:
  CorLsa()
    : Lsa()
    , m_corRad(0)
    , m_corTheta(0)
  {
    setLsType(3);
  }

  CorLsa(std::string origR, uint8_t lst, uint32_t lsn, uint32_t lt
         , double r, double theta);

  std::string
  getKey();

  std::string
  getData();

  bool
  initializeFromContent(std::string content);

  double
  getCorRadius()
  {
    if (m_corRad >= 0)
    {
      return m_corRad;
    }
    else
    {
      return -1;
    }
  }

  void
  setCorRadius(double cr)
  {
    m_corRad = cr;
  }

  double
  getCorTheta()
  {
    return m_corTheta;
  }

  void
  setCorTheta(double ct)
  {
    m_corTheta = ct;
  }

  bool
  isEqual(CorLsa& clsa);

private:
  double m_corRad;
  double m_corTheta;

};

std::ostream&
operator<<(std::ostream& os, CorLsa& cLsa);


}//namespace nlsr

#endif //NLSR_LSA_HPP
