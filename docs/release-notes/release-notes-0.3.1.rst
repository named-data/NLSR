NLSR version 0.3.1 (changes since version 0.3.0)
++++++++++++++++++++++++++++++++++++++++++++++++

Release date: January 22, 2017

**Changes**

- **breaking change** Sync uses /localhop component in prefix to reduce duplicate NACKs (:issue:`3627`)

- **breaking change** LSA uses /localhop component in prefix to reduce duplicate NACKs (:issue:`3899`)

**Code Changes**:

- Fib and FaceMap use std::map instead of std::list (:issue:`2781`)

- NamePrefixTableEntries store RoutingTableEntries using a pointer (:issue:`2863`)

- Namespace closing comments are now consistent with ndn-cxx guidelines (:issue:`3735`)

- Update copyright date across all files (:issue:`3736`, `3909`, `3923`)

- Improve formatting of FaceMap and FaceMapEntry (:issue:`3737`)

- Sort NextHops on NexthopList insertion (:issue:`2721`)

**Bug fixes**:

- HR now recalculates paths after status change of a neighbor (:issue:`3874`)

- NLSR no longer sets strategies multiple times (:issue:`3743`)

- NLSR should enable use of ndn-cxx dispatcher (:issue:`3930`)

- Fix typos in the sample configuration file (:issue:`3113`)