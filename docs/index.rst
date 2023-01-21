NLSR: Named Data Link State Routing Protocol
============================================

.. toctree::
   :hidden:
   :maxdepth: 2

   GETTING-STARTED
   INSTALL
   SECURITY-CONFIG
   ROUTER-CONFIG
   beginners-guide
   manpages
   release-notes
   releases

**NLSR** is a routing protocol for Named Data Networking (NDN) that populates the Routing
Information Base (RIB) of an NDN router, such as `NFD`_. NLSR continues to evolve alongside
the core `NDN protocol`_.

The main design goal of NLSR is to provide a routing protocol to populate NDN's FIB. NLSR
calculates the routing table using link-state or hyperbolic routing and produces multiple
faces for each reachable name prefix in a single authoritative domain. NLSR will continue
to evolve over time to include neighbor discovery and to become a full fledged
inter-domain routing protocol for NDN.  The protocol design is presented in full detail in
the `NLSR Paper`_.

NLSR was mainly developed by members of the NSF-sponsored `NDN project team
<https://named-data.net/project/participants/>`__.  See `AUTHORS.md
<https://github.com/named-data/NLSR/blob/master/AUTHORS.md>`__ for details.

Bug reports and feedback are highly appreciated and can be made through the `NLSR Redmine`_.

Documentation
-------------

- :doc:`GETTING-STARTED`
- :doc:`INSTALL`
- :doc:`SECURITY-CONFIG`
- :doc:`ROUTER-CONFIG`
- :doc:`beginners-guide`
- :doc:`manpages`
- :doc:`release-notes`
- :doc:`releases`

Downloading
-----------

- `Source code git repository <https://github.com/named-data/NLSR>`__

NLSR mailing list
-----------------

- `NLSR Mailing List`_
- `NLSR Mailing List Archives`_

`Sign up`_ is needed to post to the list or access the archives.

Additional resources
--------------------

- `NLSR Wiki`_

- `NLSR Redmine`_
  Feature requests and bug reports

- `NLSR Paper`_
  Detailed explanation of NLSR's design

- `NLSR Developer's Guide`_
  For NLSR development

- `API documentation (doxygen) <doxygen/annotated.html>`__

License
-------

NLSR is free software distributed under the GNU General Public License version 3.
See `COPYING.md <https://github.com/named-data/NLSR/blob/master/COPYING.md>`__ for details.

.. _NDN protocol: https://docs.named-data.net/NDN-packet-spec/current/
.. _NFD: https://github.com/named-data/NFD
.. _NLSR Wiki: https://redmine.named-data.net/projects/nlsr/wiki
.. _NLSR Paper: https://named-data.net/wp-content/uploads/2016/01/ndn-0037-1-nlsr.pdf
.. _NLSR Mailing List: https://listserv.memphis.edu/scripts/wa.exe?SUBED1=NLSR-HELP-L&A=1
.. _NLSR Mailing List Archives: https://listserv.memphis.edu/scripts/wa.exe?A0=NLSR-HELP-L
.. _NLSR Developer's Guide: https://github.com/named-data/NLSR/blob/developers-guide/NLSR-Developers-Guide.pdf
.. _Sign up: https://listserv.memphis.edu/scripts/wa.exe?GETPW1
.. _NLSR Redmine: https://redmine.named-data.net/projects/nlsr
