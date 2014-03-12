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
      : origRouter()
      , lsSeqNo()
      , lifeTime()
      , lsaExpiringEventId()
    {
    }


    void setLsType(uint8_t lst)
    {
      lsType=lst;
    }

    uint8_t getLsType()
    {
      return lsType;
    }

    void setLsSeqNo(uint32_t lsn)
    {
      lsSeqNo=lsn;
    }

    uint32_t getLsSeqNo()
    {
      return lsSeqNo;
    }

    string& getOrigRouter()
    {
      return origRouter;
    }

    void setOrigRouter(string& org)
    {
      origRouter=org;
    }

    uint32_t getLifeTime()
    {
      return lifeTime;
    }

    void setLifeTime(uint32_t lt)
    {
      lifeTime=lt;
    }

    void setLsaExpiringEventId(ndn::EventId leei)
    {
      lsaExpiringEventId=leei;
    }

    ndn::EventId getLsaExpiringEventId()
    {
      return lsaExpiringEventId;
    }


  protected:
    string origRouter;
    uint8_t lsType;
    uint32_t lsSeqNo;
    uint32_t lifeTime;
    ndn::EventId lsaExpiringEventId;
  };

  class NameLsa:public Lsa
  {
  public:
    NameLsa()
      : Lsa()
      , npl()
    {
      setLsType(1);
    }

    NameLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt, Npl npl);

    Npl& getNpl()
    {
      return npl;
    }

    void addNameToLsa(string& name)
    {
      npl.insertIntoNpl(name);
    }

    void removeNameFromLsa(string& name)
    {
      npl.removeFromNpl(name);
    }

    string getNameLsaKey();

    string getNameLsaData();
    bool initNameLsaFromContent(string content);

  private:
    Npl npl;

  };

  std::ostream&
  operator<<(std::ostream& os, NameLsa& nLsa);

  class AdjLsa: public Lsa
  {
  public:
    AdjLsa()
      : Lsa()
      , adl()
    {
      setLsType(2);
    }

    AdjLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt,
           uint32_t nl ,Adl padl);
    Adl& getAdl()
    {
      return adl;
    }

    void addAdjacentToLsa(Adjacent adj)
    {
      adl.insert(adj);
    }
    string getAdjLsaKey();
    string getAdjLsaData();
    bool initAdjLsaFromContent(string content);
    uint32_t getNoLink()
    {
      return noLink;
    }

    bool isLsaContentEqual(AdjLsa& alsa);
    void addNptEntriesForAdjLsa(Nlsr& pnlsr);
    void removeNptEntriesForAdjLsa(Nlsr& pnlsr);

  private:
    uint32_t noLink;
    Adl adl;
  };

  std::ostream&
  operator<<(std::ostream& os, AdjLsa& aLsa);

  class CorLsa:public Lsa
  {
  public:
    CorLsa()
      : Lsa()
      , corRad(0)
      , corTheta(0)
    {
      setLsType(3);
    }

    CorLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt
           , double r, double theta);
    string getCorLsaKey();
    string getCorLsaData();
    bool initCorLsaFromContent(string content);
    double getCorRadius()
    {
      if ( corRad >= 0 )
      {
        return corRad;
      }
      else
      {
        return -1;
      }
    }

    void setCorRadius(double cr)
    {
      corRad=cr;
    }

    double getCorTheta()
    {
      return corTheta;
    }

    void setCorTheta(double ct)
    {
      corTheta=ct;
    }

    bool isLsaContentEqual(CorLsa& clsa);
  private:
    double corRad;
    double corTheta;

  };

  std::ostream&
  operator<<(std::ostream& os, CorLsa& cLsa);


}//namespace nlsr

#endif
