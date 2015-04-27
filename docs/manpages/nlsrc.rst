nlsrc
=====

Usage
-----

::

    nlsrc [-h] [-V] COMMAND [<Command Options>]


Description
-----------

``nlsrc`` is a tool to retrieve link-state database (LSDB) status information from NLSR and
announce/withdraw Name prefixes advertised by NLSR.

Options
-------

``-h``
  Print usage information

``-V``
  Show NLSRC version information

``COMMAND``

  ``status``
    Retrieve LSDB status information

  ``advertise``
    Add a Name prefix to be advertised by NLSR

    ``advertise <name>``

      ``name``
        The Name prefix to be advertised

  ``withdraw``
    Remove a Name prefix advertised through NLSR

    ``withdraw <name>``

      ``name``
        The Name prefix to be withdrawn

Exit Status
-----------

nlsrc exits with one of the following values:
::

  0     nlsrc exited successfully
  >0    An error occurred
