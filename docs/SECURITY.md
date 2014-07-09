##Trust Model For NLSR

The trust model of NLSR is semi-hierarchical.
An example certificate signing hierarchy is:

                                            root
                                             |
                              +--------------+---------------+
                            site1                          site2
                              |                              |
                    +---------+---------+                    +
                 operator1           operator2            operator3
                    |                   |                    |
              +-----+-----+        +----+-----+        +-----+-----+--------+
           router1     router2  router3    router4  router5     router6  router7
              |           |        |          |        |           |        |
              +           +        +          +        +           +        +
            NLSR        NSLR     NSLR       NSLR     NSLR        NSLR     NSLR

Each entity's name and corresponding certificate name follow the convention below:

    Entity   | Identity Name                                     | Example                         | Certificate Name Example
    -------- | ------------------------------------------------- | ------------------------------- | -----------------------------------------------
    root     | /\<network\>                                        | /ndn                            | /ndn/KEY/ksk-1/ID-CERT/%01
    site     | /\<network\>/\<site\>                                 | /ndn/edu/ucla                   | /ndn/edu/ucla/KEY/ksk-2/ID-CERT/%01
    operator | /\<network\>/\<site\>/%C1.Operator/\<operator-name\>          | /ndn/edu/ucla/%C1.Operator/op1      | /ndn/edu/ucla/%C1.Operator/op1/KEY/ksk-3/ID-CERT/%01
    router   | /\<network\>/\<site\>/%C1.Router/\<router-name\>            | /ndn/edu/ucla/%C1.Router/rt1      | /ndn/edu/ucla/%C1.Router/rt1/KEY/ksk-4/ID-CERT/%01
    NLSR     | /\<network\>/\<site\>/%C1.Router/\<router-name\>/NLSR       | /ndn/edu/ucla/%C1.Router/rt1/NLSR | /ndn/edu/ucla/%C1.Router/rt1/NLSR/KEY/ksk-5/ID-CERT/%01

Users should create keys according to the naming and signing hierarchies above.

## Certificate Publishing

In a network, every router should have the root certificate configured as a trust anchor.
For each site, at least one router should publish the site certificate, at least one router should publish the certificate of the site operator.
Each router should publish its own certificate.
All this information should be explicitly specified in nlsr.conf file.
For example, the following configuration file indicates that NLSR should publish the site certificate and the router certificate:

    ...
    security
    {
      validator
      {
      ...
      }
      cert-to-publish "site.cert"   ; name of the file which contains the site certificate (optional).
      cert-to-publish "router.cert" ; name of the file which contains the router certificate (required).
    }

Note that when a new NLSR instance is created, it will create a new key for the instance; generating the certificate for the new NLSR key is handled internally in NLSR.
Therefore, user does not have to explcitly create keys and certificate for each NLSR instance.

For key and certificate generation, please refer to the [ndnsec tool kit](http://named-data.net/doc/ndn-cxx/0.1.0/manpages/ndnsec.html).

## Example

Security Configuration: Every machine in network will have the same set of security rules to configure.
However, different machine will have different configuration commands "cert-to-publish"
depending on responsibility. Let us use the same topology used for example(https://github.com/named-data/NLSR/blob/master/docs/TOPOLOGY.md).

ndnsec-key-gen generates a key

Lets assume that `/ndn/memphis.edu/router1` is the router who is responsible as the memphis
site router and also responsible as root which is the trust anchor of the network. So
router in memphis will do the following key creation and signing.

NOTE: The first two steps may not apply to everyone as the root certificate and the site certificate are usually present at a testbed outside the scope of NLSR. These steps will help if you are testing an isolated testbed.

   1. Create root key and self signed certificate with prefis `/ndn`
      $ndnsec-key-gen -n /ndn
      $ndnsec-sign-req /ndn > root.cert
      This root.cert will be configured by "cert-to-publish" commands in nlsr.conf

   2. Generate key for site prefix `/ndn/edu/memphis` and sign it by root key
      $ndnsec-key-gen -n /ndn/edu/memphis > unsigned_site.cert
      $ndnsec-cert-gen -S 20140701000000 -E 20150701000000 -N "University of Memphis"
      -s /ndn -p /ndn/edu/memphis -r unsigned_site.cert > site.cert

   3. Generate key for operator and sign it by memphis site key. Lets assume that
      operator name in Memphis site is ndnuser, so the prefix is `/ndn/edu/memphis/%C1.O.N./ndnuser`
      $ndnsec-key-gen -n /ndn/edu/memphis/%C1.Operator/ndnuser > unsigned_operator.cert
      $ndnsec-cert-gen -S 20140701000000 -E 20150701000000 -N "University of Memphis Operator"
      -s /ndn/edu/memphis -p /ndn/edu/memphis/%C1.Operator/ndnuser
      -r unsigned_operator.cert > operator.cert

   4. Generate key for router and sign it with operator's key
      $ndnsec-key-gen /ndn/edu/memphis/%C1.Router/router1 > unsigned_router.cert
      $ndnsec-cert-gen -S 20140701000000 -E 20150701000000 -N "University of Memphis Router"
     -s /ndn/edu/memphis/%C1.Operator/ndnuser -p /ndn/edu/memphis/%C1.Router/router1
     -r unsigned_router.cert > router.cert


Key creation and signing is done for `/ndn/memphis.edu/router1`. In nlsr.conf
this router will have this lines

    cert-to-publish "root.cert"
    cert-to-publish "site.cert"
    cert-to-publish "operator.cert"
    cert-to-publish "router.cert"

Here is the entire security configuraion of the router1 (As seen in nlsr.conf. Note that only the semi-colons before the certs have been removed, rest is same)

    security
    {
      validator
      {
        rule
        {
          id "NSLR Hello Rule"
          for data
          filter
          {
            type name
            regex ^[^<NLSR><INFO>]*<NLSR><INFO><><>$
          }
          checker
          {
            type customized
            sig-type rsa-sha256
            key-locator
            {
              type name
              hyper-relation
              {
                k-regex ^([^<KEY><NLSR>]*)<NLSR><KEY><ksk-.*><ID-CERT>$
                k-expand \\1
                h-relation equal
                p-regex ^([^<NLSR><INFO>]*)<NLSR><INFO><><>$
                p-expand \\1
              }
            }
          }
        }

        rule
        {
          id "NSLR LSA Rule"
          for data
          filter
          {
            type name
            regex ^[^<NLSR><LSA>]*<NLSR><LSA>
          }
          checker
          {
            type customized
            sig-type rsa-sha256
            key-locator
            {
              type name
              hyper-relation
              {
                k-regex ^([^<KEY><NLSR>]*)<NLSR><KEY><ksk-.*><ID-CERT>$
                k-expand \\1
                h-relation equal
                p-regex ^([^<NLSR><LSA>]*)<NLSR><LSA>(<>*)<><><>$
                p-expand \\1\\2
              }
            }
          }
        }

        rule
        {
          id "NSLR Hierarchy Exception Rule"
          for data
          filter
          {
            type name
            regex ^[^<KEY><%C1.Router>]*<%C1.Router>[^<KEY><NLSR>]*<KEY><ksk-.*><ID-CERT><>$
          }
          checker
          {
            type customized
            sig-type rsa-sha256
            key-locator
            {
              type name
              hyper-relation
              {
	              k-regex ^([^<KEY><%C1.Operator>]*)<%C1.Operator>[^<KEY>]*<KEY><ksk-.*><ID-CERT>$
                k-expand \\1
                h-relation equal
                p-regex ^([^<KEY><%C1.Router>]*)<%C1.Router>[^<KEY>]*<KEY><ksk-.*><ID-CERT><>$
                p-expand \\1
              }
            }
          }
        }

        rule
        {
          id "NSLR Hierarchical Rule"
          for data
          filter
          {
            type name
            regex ^[^<KEY>]*<KEY><ksk-.*><ID-CERT><>$
          }
          checker
          {
            type hierarchical
            sig-type rsa-sha256
          }
        }

        trust-anchor
        {
          type file
          file-name "root.cert"
        }
      }
      cert-to-publish "root.cert" //optional, a file containing the root certificate. only the router
                                  //that is designated to publish root cert needs to specify this
      cert-to-publish "site.cert" //optional, a file containing the root certificate. only the router
                                  //that is designated to publish site cert need to specify this
      cert-to-publish "operator.cert" //optional, a file containing the root certificate. only the
                                      //router that is designated to publish operator cert need to
                                      //specify this
      cert-to-publish "router.cert" //required, a file containing the router certificate.
    }

Now lets see how `/ndn/colostate.edu/router2` will create and sign keys. Let assume
that this router is responsible for hosting site keys and operator keys of colorado
site.

   2. Generate key for site prefix `/ndn/edu/colorado`
      $ndnsec-key-gen -n /ndn/edu/memphis > unsigned_site.cert

      Send this cert to Memphis site ( as in example Memphis is the root) and get
      it signed by root. After you get back the site please put it in your convenient
      certificate directory.

   3. Generate key for operator and sign it by colorado site key. Lets assume that
      operator name in Colorado site is testop, so the prefix is `/ndn/edu/colorado/%C1.O.N./testop`
      $ndnsec-key-gen -n /ndn/edu/colorado/%C1.Operator/testop > unsigned_operator.cert
      $ndnsec-cert-gen -S 20140701000000 -E 20150701000000 -N "Colorado State University Operator"
      -s /ndn/edu/colorado -p /ndn/edu/colorado/%C1.Operator/testop
      -r unsigned_operator.cert > operator.cert

   4. Generate key for router and sign it with operator's key
      $ndnsec-key-gen /ndn/edu/colorado/%C1.Router/router2 > unsigned_router.cert
      $ndnsec-cert-gen -S 20140701000000 -E 20150701000000 -N "Colorado State University Router"
      -s /ndn/edu/colorado/%C1.Operator/testop -p /ndn/edu/colorado/%C1.Router/router2
      -r unsigned_router.cert > router.cert


Key creation and signing is done for `/ndn/edu/colorado/router2`. In nlsr.conf
this router will have these lines

    cert-to-publish "site.cert"
    cert-to-publish "operator.cert"
    cert-to-publish "router.cert"

(The rest of the security configuration is same as the other router.)

Colorado also need to bring the root.cert from Memphis and put it in the certificate
directory as it is configured as trust anchor.

Now both Memphis and Colorado router is ready to run with security configuration.
Arizona router will follow the same step as Colorado to configure security for NLSR.


# Disabling Security in NLSR

To disable packet validation in NLSR, you need to change the configuration file
in section: security.validator.trust-anchor

    trust-anchor
    {
      type file
      file-name "root.cert"
    }

change the previous section in security configuration to below

    trust-anchor
    {
      type any
    }

With this change, packets are still signed, but validation will be skipped.
