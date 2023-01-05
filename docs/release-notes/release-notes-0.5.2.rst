NLSR version 0.5.2
++++++++++++++++++

**Breaking Changes**

The following breaking changes make this release incompatible with the previous version.

- Use PSync version 0.2.0, sync data compression in full sync is enabled by default (:issue:`5061`, :issue:`4917`)
- Use the floating point encoding/decoding functions for hyperbolic coordinates (:issue:`4615`)

**New Features**

- Handle zero cost links in Link State Routing (:issue:`4978`)

**Improvements and Bug Fixes**

- Remove support for old combined seq no file (:issue:`4143`)
- Sync common test infrastructure with ndn-cxx
- Refactor Certificate Storage class (:issue:`5075`)
- Avoid crashing NLSR if identity cannot be created
- Change init order to register sync routes after hello data has been verified (:issue:`5009`)
- Fix empty file creation if conf file is same as dynamic (:issue:`5071`)
- On face event notification, only proceed if face id was zero or updated (:issue:`4394`)
