Getting Started with NLSR
=============================================

Getting Source
--------------

- [Github NLSR repository](https://github.com/named-data/NLSR)

Installation instruction
------------------------

- [Installation](https://github.com/named-data/NLSR/blob/master/docs/INSTALL.md)

Installing
----------

To install NLSR from source:

- Install ndn-cxx according to [ndn-cxx installation instructions](http://named-data.net/doc/ndn-cxx/0.1.0/INSTALL.html)
- Install nfd according to [nfd installation instructions](http://named-data.net/doc/NFD/current/INSTALL.html)
- Install NLSR following [these instructions](https://github.com/named-data/NLSR/blob/master/docs/INSTALL.md)

Configuration
-------------

After installing NLSR from source, you need to create a configuration file for
NLSR. Please take a look at [nlsr.conf](https://github.com/named-data/NLSR/blob/master/nlsr.conf)
for a sample configuration. For details on configuring a router, please refer to
[build network topology](https://github.com/named-data/NLSR/blob/master/docs/TOPOLOGY.md)
and for security configuration please refer to
[NLSR's security configuration](https://github.com/named-data/NLSR/blob/master/docs/SECURITY.md)

Running
-------

Run nlsr with the following command:

    nlsr

NLSR will look for the default configuration file, nlsr.conf, in the current directory.

You can also run nlsr -f options as follows with the absolute path of the configuration file:

    nlsr -f /nlsr/nlsr.conf
