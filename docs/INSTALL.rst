NLSR Installation Instructions
==============================

.. toctree::
..

Prerequisites
-------------

-  `NFD <http://named-data.net/doc/NFD/current/>`_ and its requirements:

Refer to `Getting started with NFD <http://named-data.net/doc/NFD/current/INSTALL.html>`_
for detailed installation and running instruction.

-  log4cxx library

   On Ubuntu Linux:

   ::

          sudo apt-get install liblog4cxx10-dev

   On OS X with MacPorts:

   ::

          sudo port install log4cxx

-  protobuf

   On Ubuntu Linux:

   ::

          sudo apt-get install libprotobuf-dev protobuf-compiler

   On OS X with MacPorts:

   ::

          sudo port install protobuf-cpp

-  openssl

   On Ubuntu Linux:

   ::

          sudo apt-get install libssl-dev

   On OS X with MacPorts:

   ::

          sudo port install openssl

Build
-----

Execute the following commands to build NLSR:

::

    ./waf configure
    ./waf
    sudo ./waf install

Refer to ``./waf –help`` for more options that can be used during the configure stage and
how to properly configure NLSR.

If your pkgconfig path is not set properly you can do the following before running ``./waf
configure``

::

    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
    or
    export PKG_CONFIG_PATH=/path/to/pkgconfig/on/your/machine
