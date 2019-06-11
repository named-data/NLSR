NLSR version 0.5.1
++++++++++++++++++

**Changes**

- Set best-route strategy for certificate prefixes (:issue:`4881`)
- Use ndn-cxx InMemoryStorage for LsaSegmentStorage (:issue:`4788`)
- Switch from ndn::EventId to ndn::scheduler::EventId (:issue:`4883`)
- Add FreshnessPeriod where MustBeFresh is used (:issue:`4913`)

**Bug Fixes**

- Fix canonization by resetting io service after each neighbor (:issue:`4872`)
- Disable network interest in while fetching NLSR certificate (:issue:`4879`)

