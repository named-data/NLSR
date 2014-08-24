NLSR Installation Instructions
==============================

.. toctree::
..

Prerequisites
-------------

-  `NFD`_ and its requirements:

Refer to `Getting started with NFD`_ for detailed installation and running instruction.

-  log4cxx library

   On Linux:

   ::

          sudo apt-get install liblog4cxx10-dev

   On Mac OSx:

   ::

          sudo port install log4cxx

-  protobuf

   On Linux:

   ::

          sudo apt-get install libprotobuf-dev protobuf-compiler

   On Mac OSx:

   ::

          sudo port install protobuf-cpp

Build
-----

The following commands should be used to build NLSR:

::

    ./waf configure
    ./waf
    sudo ./waf install

Refer to ./waf –help for more options that can be used during the configure
stage and how to properly configure NLSR.

If your pkgconfig path is not set properly you can do following before
running ``./waf configure``

::

    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
    or
    export PKG_CONFIG_PATH=/path/to/pkgconfig/in/your/machine

.. _NFD: http://named-data.net/doc/NFD/current/
.. _Getting started with NFD: http://named-data.net/doc/NFD/current/INSTALL.html
