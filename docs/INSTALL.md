NLSR Installation Instructions
=============================

Prerequisites
-------------

- [ndn-cxx library](https://github.com/named-data/ndn-cxx) and its requirements:

         libcrypto
         libsqlite3
         libcrypto++
         pkg-config
         Boost libraries (>= 1.48)
         OSX Security framework (on OSX platform only)

   Refer to [Getting started with ndn-cxx](http://named-data.net/doc/ndn-cxx/current/INSTALL.html) -for detailed installation instructions of the library.


- [NFD](https://github.com/named-data/nfd) and its requirements:

   Refer to [Getting started with NFD ](http://named-data.net/doc/NFD/current/getting-started.html)-for detailed installation and running instruction.


- log4cxx library

       On linux variants

           sudo apt-get install liblog4cxx10-dev

       On Mac OSx

           sudo port install log4cxx

- protobuf

      On Mac OSx

           sudo port install protobuf-cpp

      On linux variants

           sudo apt-get install libprotobuf-dev protobuf-compiler


Build
-----

The following commands should be used to build NLSR:

    ./waf configure
    ./waf
    sudo ./waf install

Refer to ./waf --help for more options that can be used during configure stage and
how to properly configure and run NLSR.

If your pkgconfig path is not set properly you can do following before running `./waf configure`

    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/
    or
    export PKG_CONFIG_PATH=/path/to/pkgconfig/in/your/machine
