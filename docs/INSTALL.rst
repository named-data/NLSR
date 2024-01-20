Installation Instructions
=========================

.. toctree::
..

Prerequisites
-------------

- `NFD <https://github.com/named-data/NFD>`__ and its dependencies.

  Check out the `documentation <https://docs.named-data.net/NFD/current/INSTALL.html>`__
  for detailed installation and running instructions.

- PSync library [optional but recommended, enabled by default]

  Download the PSync library and build it according to the instructions available at
  https://github.com/named-data/PSync#build

- ChronoSync library [optional, deprecated]

  For testing purposes, NLSR can be optionally built with ChronoSync support. Download
  the ChronoSync library and build it according to the instructions available at
  https://github.com/named-data/ChronoSync#build

- SVS library [optional]

  NLSR can be used with State Vector Sync as the underlying Sync protocol. Download
  the ndn-svs library and build it according to the instructions available at
  https://github.com/named-data/ndn-svs#installation

Note that at least one Sync library must be enabled.

Build
-----

Execute the following commands to build NLSR:

.. code-block:: sh

    ./waf configure
    ./waf
    sudo ./waf install

Refer to ``./waf --help`` for more options that can be used during the configure stage and
how to properly configure NLSR.

If your pkgconfig path is not set properly, you can do the following before running ``./waf
configure``:

.. code-block:: sh

    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
    # or
    export PKG_CONFIG_PATH=/path/to/pkgconfig/on/your/machine

If ChronoSync support is desired, NLSR needs to be configured with the following option:

.. code-block:: sh

    ./waf configure --with-chronosync
