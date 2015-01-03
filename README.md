NLSR - Named Data Link State Routing Protocol
=============================================

For complete documentation and more extensive information,
please visit the [NLSR homepage](http://named-data.net/doc/NLSR/current/).

## Overview

NLSR is a routing protocol in NDN that populates NDN's Routing Information Base. NLSR
will continue to evolve alongside the Named Data Networking [protocol](http://named-data.net/doc/ndn-tlv/).

NLSR is an open and free software package licensed under the GPL 3.0 license and free to all
Internet users and developers.  For more information about the licensing details and
limitations, refer to [COPYING.md](https://github.com/named-data/NLSR/blob/master/COPYING.md).

NLSR is developed by the members of the [NSF-sponsored NDN project team](http://named-data.net/project/participants/).
For more details, please refer to [AUTHORS.md](https://github.com/named-data/NLSR/blob/master/AUTHORS.md).
Bug reports and feedback are highly appreciated and can be made through the
[Redmine site](http://redmine.named-data.net/projects/nlsr).

The main design goal of NLSR is to provide a routing protocol to populate NDN's FIB.
NLSR calculates the routing table using link-state or hyperbolic routing and produces
multiple faces for each reachable name prefix in a single authoritative domain. NLSR
will continue to evolve over time to include neighbor discovery and to become a full
fledged inter-domain routing protocol for NDN.


Source releases
---------------

The source code and source-code installation instructions are always available at
the following links:

- [Installation](http://named-data.net/doc/NLSR/current/INSTALL.html)
- [Getting Started with NLSR](http://named-data.net/doc/NLSR/current/GETTING-STARTED.html)
- [GitHub NLSR repository](https://github.com/named-data/NLSR)

Additional information
----------------------

- [NLSR Wiki](http://redmine.named-data.net/projects/nlsr/wiki/)
- Feature requests and bug reports are welcome on our
  [NLSR on NDN Redmine](http://redmine.named-data.net/projects/nlsr)
