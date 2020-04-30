# NLSR: Named Data Link State Routing Protocol

![Language](https://img.shields.io/badge/C%2B%2B-14-blue.svg)
[![Build Status](https://travis-ci.org/named-data/NLSR.svg?branch=master)](https://travis-ci.org/named-data/NLSR)
![Latest Version](https://img.shields.io/github/tag/named-data/NLSR.svg?color=darkkhaki&label=latest%20version)

For complete documentation and more extensive information,
please visit the [NLSR homepage](https://named-data.net/doc/NLSR/current/).

If you are new to the NDN software community, please read the
[Contributor's Guide](https://github.com/named-data/.github/blob/master/CONTRIBUTING.md)
to get started.

## Overview

NLSR is a routing protocol in NDN that populates NDN's Routing Information Base.
NLSR will continue to evolve alongside the Named Data Networking
[protocol](https://named-data.net/doc/NDN-packet-spec/current/).

NLSR is an open and free software package licensed under the GPL 3.0 license and free to
all Internet users and developers.  For more information about the licensing details and
limitations, refer to [`COPYING.md`](COPYING.md).

NLSR is developed by the members of the [NSF-sponsored NDN project team](https://named-data.net/project/participants/).
For more details, please refer to [`AUTHORS.md`](AUTHORS.md).
Bug reports and feedback are highly appreciated and can be made through our
[Redmine site](https://redmine.named-data.net/projects/nlsr/issues).

The main design goal of NLSR is to provide a routing protocol to populate NDN's RIB.
NLSR calculates the routing table using link-state or hyperbolic routing and produces
multiple faces for each reachable name prefix in a single authoritative domain. NLSR
will continue to evolve over time to include neighbor discovery and to become a full
fledged inter-domain routing protocol for NDN.

## Source releases

The source code and source-code installation instructions are always available at
the following links:

- [Installation](https://named-data.net/doc/NLSR/current/INSTALL.html)
- [Getting Started with NLSR](https://named-data.net/doc/NLSR/current/GETTING-STARTED.html)
- [GitHub NLSR repository](https://github.com/named-data/NLSR)

## Additional information

- [Contributor's Guide](https://github.com/named-data/.github/blob/master/CONTRIBUTING.md)
- [NLSR Wiki](https://redmine.named-data.net/projects/nlsr/wiki/)
- Feature requests and bug reports are welcome on our
  [NLSR on NDN Redmine](https://redmine.named-data.net/projects/nlsr/issues)
- [NLSR Mailing List Sign Up](https://listserv.memphis.edu/scripts/wa.exe?GETPW1)
- [NLSR Mailing List](https://listserv.memphis.edu/scripts/wa.exe?SUBED1=NLSR-HELP-L&A=1)
- [NLSR Mailing List Archives](https://listserv.memphis.edu/scripts/wa.exe?A0=NLSR-HELP-L)
- [NLSR Developer's guide](https://github.com/named-data/NLSR/blob/developers-guide/NLSR-Developers-Guide.pdf)
