NLSR - Named Data Link State Routing Protocol
=============================================

NLSR is a routing protocol in NDN that populates NDN's Routing Information Base. NLSR will
continue to evolve alongside the Named Data Networking `protocol
<http://named-data.net/doc/ndn-tlv/>`_.

NLSR is an open and free software package licensed under the GPL 3.0 license and free to
all Internet users and developers. For more information about the licensing details and
limitations, refer to `COPYING.md
<https://github.com/named-data/NLSR/blob/master/COPYING.md>`_.

NLSR is developed by the members of the `NSF-sponsored NDN project team
<http://named-data.net/project/participants/>`_. For more details, please refer to
`AUTHORS.md <https://github.com/named-data/NLSR/blob/master/AUTHORS.md>`_. Bug reports and
feedback are highly appreciated and can be made through the `NLSR Wiki`_.

The main design goal of NLSR is to provide a routing protocol to populate NDN's FIB. NLSR
calculates the routing table using link-state or hyperbolic routing and produces multiple
faces for each reachable name prefix in a single authoritative domain. NLSR will continue
to evolve over time to include neighbor discovery and to become a full fledged
inter-domain routing protocol for NDN.  The protocol design is presented in full detail in
the `NLSR Paper`_.

NLSR Documentation
-------------------

.. toctree::
   :hidden:
   :maxdepth: 3

   RELEASE-NOTES
   GETTING-STARTED
   INSTALL
   ROUTER-CONFIG
   SECURITY-CONFIG
   manpages

-  :doc:`GETTING-STARTED`
-  :doc:`INSTALL`
-  :doc:`ROUTER-CONFIG`
-  :doc:`SECURITY-CONFIG`
-  :doc:`RELEASE-NOTES`
-  :doc:`manpages`

Downloading
-----------

-  `GitHub NLSR repository <https://github.com/named-data/NLSR>`_

Additional information
----------------------

-  `NLSR Wiki`_

-  `NLSR Paper`_
   Detailed explanation of the NLSR design


.. _NLSR Wiki: http://redmine.named-data.net/projects/nlsr
.. _NLSR Paper: http://www.cs.memphis.edu/~lanwang/paper/nlsr-final.pdf
