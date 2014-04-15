#include<string>
#include<iostream>
#include<sstream>
#include<algorithm>
#include<cmath>
#include<limits>

#include "nlsr.hpp"
#include "nlsr_lsa.hpp"
#include "nlsr_npl.hpp"
#include "nlsr_adjacent.hpp"
#include "utility/nlsr_tokenizer.hpp"
#include "utility/nlsr_logger.hpp"

#define THIS_FILE "nlsr_lsa.cpp"

namespace nlsr
{

  using namespace std;


  string
  NameLsa::getKey()
  {
    string key;
    key=m_origRouter + "/" + boost::lexical_cast<std::string>(1);
    return key;
  }

  NameLsa::NameLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt, Npl npl)
  {
    m_origRouter=origR;
    m_lsType=lst;
    m_lsSeqNo=lsn;
    m_lifeTime=lt;
    std::list<string> nl=npl.getNameList();
    for( std::list<string>::iterator it=nl.begin(); it != nl.end(); it++)
    {
      addName((*it));
    }
  }

  string
  NameLsa::getData()
  {
    string nameLsaData;
    nameLsaData=m_origRouter + "|" + boost::lexical_cast<std::string>(1) + "|"
                + boost::lexical_cast<std::string>(m_lsSeqNo) + "|"
                + boost::lexical_cast<std::string>(m_lifeTime);
    nameLsaData+="|";
    nameLsaData+=boost::lexical_cast<std::string>(m_npl.getSize());
    std::list<string> nl=m_npl.getNameList();
    for( std::list<string>::iterator it=nl.begin(); it != nl.end(); it++)
    {
      nameLsaData+="|";
      nameLsaData+=(*it);
    }
    return nameLsaData+"|";
  }

  bool
  NameLsa::initializeFromContent(string content)
  {
    uint32_t numName=0;
    nlsrTokenizer nt(content, "|");
    m_origRouter=nt.getNextToken();
    if(m_origRouter.empty())
    {
      return false;
    }
    try
    {
      m_lsType=boost::lexical_cast<uint8_t>(nt.getNextToken());
      m_lsSeqNo=boost::lexical_cast<uint32_t>(nt.getNextToken());
      m_lifeTime=boost::lexical_cast<uint32_t>(nt.getNextToken());
      numName=boost::lexical_cast<uint32_t>(nt.getNextToken());
    }
    catch(std::exception &e)
    {
      return false;
    }
    for(int i=0; i<numName; i++)
    {
      string name=nt.getNextToken();
      addName(name);
    }
    return true;
  }
  
  void
  NameLsa::writeLog()
  {
    src::logger lg;
    BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"Name-LSA";
    BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"  Origination Router: "
                 <<m_origRouter;
    BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"  LS Type: "<<m_lsType;
    BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"  LS Seq: "<<m_lsSeqNo;
    BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"  LS Lifetime: "<<m_lifeTime;
    BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"  LS Data: ";
    int i=1;
    std::list<string> nl=m_npl.getNameList();
    for( std::list<string>::iterator it=nl.begin(); it != nl.end(); it++)
    {
      BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"    Name "<<i<<": "<<(*it);
    }
    BOOST_LOG(lg)<<" "<<THIS_FILE<<" "<<__LINE__<<": "<<"name_lsa_end";
  }

  std::ostream&
  operator<<(std::ostream& os, NameLsa& nLsa)
  {
    os<<"Name Lsa: "<<endl;
    os<<"  Origination Router: "<<nLsa.getOrigRouter()<<endl;
    os<<"  Ls Type: "<<(unsigned short)nLsa.getLsType()<<endl;
    os<<"  Ls Seq No: "<<(unsigned int)nLsa.getLsSeqNo()<<endl;
    os<<"  Ls Lifetime: "<<(unsigned int)nLsa.getLifeTime()<<endl;
    os<<"  Names: "<<endl;
    int i=1;
    std::list<string> nl=nLsa.getNpl().getNameList();
    for( std::list<string>::iterator it=nl.begin(); it != nl.end(); it++)
    {
      os<<"    Name "<<i<<": "<<(*it)<<endl;
    }
    return os;
  }



  CorLsa::CorLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt
                 , double r, double theta)
  {
    m_origRouter=origR;
    m_lsType=lst;
    m_lsSeqNo=lsn;
    m_lifeTime=lt;
    m_corRad=r;
    m_corTheta=theta;
  }

  string
  CorLsa::getKey()
  {
    string key;
    key=m_origRouter + "/" + boost::lexical_cast<std::string>(3);
    return key;
  }

  bool
  CorLsa::isEqual(CorLsa& clsa)
  {
    return (std::abs(m_corRad - clsa.getCorRadius()) <
            std::numeric_limits<double>::epsilon()) &&
           (std::abs(m_corTheta - clsa.getCorTheta()) <
            std::numeric_limits<double>::epsilon());
  }

  string
  CorLsa::getData()
  {
    string corLsaData;
    corLsaData=m_origRouter + "|";
    corLsaData+=(boost::lexical_cast<std::string>(3) + "|");
    corLsaData+=(boost::lexical_cast<std::string>(m_lsSeqNo) + "|");
    corLsaData+=(boost::lexical_cast<std::string>(m_lifeTime) + "|");
    corLsaData+=(boost::lexical_cast<std::string>(m_corRad) + "|");
    corLsaData+=(boost::lexical_cast<std::string>(m_corTheta) + "|");
    return corLsaData;
  }

  bool
  CorLsa::initializeFromContent(string content)
  {
    nlsrTokenizer nt(content, "|");
    m_origRouter=nt.getNextToken();
    if(m_origRouter.empty())
    {
      return false;
    }
    try
    {
      m_lsType=boost::lexical_cast<uint8_t>(nt.getNextToken());
      m_lsSeqNo=boost::lexical_cast<uint32_t>(nt.getNextToken());
      m_lifeTime=boost::lexical_cast<uint32_t>(nt.getNextToken());
      m_corRad=boost::lexical_cast<double>(nt.getNextToken());
      m_corTheta=boost::lexical_cast<double>(nt.getNextToken());
    }
    catch(std::exception &e)
    {
      return false;
    }
    return true;
  }

  std::ostream&
  operator<<(std::ostream& os, CorLsa& cLsa)
  {
    os<<"Cor Lsa: "<<endl;
    os<<"  Origination Router: "<<cLsa.getOrigRouter()<<endl;
    os<<"  Ls Type: "<<(unsigned short)cLsa.getLsType()<<endl;
    os<<"  Ls Seq No: "<<(unsigned int)cLsa.getLsSeqNo()<<endl;
    os<<"  Ls Lifetime: "<<(unsigned int)cLsa.getLifeTime()<<endl;
    os<<"    Hyperbolic Radius: "<<cLsa.getCorRadius()<<endl;
    os<<"    Hyperbolic Theta: "<<cLsa.getCorTheta()<<endl;
    return os;
  }


  AdjLsa::AdjLsa(string origR, uint8_t lst, uint32_t lsn, uint32_t lt,
                 uint32_t nl ,Adl padl)
  {
    m_origRouter=origR;
    m_lsType=lst;
    m_lsSeqNo=lsn;
    m_lifeTime=lt;
    m_noLink=nl;
    std::list<Adjacent> al=padl.getAdjList();
    for( std::list<Adjacent>::iterator it=al.begin(); it != al.end(); it++)
    {
      if((*it).getStatus()==1)
      {
        addAdjacent((*it));
      }
    }
  }

  string
  AdjLsa::getKey()
  {
    string key;
    key=m_origRouter + "/" + boost::lexical_cast<std::string>(2);
    return key;
  }

  bool
  AdjLsa::isEqual(AdjLsa& alsa)
  {
    return m_adl.isEqual(alsa.getAdl());
  }


  string
  AdjLsa::getData()
  {
    string adjLsaData;
    adjLsaData=m_origRouter + "|" + boost::lexical_cast<std::string>(2) + "|"
               + boost::lexical_cast<std::string>(m_lsSeqNo) + "|"
               + boost::lexical_cast<std::string>(m_lifeTime);
    adjLsaData+="|";
    adjLsaData+=boost::lexical_cast<std::string>(m_adl.getSize());
    std::list<Adjacent> al=m_adl.getAdjList();
    for( std::list<Adjacent>::iterator it=al.begin(); it != al.end(); it++)
    {
      adjLsaData+="|";
      adjLsaData+=(*it).getName();
      adjLsaData+="|";
      adjLsaData+=boost::lexical_cast<std::string>((*it).getConnectingFace());
      adjLsaData+="|";
      adjLsaData+=boost::lexical_cast<std::string>((*it).getLinkCost());
    }
    return adjLsaData+"|";
  }

  bool
  AdjLsa::initializeFromContent(string content)
  {
    uint32_t numLink=0;
    nlsrTokenizer nt(content, "|");
    m_origRouter=nt.getNextToken();
    if(m_origRouter.empty())
    {
      return false;
    }
    try
    {
      m_lsType=boost::lexical_cast<uint8_t>(nt.getNextToken());
      m_lsSeqNo=boost::lexical_cast<uint32_t>(nt.getNextToken());
      m_lifeTime=boost::lexical_cast<uint32_t>(nt.getNextToken());
      numLink=boost::lexical_cast<uint32_t>(nt.getNextToken());
    }
    catch(std::exception &e)
    {
      return false;
    }
    for(int i=0; i< numLink; i++)
    {
      try
      {
        string adjName=nt.getNextToken();
        int connectingFace=boost::lexical_cast<int>(nt.getNextToken());
        double linkCost=boost::lexical_cast<double>(nt.getNextToken());
        Adjacent adjacent(adjName, connectingFace, linkCost, 0, 0);
        addAdjacent(adjacent);
      }
      catch( std::exception &e )
      {
        return false;
      }
    }
    return true;
  }


  void
  AdjLsa::addNptEntries(Nlsr& pnlsr)
  {
    if ( getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
    {
      pnlsr.getNpt().addNpteByDestName(getOrigRouter(), getOrigRouter(),pnlsr);
    }
  }


  void
  AdjLsa::removeNptEntries(Nlsr& pnlsr)
  {
    if ( getOrigRouter() !=pnlsr.getConfParameter().getRouterPrefix() )
    {
      pnlsr.getNpt().removeNpte(getOrigRouter(), getOrigRouter(),pnlsr);
    }
  }



  std::ostream&
  operator<<(std::ostream& os, AdjLsa& aLsa)
  {
    os<<"Adj Lsa: "<<endl;
    os<<"  Origination Router: "<<aLsa.getOrigRouter()<<endl;
    os<<"  Ls Type: "<<(unsigned short)aLsa.getLsType()<<endl;
    os<<"  Ls Seq No: "<<(unsigned int)aLsa.getLsSeqNo()<<endl;
    os<<"  Ls Lifetime: "<<(unsigned int)aLsa.getLifeTime()<<endl;
    os<<"  No Link: "<<(unsigned int)aLsa.getNoLink()<<endl;
    os<<"  Adjacents: "<<endl;
    int i=1;
    std::list<Adjacent> al=aLsa.getAdl().getAdjList();
    for( std::list<Adjacent>::iterator it=al.begin(); it != al.end(); it++)
    {
      os<<"    Adjacent "<<i<<": "<<endl;
      os<<"      Adjacent Name: "<<(*it).getName()<<endl;
      os<<"      Connecting Face: "<<(*it).getConnectingFace()<<endl;
      os<<"      Link Cost: "<<(*it).getLinkCost()<<endl;
    }
    return os;
  }

}//namespace nlsr
