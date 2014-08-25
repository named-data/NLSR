Release Notes
=============

NLSR version 0.1.0 (initial release)
++++++++++++++++++++++++++++++++++++

Release date: August 25, 2014

The main design goal of NLSR is to provide a routing protocol to populate `NFD's
<http://named-data.net/doc/NFD/current/>`_ RIB.  NLSR calculates the routing table using
link-state or hyperbolic routing and produces multiple faces for each reachable name
prefix in a single authoritative domain.

**Included features**:

- Produce a list of ranked forwarding options for each name prefix to facilitate NDN's adaptive forwarding strategies

- Name Prefix advertisement

  + Advertise availability of content through the configured router

- Use `ChronoSync <https://github.com/named-data/ChronoSync>`_ to synchronize routers' LSA sequence numbers and Interest/Data to retrieve LSAs

- Limit faces per prefix

  + Configure maximum number of faces per prefix in NFD's RIB

- Automatic NFD tunnel configuration and maintenance to neighbors

- Routing protocol

  + Link State routing
  + Hyperbolic routing

    + Calculates route cost based on a node's polar coordinates

- Use a hierarchical trust model for routing within a single administrative domain

- NFD Routing Information Base (RIB) consistency

  + Provides shortest path next hops for efficient forwarding
  + Keeps NFD's RIB updated on node failure and recovery
