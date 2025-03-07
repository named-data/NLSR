nlsrc
=====

Usage
-----

::

    nlsrc [-h | -V]
    nlsrc [-R <router prefix> [-c <nlsr.conf path> | -k]] COMMAND [<Command Options>]


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

``-R <router prefix>``
  Target a remote NLSR instance.
  The default is the local NLSR instance ``/localhost``.

``-c <nlsr.conf path>``
  Verify remote status information with the trust schema loaded from ``security.validator`` section of specified nlsr.conf config file.
  The default is loading from ``/etc/ndn/nlsr.conf``.

``-k``
  Insecure: do not verify signature on status information retrieved from remote router.

``COMMAND``

  ``lsdb``
    Retrieve LSDB status information

  ``routing``
    Retrieve routing table status information

  ``status``
    Retrieve LSDB status and routing table status information

  ``advertise``
    Add a Name prefix to be advertised by NLSR

    ``advertise <name>``

      ``name``
        The Name prefix to be advertised

    ``advertise <name> save``

      ``save``
        Advertise a prefix and also save it to the nlsr.conf file residing in the state-dir for the next start of NLSR that operator may copy and use for the next start of NLSR

  ``withdraw``
    Remove a Name prefix advertised through NLSR

    ``withdraw <name>``

      ``name``
        The Name prefix to be withdrawn

    ``withdraw <name> delete``

      ``delete``
        Withdraw a prefix and also delete it from the nlsr.conf file residing in the state-dir

Notes
-----

When security is enabled, NLSR will not be allowed to successfully
advertise/withdraw names without first setting a default identity of operator.
If default identity is not set as operator, the user will be presented with the
error message: "Name prefix update error (code: 403)". To remedy this
issue, use the command ``ndnsec set-default``. Example::

  ndnsec set-default /ndn/a-site/%C1.Operator/op

Support for adding prefix costs is currently only implemented via readvertisement;
this functionality will be added in a future release.

Exit Status
-----------

nlsrc exits with one of the following values::

  0     nlsrc exited successfully
  1     generic error
  2     bad command line
