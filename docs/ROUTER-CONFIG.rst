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

Step 1. Ensure nfd is running
-------------------------------

Type the following in the terminal:

::

    nfd-status -f

If you see ``ERROR: error while connecting to the forwarder (No such file or directory)``,
``nfd`` is not running. Follow the instructions in `Getting started with NFD
<http://named-data.net/doc/NFD/current/INSTALL.html>`_ to run nfd.

Step 2. Determining FaceUri
---------------------------

Assume that ``/ndn/edu/arizona/%C1.Router/router3`` has hostname ``router3.arizona.edu``,
``/ndn/edu/colostate/%C1.Router/router2`` has ip address ``79.123.10.145``, and that all
routers in the network have agreed to sync data with name prefix ``/ndn/nlsr/sync``.
``/ndn/edu/memphis/%C1.Router/router1`` will consider face uri
``udp4://router3.arizona.edu`` for router ``/ndn/edu/arizona/%C1.Router/router3`` and face
uri ``udp4://79.123.10.145`` for router ``/ndn/edu/colostate/%C1.Router/router2``.

Step 3: Creating configuration file
-----------------------------------

Now, assume that ``/ndn/memphis.edu/router1`` wants to advertise three name prefixes
(``/ndn/memphis/sports/basketball/grizzlies``, ``/ndn/memphis/entertainment/blues``,
``/ndn/news/memphis/politics/lutherking``). The configuration file with minimum
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

        ; log-level is to set the levels of log for NLSR

        log-level  INFO       ; default value INFO, valid value DEBUG, INFO
        log-dir /var/log/nlsr/
        seq-dir /var/lib/nlsr/
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
       ; neighbor command is used to configure router's neighbor. Each neighbor will need
       ; one block of neighbor command

      neighbor
      {
          name /ndn/edu/arizona/%C1.Router/router3        ; name prefix of the neighbor router consists
                                                          ; of network, site-name and router-name

          face-uri  udp4://router3.arizona.edu  ; face uri of the face connected to the neighbor
          link-cost 25                         ; cost of the connecting link to neighbor
      }

      neighbor
      {
          name /ndn/edu/colostate/%C1.Router/router2          ; name prefix of the neighbor router consists
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

        state off             ; default value 'off', set value 'on' to enable hyperbolic routing table
                              ; calculation which turns link state routing 'off'. set value to 'dry-run"
                              ; to test hyperbolic routing and compare with link state routing.


        radius   123.456       ; radius of the router in hyperbolic coordinate system
        angle    1.45          ; angle of the router in hyperbolic coordinate system
    }


    ; the fib section is used to configure fib entry's type to ndn FIB updated by NLSR

    fib
    {
        ; the max-faces-per-prefix is used to limit the number of faces for each name prefixes
        ; by NLSR in ndn FIB

        max-faces-per-prefix 3   ; default value 0. Valid value 0-60. By default (value 0) NLSR adds
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

.. note::

    Security configuration is discussed in :doc:`SECURITY-CONFIG`, which will be also part
    of this configuration file

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

Assuming that all three routers are configured correctly and have converged, ``nfd-status`` in
``/ndn/edu/colostate/%C1.Router/router2`` will have the following entries for the name
advertised by ``/ndn/edu/memphis/%C1.Router/router1``:

FIB:

::

    /ndn/memphis/sports/basketball/grizzlies nexthops={faceid=17 (cost=25), faceid=7 (cost=58)}
    /ndn/memphis/entertainment/blues nexthops={faceid=17 (cost=25), faceid=7 (cost=58)}
    /ndn/news/memphis/politics/lutherking nexthops={faceid=17 (cost=25), faceid=7 (cost=58)}

This output can be seen by typing ``nfd-status -b`` in the terminal.  Please refer to the
network figure for face IDs.

.. _hyperbolic routing table calculation: http://arxiv.org/abs/0805.1266
