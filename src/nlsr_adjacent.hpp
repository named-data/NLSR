#ifndef ADJACENT_HPP
#define ADJACENT_HPP

using namespace std;

class Adjacent{

	public:
		Adjacent()
			:adjacentName()
			,connectingFace(0)
			,linkCost(10.0)
			,status(0)
			,interestTimedOutNo(0)
		{
		}

		Adjacent(const string& an)
			:connectingFace(0)
			,linkCost(0.0)
			,status(0)
			,interestTimedOutNo(0)
		{
			adjacentName=an;
		}

		Adjacent(const string& an, int cf, double lc, int s, int iton);	

		string getAdjacentName(){
			return adjacentName;
		}

		void setAdjacentName(const string& an){
			adjacentName=an;
		}

		int getConnectingFace(){
			return connectingFace;
		}
		 
		void setConnectingFace(int cf){
			connectingFace=cf;
		}

		double getLinkCost(){
			return linkCost;
		}

		void setLinkCost(double lc){
			linkCost=lc;
		}

		int getStatus(){
			return status;
		}

		void setStatus(int s){
			status=s;
		}

		int getInterestTimedOutNo(){
			return interestTimedOutNo;
		}

		void setInterestTimedOutNo(int iton){
			interestTimedOutNo=iton;
		}

		bool isAdjacentEqual(Adjacent& adj);
	private:
		string adjacentName;
		int connectingFace;
		double linkCost;
		int status;
		int interestTimedOutNo;
};

std::ostream&
operator << (std::ostream &os, Adjacent &adj);

#endif
