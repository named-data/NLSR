NLSR version 0.7.0
------------------

Release date: August 18, 2023

New features
^^^^^^^^^^^^

- Add experimental support for State Vector Sync (SVS) (:issue:`5248`)
- ``nlsrc``: Add an option to query the status from remote routers (:issue:`4544`)

Notable changes and improvements
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- **Deprecate ChronoSync support**: ChronoSync support is now optional and excluded from the
  default build; it remains available mainly for testing purposes (:issue:`5147`)
- Switched to the C++17 standard
- The minimum build requirements have been increased as follows:

  - Either GCC >= 7.4.0 or Clang >= 6.0 is required on Linux
  - On macOS, Xcode 11.3 or later is recommended; older versions may still work but are not
    officially supported
  - Boost >= 1.65.1 and ndn-cxx >= 0.8.1 are required on all platforms
  - Sphinx 4.0 or later is required to build the documentation

- Refactor LSDB for readability and add signals to notify of modifications (:issue:`4127`)
- Retrieve LSA from the face that triggered the notification of LSA changes
- Accommodate certificate name in ``KeyLocator`` (:issue:`5195`)
- Change ``sig-type`` to ``ecdsa-sha256`` in ``nlsr.conf`` as ndn-cxx now has strict checking for it
- Improve error handling and reporting
- Stop using the ``gold`` linker on Linux; prefer instead linking with ``lld`` if installed
- Update waf build system to version 2.0.24
- Various test suite and documentation improvements

Bug fixes
^^^^^^^^^

- lsdb: Fix for issues involving incorrect retransmission behavior in segment fetcher
- Prevent NLSR from unregistering route during update for router that advertised it initially
  (:issue:`5179`)
- NameLsa: fix LSDB update bug by sorting the NPLs before difference
- Fix recurring sync prefix registration on every hello data (:issue:`5157`)
- Return the appropriate exit codes when executing ``nlsrc`` advertise/withdraw commands
  (:issue:`5271`)
