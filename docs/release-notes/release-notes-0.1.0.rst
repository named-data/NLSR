NLSR version 0.1.0 (initial release)
++++++++++++++++++++++++++++++++++++

Release date: August 25, 2014

The main design goal of NLSR is to provide a routing protocol to populate `NFD's
<https://github.com/named-data/NFD>`__ RIB.  NLSR calculates the routing table using
link-state or hyperbolic routing and produces multiple faces for each reachable name
prefix in a single authoritative domain.

Included features
^^^^^^^^^^^^^^^^^

- Advertise availability of content through the configured router

- Use `ChronoSync <https://github.com/named-data/ChronoSync>`__ to synchronize routers' LSA sequence numbers and Interest/Data to retrieve LSAs

- Produce a list of ranked forwarding options for each name prefix to facilitate NDN's adaptive forwarding strategies

- Configure maximum number of faces per prefix in NFD's RIB

- Set up NFD tunnels to neighbors automatically and maintain the tunnels to neighbors

- Provide two routing protocols:

  + Link State routing: calculate route cost based on link costs
  + Hyperbolic routing: calculate route cost based on hyperbolic coordinates

- Use a hierarchical trust model for routing within a single administrative domain

- Keep NFD's RIB updated on failure and recovery
