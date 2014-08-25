nlsr
====

Usage
-----

::

    nlsr [options]


Description
-----------

``nlsr`` is a daemon that implements routing protocol in NDN to populates NDN's Routing
Information Base.

Options:
--------


``-d``
  Run in daemon mode

``-f <FILE>``
  Specify configuration file name (default: ``./nlsr.conf``)

``-V``
  Display version information

``-h``
  Display this help message

Examples
--------

To run NLSR daemon and use a configuration file from the ``/path/to`` directory.

::

    nlsr -f /path/to/nlsr.conf

To run NLSR as daemon, use the ``-d`` flag:

::

    nlsr -d
