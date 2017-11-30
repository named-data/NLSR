NLSR version 0.2.1 (changes since version 0.2.0)
++++++++++++++++++++++++++++++++++++++++++++++++

Release date: June 30, 2015

**Code changes**:

- Using a configuration file without ``prefix-update-validator`` disables runtime advertise/withdraw instead of exiting (:issue:`2814`)

**Bug fixes**:

- Prevent incomplete removal of NPT entry (:issue:`2785`)

- Correct bidirectional links with differing costs (:issue:`2801`)

- Fix non-incrementing Adjacency number in AdjLsa::writeLog (:issue:`2594`)

- Remove FaceDestroyEvent test's reliance on nlsrSeqNo.txt (:issue:`2881`)

- Test ACTIVE/INACTIVE Adjacencies in LSA construction (:issue:`2735`)