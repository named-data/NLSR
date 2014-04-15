#ifndef ADJACENT_HPP
#define ADJACENT_HPP

namespace nlsr
{

  using namespace std;

  class Adjacent
  {

  public:
    Adjacent()
      :m_name()
      ,m_connectingFace(0)
      ,m_linkCost(10.0)
      ,m_status(0)
      ,m_interestTimedOutNo(0)
    {
    }

    Adjacent(const string& an)
      :m_connectingFace(0)
      ,m_linkCost(0.0)
      ,m_status(0)
      ,m_interestTimedOutNo(0)
    {
      m_name=an;
    }

    Adjacent(const string& an, int cf, double lc, int s, int iton);

    string getName()
    {
      return m_name;
    }

    void setName(const string& an)
    {
      m_name=an;
    }

    int getConnectingFace()
    {
      return m_connectingFace;
    }

    void setConnectingFace(int cf)
    {
      m_connectingFace=cf;
    }

    double getLinkCost()
    {
      return m_linkCost;
    }

    void setLinkCost(double lc)
    {
      m_linkCost=lc;
    }

    int getStatus()
    {
      return m_status;
    }

    void setStatus(int s)
    {
      m_status=s;
    }

    int getInterestTimedOutNo()
    {
      return m_interestTimedOutNo;
    }

    void setInterestTimedOutNo(int iton)
    {
      m_interestTimedOutNo=iton;
    }

    bool isEqual(Adjacent& adj);
  private:
    string m_name;
    int m_connectingFace;
    double m_linkCost;
    int m_status;
    int m_interestTimedOutNo;
  };

  std::ostream&
  operator << (std::ostream &os, Adjacent &adj);

} // namespace nlsr

#endif
