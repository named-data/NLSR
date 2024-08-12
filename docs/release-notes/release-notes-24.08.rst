NLSR version 24.08
------------------

*Release date: August 12, 2024*

Notable changes
^^^^^^^^^^^^^^^

- We have moved over to the `CalVer <https://calver.org/>`__ versioning scheme to maintain
  consistency with NFD going forward: 24.08 is the successor to 0.7.0

- The minimum build requirements have been increased as follows:

  - GCC >= 9.3 or Clang >= 7.0 are strongly *recommended* on Linux; GCC 8.x is also known
    to work but is not officially supported
  - Xcode 13 or later is *recommended* on macOS; older versions may still work but are not
    officially supported
  - Boost >= 1.71.0 and ndn-cxx >= 0.9.0 are *required* on all platforms

- Added Dockerfile for NLSR. A prebuilt image for *linux/amd64* and *linux/arm64* platforms
  is available on the `GitHub container registry <https://github.com/named-data/NLSR/pkgs/container/nlsr>`__

Improvements
^^^^^^^^^^^^

- Defined equality operator for LSA classes and simplified LSA-related code in
  :nlsr:`NamePrefixList` (:issue:`4094`)
- Standardized on ``operator<<`` in LSA classes for printing rather than ``toString`` (:issue:`5308`)
- Introduced use of Boost ``bimap`` and ``multi_array`` classes to simplify implementation of
  :nlsr:`NameMap` and link-state calculations for :nlsr:`RoutingTable` respectively (:issue:`5308`)
- Removed forward dependency on :nlsr:`ConfParameter` in :nlsr:`SyncLogicHandler` (:issue:`4208`)
- Split routing calculator code from main class to separate files
  with shared header to allow for transparent changes to routing code
- Additional miscellaneous refactoring
- Update waf build system to version 2.0.27
- Fix building the documentation with Python 3.12 (:issue:`5298`)
- Various miscellaneous improvements to build and CI scripts

Bug fixes
^^^^^^^^^

- Prevent crashes if received segment lacks KeyLocator when LSDB is trying
  to fetch a certificate
- Prevent issues from dereferencing empty optional when link state calculator
  was passed a non-existent source node (:issue:`5308`)
- Hello data freshness has been dropped to 0 seconds; this prevents issues where Hellos can
  be cached and cause delays in detecting link failures in certain cases (:issue:`5265`)
- Removed dead links to tarballs in docs
- Various adjustments to match ndn-cxx namespace changes
