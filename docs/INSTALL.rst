NLSR Installation Instructions
==============================

.. toctree::
..

Prerequisites
-------------

-  `NFD <https://named-data.net/doc/NFD/>`_ and its dependencies

Refer to `Getting started with NFD <https://named-data.net/doc/NFD/current/INSTALL.html>`_
for detailed installation and running instruction.

-  ChronoSync library

Download the ChronoSync library and build it according to the instructions available at
https://github.com/named-data/ChronoSync#build

-  PSync library

Download the PSync library and build it according to the instructions available at
https://github.com/named-data/PSync#build

Build
-----

Execute the following commands to build NLSR::

    ./waf configure
    ./waf
    sudo ./waf install

Refer to ``./waf --help`` for more options that can be used during the configure stage and
how to properly configure NLSR.

If your pkgconfig path is not set properly you can do the following before running ``./waf
configure``::

    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
    # or
    export PKG_CONFIG_PATH=/path/to/pkgconfig/on/your/machine
