NLSR version 0.5.0
++++++++++++++++++

**New Features**

- Set CanBePrefix to false on Hello Interest (:issue:`4581`)
- Optionally write prefixes to a copy of config file via nlsrc advertise (:issue:`4303`)
- Use SegmentPublisher utility from PSync to publish LSA data (:issue:`4760`)
- Add PSync as a dependency and switch it as default in conf file. (:issue:`4082`)

**Changes**

- Decouple classes from Nlsr object for easier unit testing (:issue:`1952`)
- Set sequence number file in SequencingManager constructor. (:issue:`4288`)
- Read conf file in test prefix update processor instead of static string (:issue:`3960`)
- Canonize FaceUri in conf file processor (:issue:`4817`)
- Remove redundant function "calculateHypDryRoutingTable" (:issue:`4811`)
- In dry run mode, print HR and LS routing table in nlsrc separately (:issue:`4452`)
- Fix ASan memory leaks in unit tests (:issue:`4682`)
- Systemd service files dependency corrections for Ubuntu 18.04 (:issue:`4594`)

**Bug Fixes**

- Fix processing of interest w/ segments for segmented LSA data (:issue:`4784`)
- Fib: don't refresh neighbor router's FIB entry (:issue:`4799`)
- Disable Interest filter loopback where ever not needed (:issue:`4768`)
- Avoid crash when ndn-cxx is compiled with tests by setting canBePrefix (:issue:`4691`)
- Make sure NLSR is not using any deprecated API (:issue:`4679`)
- Fix crash in equals operator of adjacency-list (:issue:`4705`)
- Fix error handling and normalize exit codes

**Miscellaneous**

- Improving documentation for NLSR
- A beginners' guide to installing and testing on Fedora
