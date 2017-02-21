nlsr
====

Usage
-----

::

    nlsr [options]


Description
-----------

``nlsr`` is a process that implements routing protocol in NDN to populate NDN's Routing
Information Base.

Options:
--------

``-f <FILE>``
  Specify configuration file name (default: ``./nlsr.conf``)

``-V``
  Display version information

``-h``
  Display this help message

Examples
--------

To run NLSR and use a configuration file from the ``/path/to`` directory.

::

    nlsr -f /path/to/nlsr.conf
