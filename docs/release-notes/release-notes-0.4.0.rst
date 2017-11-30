NLSR version 0.4.0
++++++++++++++++++

**New features**:

- **breaking change** Discover Faces from NFD (:issue:`2954`)

- **breaking change** Accommodate n-Dimensional HR coordinates (:issue:`3751`)

**Changes**:

- **breaking change** Make chronosync a dependency (:issue:`2400`)

- **breaking change** Use separate name prefix and sequence number
  for each LSA type (:issue:`3964`)

- **breaking change** Convert to ``v2::security`` and adapt to ndn-cxx changes (:issue:`3964`)

- Use unix timestamp as default (:issue:`4187`)

**Bug Fixes**:

- Add support for sources to NamePrefixList (:issue:`4177`)

- Fix incorrect error on face-dataset-fetch-interval valid values (:issue:`4211`)

- Fix prefix advertise runtime error (:issue:`4215`)

- Fix heap buffer overflow error (:issue:`4217`)

**Code Changes**:

- LSA getData() now uses ostream instead of manually building the string (:issue:`2346`)

- Refresh FIB entries independently of routing table calculation (:issue:`2778`)

- Change configuration parameter output level to info (:issue:`2850`)

- Updates to name prefixes are more efficient (:issue:`2864`)

- Implement statistics collector/recorder (:issue:`2956`)

- Implement ``NameLsa::isEqualContent`` (:issue:`2962`)

- Refactor ``Fib::removeOldNextHopsFromFibEntryAndNfd()`` for simplicity, readability. (:issue:`3820`)

- Improve logging throughout lsdb.cpp and others (:issue:`3934`)

- Enable dispatcher for LOCALHOST_PREFIX (:issue:`3938`)

- Don't use ndn aliases provided by ndn-cxx/common.hpp (:issue:`3983`)

- Canonize Adjacent FaceURIs on startup (:issue:`4063`)

- Refactor AdjacencyList to use iterators instead of bare ptrs (:issue:`4068`)

- Merge redundant functions (:issue:`4105`)

- Remove writeLog in NamePrefixList (:issue:`4131`)

- Check for self in ChronoSync updates (:issue:`4134`)

- Install nlsr.conf as sample in the system (:issue:`4197`)

- Enable AddressSanitizer and LeakSanitizer for unit tests (:issue:`4206`)

- Resolve all ASan errors in unit tests (:issue:`4230`)

- Simplified LSDB dataset interest handler. (:issue:`4235`)

- Made map more container-agnostic (:issue:`4239`)

- Improved routing map data structure. (:issue:`4240`)

- Increased independence of SyncLogicHandler (:issue:`4264`)

- Use unique_ptr for signal container (:issue:`4268`)

- Use network name in sync prefix to avoid sync leaks from other networks (:issue:`4101`)
