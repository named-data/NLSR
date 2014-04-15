#ifndef NLSR_LSA_HPP
#define NLSR_LSA_HPP

#include <ndn-cpp-dev/util/scheduler.hpp>
#include "nlsr_adjacent.hpp"
#include "nlsr_npl.hpp"
#include "nlsr_adl.hpp"

namespace nlsr
{

  using namespace std;
  using namespace ndn;

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


    void setLsType(uint8_t lst)
    {
      m_lsType=lst;
    }

    uint8_t getLsType()
    {
      return m_lsType;
    }

    void setLsSeqNo(uint32_t lsn)
    {
      m_lsSeqNo=lsn;
    }

    uint32_t getLsSeqNo()
    {
      return m_lsSeqNo;
    }

    string& getOrigRouter()
    {
      return m_origRouter;
    }

    void setOrigRouter(string& org)
    {
      m_origRouter=org;
    }

    uint32_t getLifeTime()
    {
      return m_lifeTime;
    }

    void setLifeTime(uint32_t lt)
    {
      m_lifeTime=lt;
    }

    void setExpiringEventId(ndn::EventId leei)
    {
      m_expiringEventId=leei;
    }

    ndn::EventId getExpiringEventId()
    {
      return m_expiringEventId;
    }

  protected:
    string m_origRouter;
    uint8_t m_lsType;
    uint32_t m_lsSeqNo;
    uint32_t m_lifeTime;
    ndn::EventId m_expiringEventId;
  };

  class NameLsa:public Lsa
  {
  public:
    NameLsa()
      : Lsa()
      , m_npl()
    {
      setLsType(1);
    }

    NameLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt, Npl npl);

    Npl& getNpl()
    {
      return m_npl;
    }

    void addName(string& name)
    {
      m_npl.insert(name);
    }

    void removeName(string& name)
    {
      m_npl.remove(name);
    }

    string getKey();

    string getData();
    bool initializeFromContent(string content);
    void writeLog();

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

    AdjLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt,
           uint32_t nl ,Adl padl);
    Adl& getAdl()
    {
      return m_adl;
    }

    void addAdjacent(Adjacent adj)
    {
      m_adl.insert(adj);
    }
    string getKey();
    string getData();
    bool initializeFromContent(string content);
    uint32_t getNoLink()
    {
      return m_noLink;
    }

    bool isEqual(AdjLsa& alsa);
    void addNptEntries(Nlsr& pnlsr);
    void removeNptEntries(Nlsr& pnlsr);

  private:
    uint32_t m_noLink;
    Adl m_adl;
  };

  std::ostream&
  operator<<(std::ostream& os, AdjLsa& aLsa);

  class CorLsa:public Lsa
  {
  public:
    CorLsa()
      : Lsa()
      , m_corRad(0)
      , m_corTheta(0)
    {
      setLsType(3);
    }

    CorLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt
           , double r, double theta);
    string getKey();
    string getData();
    bool initializeFromContent(string content);
    double getCorRadius()
    {
      if ( m_corRad >= 0 )
      {
        return m_corRad;
      }
      else
      {
        return -1;
      }
    }

    void setCorRadius(double cr)
    {
      m_corRad=cr;
    }

    double getCorTheta()
    {
      return m_corTheta;
    }

    void setCorTheta(double ct)
    {
      m_corTheta=ct;
    }

    bool isEqual(CorLsa& clsa);
  private:
    double m_corRad;
    double m_corTheta;

  };

  std::ostream&
  operator<<(std::ostream& os, CorLsa& cLsa);


}//namespace nlsr

#endif
