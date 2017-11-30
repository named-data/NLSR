NLSR - Named Data Link State Routing Protocol
=============================================

NLSR is a routing protocol in NDN that populates NDN's Routing Information Base. NLSR will
continue to evolve alongside the Named Data Networking `protocol
<https://named-data.net/doc/ndn-tlv/>`_.

NLSR is an open and free software package licensed under the GPL 3.0 license and free to
all Internet users and developers. For more information about the licensing details and
limitations, refer to `COPYING.md
<https://github.com/named-data/NLSR/blob/master/COPYING.md>`_.

NLSR is developed by the members of the `NSF-sponsored NDN project team
<https://named-data.net/project/participants/>`_. For more details, please refer to
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

   GETTING-STARTED
   INSTALL
   ROUTER-CONFIG
   SECURITY-CONFIG
   RELEASE-NOTES
   releases
   manpages

-  :doc:`GETTING-STARTED`
-  :doc:`INSTALL`
-  :doc:`ROUTER-CONFIG`
-  :doc:`SECURITY-CONFIG`
-  :doc:`RELEASE-NOTES`
-  :doc:`releases`
-  :doc:`manpages`

Downloading
-----------

-  `GitHub NLSR repository <https://github.com/named-data/NLSR>`_

NLSR mailing list
-----------------

-  `NLSR Mailing List`_

-  `NLSR Mailing List Archives`_

`Sign up`_ is needed to post to the list or access the archives.

Additional information
----------------------

-  `NLSR Wiki`_

-  `NLSR Redmine`_
   Feature requests and bug reports

-  `NLSR Paper`_
   Detailed explanation of the NLSR design

-  `NLSR Developer's Guide`_
   For NLSR Development

.. _NLSR Wiki: https://redmine.named-data.net/projects/nlsr/wiki
.. _NLSR Paper: https://named-data.net/wp-content/uploads/2016/01/ndn-0037-1-nlsr.pdf
.. _NLSR Mailing List: https://listserv.memphis.edu/scripts/wa.exe?SUBED1=NLSR-HELP-L&A=1
.. _NLSR Mailing List Archives: https://listserv.memphis.edu/scripts/wa.exe?A0=NLSR-HELP-L
.. _NLSR Developer's Guide: https://github.com/named-data/NLSR/blob/developers-guide/NLSR-Developers-Guide.pdf
.. _Sign up: https://listserv.memphis.edu/scripts/wa.exe?GETPW1
.. _NLSR Redmine: https://redmine.named-data.net/projects/nlsr
