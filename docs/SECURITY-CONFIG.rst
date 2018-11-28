Security Configuration
=======================

.. toctree::
..

The trust model of NLSR is semi-hierarchical. An example certificate signing hierarchy is:

::

                                            root
                                             |
                              +--------------+---------------+
                            site1                          site2
                              |                              |
                    +---------+---------+                    +
                 operator1           operator2            operator3
                    |                   |                    |
              +-----+-----+        +----+-----+        +-----+-----+--------+
           router1     router2  router3    router4  router5     router6  router7
              |           |        |          |        |           |        |
              +           +        +          +        +           +        +
            NLSR        NLSR     NLSR       NLSR     NLSR        NLSR     NLSR

Each entity's name and corresponding certificate name follow the
convention below:

========  ===================================================== =================================  ===============================================
Entity    Identity Name                                         Example                            Certificate Name Example
--------  ----------------------------------------------------- ---------------------------------  -----------------------------------------------
root      /\<network\>                                          /ndn                               /ndn/KEY/1/%00/%01
site      /\<network\>/\<site\>                                 /ndn/edu/ucla                      /ndn/edu/ucla/KEY/2/%00/%01
operator  /\<network\>/\<site\>/%C1.Operator/\<operator-name\>  /ndn/edu/ucla/%C1.Operator/op1     /ndn/edu/ucla/%C1.Operator/op1/KEY/3/%00/%01
router    /\<network\>/\<site\>/%C1.Router/\<router-name\>      /ndn/edu/ucla/%C1.Router/rt1       /ndn/edu/ucla/%C1.Router/rt1/KEY/4/%00/%01
NLSR      /\<network\>/\<site\>/%C1.Router/\<router-name\>/NLSR /ndn/edu/ucla/%C1.Router/rt1/NLSR  /ndn/edu/ucla/%C1.Router/rt1/NLSR/KEY/5/%00/%01
========  ===================================================== =================================  ===============================================

Users should create keys according to the naming and signing hierarchies above. The
``%C1.Operator`` and ``%C1.Router`` labels are NDN keywords that should not be modified
by the user.


.. note::

    Detailed information about key setup is given in NLSR Developer's Guide and the
    :doc:`beginners-guide`.

Certificate Publishing
----------------------

In a network, every router should have the root certificate configured as a trust
anchor. For each site, at least one router should publish the site certificate, and at
least one router should publish the certificate of the site operator. Each router should
publish its own certificate. All this information should be explicitly specified in the
nlsr.conf file. For example, the following configuration file indicates that NLSR should
publish the site certificate and the router certificate:

::

    ...
    security
    {
      validator
      {
        ...
      }
      cert-to-publish "site.cert"   ; name of the file which contains the site certificate (optional).
      cert-to-publish "router.cert" ; name of the file which contains the router certificate (required).
      ...
    }
