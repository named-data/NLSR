Router Configuration
====================

.. toctree::
..

Example network and sample configuration
----------------------------------------

Assume that three routers in the same network, but at three different sites
(``memphis.edu``, ``arizona.edu``, and ``colostate.edu``), are connected to each other to
construct the following topology:

::

                        +-------------------------------------+
                        | /ndn/edu/memphis/%C1.Router/router1 |
                        +-------------------------------------+
                            / 11                      12\
                           /                             \
          route-cost = 25 /                               \ route-cost = 30
                         /                                 \
                     17 /                                   \ 13
    +-----------------------------------+ 7              10 +-------------------------------------+
    |/ndn/edu/arizona/%C1.Router/router3|-------------------|/ndn/edu/colostate/%C1.Router/router2|
    +-----------------------------------+   route-cost = 28 +-------------------------------------+

                          Figure: Network Topology

The number represents the connecting face id. For example,
``/ndn/edu/memphis/%C1.Router/router1`` is connected to
``/ndn/edu/arizona/%C1.Router/router3`` via face 11 and the route cost is 25. To reach
``/ndn/edu/colostate/%C1.Router/router2`` via face 12, the route cost is 30.

We will walk through setting up the faces and creating the configuration file for
``/ndn/edu/memphis/%C1.Router/router1``.

It is recommended to configure security as described at :doc:`SECURITY-CONFIG`, before
starting router configuration.

Step 1. Ensuring nfd is running
-------------------------------

Type the following in the terminal:

::

    nfd-status

If you see ``error while connecting to the forwarder (No such file or directory)``,
``nfd`` is not running. Follow the instructions in `Getting started with NFD
<https://docs.named-data.net/NFD/current/INSTALL.html>`_ to run nfd.

Since v0.4.0, NLSR no longer creates Faces for the neighbors that are
specified in ``nlsr.conf``. Instead, it relies on the pre-existence of
the Faces in NFD. NLSR will obtain this information from NFD, and
configure its neighbors using this information.

To create faces it is now necessary to use the ``nfdc`` command. See ``man nfdc`` for
its documentation (`online version <https://docs.named-data.net/NFD/current/manpages/nfdc.html>`__).

Step 2. Determining FaceUri
---------------------------

Assume that ``/ndn/edu/arizona/%C1.Router/router3`` has hostname ``router3.arizona.edu`` and
``/ndn/edu/colostate/%C1.Router/router2`` has IP address ``79.123.10.145``.
``/ndn/edu/memphis/%C1.Router/router1`` will consider FaceUri
``udp4://router3.arizona.edu`` for router ``/ndn/edu/arizona/%C1.Router/router3`` and
FaceUri ``udp4://79.123.10.145`` for router ``/ndn/edu/colostate/%C1.Router/router2``.

Step 3: Creating configuration file
-----------------------------------

Now, assume that ``/ndn/memphis.edu/router1`` wants to advertise three name prefixes
(``/ndn/memphis/sports/basketball/grizzlies``, ``/ndn/memphis/entertainment/blues``,
``/ndn/news/memphis/politics/lutherking``). The configuration file with the necessary
configuration commands follows:

::

    ; nlsr.conf starts here
    ; the general section contains all the general settings for router

    general
    {
        ; mandatory configuration command section network, site and router
        network /ndn/         ; name of the network the router belongs to in ndn URI format
        site /edu/memphis/    ; name of the site the router belongs to in ndn URI format
        router /%C1.Router/router1    ; name of the network the router belongs to in ndn URI format

        ; lsa-refresh-time is the time in seconds, after which router will refresh its LSAs
        lsa-refresh-time 1800      ; default value 1800. Valid values 240-7200

        ; InterestLifetime (in seconds) for LSA fetching
        lsa-interest-lifetime 4    ; default value 4. Valid values 1-60

        state-dir /var/lib/nlsr/ ; state directory to store all dynamic changes to NLSR
    }

    ; the neighbors section contains the configuration for router's neighbors and hello's behavior

    neighbors
    {
       ; in case hello interest timed out, router will try 'hello-retries' times at 'hello-time-out'
       ; seconds interval before giving up for any neighbors (deciding link is down)

       hello-retries 3                     ; interest retries number in integer. Default value 3
                                           ; valid values 1-10

       hello-timeout 1                    ; interest time out value in integer. Default value 1
                                          ; Valid values 1-15

       hello-interval  60                  ; interest sending interval in seconds. Default value 60
                                           ; valid values 30-90

       ; adj-lsa-build-interval is the time to wait in seconds after an Adjacency LSA
       ; build is scheduled before actually building the Adjacency LSA

       adj-lsa-build-interval 5   ; default value 5. Valid values 0-5. It is recommended that
                                  ; adj-lsa-build-interval have a lower value than
                                  ; routing-calc-interval

       ; first-hello-interval is the time to wait in seconds before sending the first Hello Interest

       first-hello-interval  10   ; Default value 10. Valid values 0-10

       ; neighbor command is used to configure router's neighbor. Each neighbor will need
       ; one block of neighbor command

      neighbor
      {
          name /ndn/edu/arizona/%C1.Router/router3   ; name prefix of the neighbor router consists
                                                     ; of network, site-name and router-name

          face-uri  udp4://router3.arizona.edu  ; face uri of the face connected to the neighbor
          link-cost 25                         ; cost of the connecting link to neighbor
      }

      neighbor
      {
          name /ndn/edu/colostate/%C1.Router/router2  ; name prefix of the neighbor router consists
                                                   ; of network, site-name and router-name

          face-uri  udp4://79.123.10.145     ; face uri of the face connected to the neighbor
          link-cost 30                        ; cost of the connecting link to neighbor
      }
    }

    ; the hyperbolic section contains the configuration settings of enabling
    a router to calculate ; routing table using `hyperbolic routing table
    calculation`_ method

    hyperbolic
    {
        ; commands in this section follows a strict order
        ; the switch is used to set hyperbolic routing calculation in NLSR

        state off          ; default value 'off', set value 'on' to enable hyperbolic routing table
                           ; calculation which turns link state routing 'off'. set value to 'dry-run'
                           ; to test hyperbolic routing and compare with link state routing.


        radius   123.456       ; radius of the router in hyperbolic coordinate system
        angle    1.45          ; angle of the router in hyperbolic coordinate system
    }


    ; the fib section is used to configure fib entry's type to ndn FIB updated by NLSR

    fib
    {
        ; the max-faces-per-prefix is used to limit the number of faces for each name prefixes
        ; by NLSR in ndn FIB

        max-faces-per-prefix 3  ; default value 0. Valid value 0-60. By default (value 0) NLSR adds
                                ; all available faces for each reachable name prefixes in NDN FIB

    }

    ; the advertising section contains the configuration settings of the
    name prefixes ; hosted by this router

    advertising
    {
        ; the ndnname is used to advertised name from the router. To advertise each name prefix
        ; configure one block of ndnname configuration command for every name prefix.

        prefix /ndn/memphis/sports/basketball/grizzlies
        prefix /ndn/memphis/entertainment/blues
        prefix /ndn/news/memphis/politics/lutherking
    }

By default NLSR's sequence file directory is set to ``/var/lib/nlsr/``. User must create this
directory before starting NLSR. Since this is a system directory, NLSR must either be executed
as root, or NLSR can be run as user "nlsr" after configuring the filesystem to grant "nlsr"
read and write permissions. If user lacks permissions, the following error will occur:

::

    User does not have read and write permission on the directory
    Error in configuration file processing! Exiting from NLSR

To avoid this, the directory can be set in a user-owned directory. For example:

::

    general
    {
      ...

      state-dir /home/username/nlsr/log  ; path for sequence directory and other dynamic changes (Absolute path)

    }
    ...

The user will encounter errors such as the following if security is not configured:

::

    Cannot read certificate from file: /home/username/NLSR/root.cert

To get rid of this and similar errors relating to security one has to configure security
as described in :doc:`SECURITY-CONFIG`. Security can be disabled for testing purposes as follows:

::

    ...
    trust-anchor
    {
      type any
      ;file-name "root.cert"
    }
    ...
    trust-anchor
    {
      type any
      ;file-name "site.cert"
    }
    ; cert-to-publish "root.cert"
    ; cert-to-publish "router.cert"
    ; cert-to-publish "site.cert"
    ; cert-to-publish "router.cert"

Or by simply replacing the whole security section with:

::

    security
    {
      validator
      {
        trust-anchor
        {
          type any
        }
      }
      prefix-update-validator
      {
        trust-anchor
        {
          type any
        }
      }
    }

Step 4: Running NLSR on /ndn/memphis.edu/router1
-------------------------------------------------

Assuming the configuration file is saved as ``nlsr.conf``, type the following to run nlsr:

::

    $ nlsr -f nlsr.conf

NLSR will look for nlsr.conf in the current directory. If nlsr.conf is not in the current
directory, please provide the absolute path with the file name as the value. If
``nlsr.conf`` resides in ``/home/ndnuser/configuration`` directory, type ``nlsr -f
/home/ndnuser/configuration/nlsr.conf`` to run nlsr.

The same process needs to be followed for ``/ndn/arizona.edu/router3`` and
``/ndn/colostate.edu/router2`` to run NLSR on these routers.

Expected Output
----------------

Assuming that all three routers are configured correctly and routing has converged,
``nfd-status`` in ``/ndn/edu/colostate/%C1.Router/router2`` will have the following (or similar)
entries for the name advertised by ``/ndn/edu/memphis/%C1.Router/router1``:

RIB:

::

    /ndn/memphis/entertainment/blues route={faceid=17 (origin=128 cost=25 ChildInherit), faceid=7 (origin=128 cost=58 ChildInherit)}
    /ndn/memphis/sports/basketball/grizzlies route={faceid=17 (origin=128 cost=25 ChildInherit), faceid=7 (origin=128 cost=58 ChildInherit)}
    /ndn/news/memphis/politics/lutherking route={faceid=17 (origin=128 cost=25 ChildInherit, faceid=7 (origin=128 cost=58 ChildInherit)}

This output can be seen by typing ``nfdc route list`` in the terminal.  Please refer to the
network figure for face IDs.

.. _hyperbolic routing table calculation: http://arxiv.org/abs/0805.1266
