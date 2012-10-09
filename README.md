NLSR0.0
=======

Named-data Link State Routing ( NLSR ):
  NLSR is routing protocol caluclate routing table dynamically and manipulate 
ccnd forwarding information base (FIB) based on routing table.

1. Download:
	Check out the code from the github by following
git://github.com/NDN-Routing/NLSR0.0.git

2. Installation:
	For installation of NLSR you must need to have ccnd installed. Go to NLSR0.0.
Then make it and install by following

$make
$sudo make install

3. Configuring NLSR:
	For the explanation of configuring NLSR, we will consider following network topology.


				---------
				|	|
				|	|  /ndn/memphis.edu/castor
				---------
			face9	/       \ face11
			       /         \
			      /           \
			     /             \
		       face7/               \ face10
		   ---------		   ---------			---------
		   |	   |face8   face11 |	   |face13	face7	|	|
		   |	   |---------------|	   |--------------------|	|
		   ---------		   ---------			---------
	/ndn/memphis.edu/pollux		/ndn/memphis.edu/mira		/ndn/memphis.edu/sirius
		

3.1 Creating faces for neighbors
	For configuring of NLSR first system admin first need to create faces for the neighbors.
For router having router-name /ndn/memphis.edu/castor, system admin need to create two faces for
neighbors routers /ndn/memphis.edu/pollux and /ndn/memphis.edu/mira. By ccndc utility one can
create face as below

$ccndc add /ndn/memphis.edu/pollux udp pollux.cs.memphis.edu
$ccndc add /ndn/memphis.edu/mira udp mira.cs.memphis.edu

Let us assume that that face id for /ndn/memphis.edu/pollux is 9 and /ndn/memphis.edu/mira is 11.

3.2 Configuration Commands
3.2.1 router-name /name/prefix/of/router
	router-name commands assigns name of the router. Router-name is unique in the network and
maintained by name management. This is mandatory configuration command.

3.2.2 ccnneighbor /name/prefix/of/neighbor/router faceX.
	By this command router configures neighboring router. X is integer and must be precedeed 
by "face". faceX is the connecting face for that neighbors.

3.2.3 ccnname /name/prefix/to/be/advertised
	With this configuration command router advertise name prefix

3.2.4 lsdb-synch-interval secs
	its the time interval after which neighbors will synchronize Link State Database. Default
value is 300.

3.2.5 interest-retry tries
	its number of tries router will try if interest is timed out. Defualt value is 3.

3.2.6 interest-resend-time secs
	its the amount of time after which interest for a content will timed out. Defualt value
is 15.

3.2.7 lsa-refresh-time secs
	its the amount of time after which router will regenerate its own LSDB and regenerate
every LSA. Default value is 1800.

3.2.8 router-dead-interval secs
	its the amounnt of time after which a router will consider its neighbor dead if it does
not hear anything from the neighbor. Default value 3600.

3.2.9 multi-path-face-num n
	with this command one activate to do multi-path routing. If n is greater than 1 then
multi-path is triggered on and n faces will be added in ccnd fib after routing table calculation.
Default value is 0 and then NLSR perform single path routing.

3.2.10 logdir path/to/log/dir
	By this command one can configure where NLSR will write logfiles. By default NLSR will
write log file to /home/nlsrLog/ directory.

3.3 Sample configuration file
	Let assume that router /ndn/memphis.edu/castor wants to advertise two names 
/ndn/memphis.edu/cslab/netlab and /ndn/memphis.edu/cslab/wisemanet. Configuration file for 
/ndn/memphis.edu/castor will look like below 

#-----Configuration file starts here

router-name /ndn/memphis.edu/netlab/macbook/
ccnneighbor /ndn/memphis.edu/dunhall/castor face9
ccnneighbor /ndn/memphis.edu/netlab/pollux face11
ccnname /ndn/memphis.edu/patterson
ccnname /ndn/memphis.edu/houston/

lsdb-synch-interval 350
interest-retry 3 
interest-resend-time 10
lsa-refresh-time 1800
router-dead-interval 3600
multi-path-face-num 2 
logdir /home/NLSR2.0 

#-----Configuration file ends here
