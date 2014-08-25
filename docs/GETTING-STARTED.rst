Getting Started with NLSR
=========================

.. toctree::
..

Getting Source
--------------

-  `GitHub NLSR repository <https://github.com/named-data/NLSR>`_

Installation
------------

:doc:`INSTALL`

Configuration
-------------

After installing NLSR from source, you need to create a configuration file for
NLSR. Please take a look at :doc:`manpages/nlsr.conf` for a sample configuration. For
details on configuring a router, please refer to :doc:`ROUTER-CONFIG`. For details on
security configuration, please refer to :doc:`SECURITY-CONFIG`.

Running
-------

Run nlsr with the following command:

::

    nlsr

NLSR will look for the default configuration file, ``nlsr.conf``, in the current directory.

You can also run ``nlsr -f`` with the absolute path of the configuration file:

::

    nlsr -f /usr/local/etc/ndn/nlsr.conf

To run NLSR as daemon, use the ``-d`` flag:

::

    nlsr -d
