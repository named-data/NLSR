# NLSR: Named Data Link State Routing Protocol

![Latest version](https://img.shields.io/github/v/tag/named-data/NLSR?label=Latest%20version)
![Language](https://img.shields.io/badge/C%2B%2B-17-blue)
[![CI](https://github.com/named-data/NLSR/actions/workflows/ci.yml/badge.svg)](https://github.com/named-data/NLSR/actions/workflows/ci.yml)
[![Docs](https://github.com/named-data/NLSR/actions/workflows/docs.yml/badge.svg)](https://github.com/named-data/NLSR/actions/workflows/docs.yml)

> [!TIP]
> For more extensive documentation, visit the [NLSR homepage](https://docs.named-data.net/NLSR/current/).

## Overview

NLSR is a routing protocol for Named Data Networking (NDN) that populates the Routing Information
Base (RIB) of an NDN router, such as [NFD](https://github.com/named-data/NFD). NLSR continues to
evolve alongside the core [NDN protocol](https://docs.named-data.net/NDN-packet-spec/current/).

The main design goal of NLSR is to provide a routing protocol to populate NDN's RIB.
NLSR calculates the routing table using link-state or hyperbolic routing and produces
multiple faces for each reachable name prefix in a single authoritative domain. NLSR
will continue to evolve over time to include neighbor discovery and to become a full
fledged inter-domain routing protocol for NDN.

NLSR was mainly developed by members of the NSF-sponsored [NDN project team](
https://named-data.net/project/participants/). See [`AUTHORS.md`](AUTHORS.md) for details.

## Source releases

The source code and installation instructions are available at the following locations:

- [Installation](https://docs.named-data.net/NLSR/current/INSTALL.html)
- [Getting started with NLSR](https://docs.named-data.net/NLSR/current/GETTING-STARTED.html)
- [NLSR GitHub repository](https://github.com/named-data/NLSR)

## Reporting bugs

Please submit any bug reports or feature requests to the
[NLSR issue tracker](https://redmine.named-data.net/projects/nlsr/issues).

## Additional resources

- [Contributor's Guide](https://github.com/named-data/.github/blob/main/CONTRIBUTING.md)
- [NLSR Wiki](https://redmine.named-data.net/projects/nlsr/wiki)
- [NLSR Issue Tracker](https://redmine.named-data.net/projects/nlsr/issues)
- [NLSR Mailing List Sign Up](https://listserv.memphis.edu/scripts/wa.exe?GETPW1)
- [NLSR Mailing List](https://listserv.memphis.edu/scripts/wa.exe?SUBED1=NLSR-HELP-L&A=1)
- [NLSR Mailing List Archives](https://listserv.memphis.edu/scripts/wa.exe?A0=NLSR-HELP-L)
- [NLSR Developer's guide](https://github.com/named-data/NLSR/blob/developers-guide/NLSR-Developers-Guide.pdf)

## License

NLSR is free software distributed under the GNU General Public License version 3.
See [`COPYING.md`](COPYING.md) for details.

NLSR contains third-party software, licensed under the following licenses:

- The *waf* build system is licensed under the [3-clause BSD license](waf)
