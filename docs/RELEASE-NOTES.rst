Release Notes
==============

.. toctree::
..

Version 0.1.0 is the initial release of NLSR, a routing protocol for NDN.

The main design goal of NLSR is to provide a routing protocol to
populate `NFD`_’s FIB. NLSR calculates the routing table using link-state
or hyperbolic routing and produces multiple faces for each reachable
name prefix in a single authoritative domain.

The current features include:

- **Produce a list of ranked forwarding options for each name prefix to facilitate NDN’s adaptive forwarding strategies**

- **Name Prefix advertisement**

  + Advertise availability of data through the configured router

- **Use ChronoSync to synchronize routers' LSA sequence numbers and Interest/Data to retrieve LSAs**

- **Limit faces per prefix**

  + Configure maximum number of faces per prefix in NFD's FIB

- **Automatic NFD tunnel configuration and maintainance to neighbors**

- **Routing protocol**

  + Link State routing
  + **Hyperbolic routing**

    + Calculates route cost based on a node's polar coordinates

- **Use a hierarchical trust model for routing within a single administrative domain**

- **NFD Forwarding Information Base (FIB) consistency**

  + Provides shortest path next hops for efficient forwarding
  + Keeps NFD's FIB updated on node failure and recovery

.. _NFD: http://named-data.net/doc/NFD/current/