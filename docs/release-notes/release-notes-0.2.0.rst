NLSR version 0.2.0 (changes since version 0.1.0)
++++++++++++++++++++++++++++++++++++++++++++++++

Release date: April 30, 2015

**Build**:

- The code now requires C++11. The minimum supported gcc version is 4.6, as earlier versions do not have proper support for C++11 features. (:issue:`1930`)

- Require OpenSSL (:issue:`2375`)

**New features**:

- Add nlsrc command-line tool (:issue:`1834`)

- Advertise/Withdraw name prefixes at runtime (:issue:`1834`)

- Assign zero cost to hyperbolic direct neighbors (:issue:`1975`)

- Only hyperbolic costs are adjusted before registering (:issue:`1974`)

- Add ability to specify different logging levels (:issue:`1935`)

- Add LSDB dataset publishers (:issue:`2280`)

- Add LSDB dataset tlv abstractions (:issue:`2280`)

- Allow zero cost neighbors for hyperbolic routing table calculation (:issue:`1982`)

- Faces are now created with canonical Face URI (:issue:`1911`)

- Add configurable router event intervals (:issue:`2071`)

- Add router-dead-interval configuration setting (:issue:`1878`)

- Add log formatting specified by log4cxx configuration file (:issue:`1950`)

- Face URIs in configuration file are checked for validity on load (:issue:`2070`)

- Configuration variables with default values are now optional (:issue:`1949`)

- Recalculate routing table after face destroy event (:issue:`2635`)

- Add Travis-CI build script

**Code changes**:

- Change adjacency status from uint32_t to enum (:issue:`1946`)

- Remove scheduler from NLSR class (:issue:`1981`)

- Change hyperbolic cost adjustment factor from 100 to 1000 (:issue:`1974`)

- Remove use of outdated namespace ``ndn::Tlv`` (:issue:`2079`)

- Remove use of deprecated ``ndn::nfd::Controller(Face&)`` constructor (:issue:`2133`)

- Routing update publishing is abstracted to SyncLogicHandler (:issue:`2289`)

- Make ConfParameter variable types consistent with min and max values (:issue:`2084`)

- Replace LSA string literals with constants (:issue:`2310`)

- Subscribe to FaceMonitor notifications using connect (:issue:`2350`)

- Log exceptions thrown in event loop

- Change ``ndn::ptr_lib::shared_ptr`` to ``std::shared_ptr`` (:issue:`2110`)

- Avoid deprecated ``ndn::Name::set`` method (:issue:`2505`)

- Update waf to version 1.8.8-dev

- Add sorting tiebreaker to NexthopList (:issue:`2715`)

- Update Jenkins-CI build scripts

**Bug fixes**:

- Fix python3 build error (:issue:`1956`)

- Fix typos in nlsr.conf security section (:issue:`1985`)

- Stop SyncLogicHandler from fetching a router’s own LSAs (:issue:`2005`)

- Fix Name LSA comparison which could cause incorrect advertised names to be removed (:issue:`2029`)

- Fix issue where more faces than max-faces-per-prefix could be applied to a prefix (:issue:`2018`)

- Stop LSDB from retrieving and out of date LSA when a new one is detected (:issue:`1987`)

- Correct mistakes in documentation

- Correct bitmask used to copy Name LSA sequence number (:issue:`2323`)