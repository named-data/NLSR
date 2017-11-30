NLSR version 0.3.2 (changes since version 0.3.1)
++++++++++++++++++++++++++++++++++++++++++++++++

Release date: March 15, 2017

**Changes**

- NLSR constructor now accept a KeyChain argument. (:issue:`3742`)

- Fix security rule for /localhop LSA prefix (:issue:`3899`)

- Use ndn::nfd::ROUTE_ORIGIN_NLSR instead of a magic number. (:issue:`3903`)

- Handle "Face Already Exists" error (error code 409) in NFD. (:issue:`3976`)

- Enable NLSR to register localhost prefix with dispatcher before top-level prefix. (:issue:`3938`)

- Don't use ndn aliases provided by ndn-cxx/common.hpp. (:issue:`3983`)

- Remove use of deprecated code. (:issue:`3988`)