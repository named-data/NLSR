NLSR version 0.4.1
++++++++++++++++++

**New features**:

- The routing table dataset publisher (:issue:`3630`)

- Use ndn-cxx logging facility, remove log4cxx dependency (:issue:`3949`)

- Automatic serving of other routers' certificates (:issue:`4448`)

**Bug Fixes**:

- Hard-code a fixed session name for ChronoSync sockets (:issue:`4513`)

- Fix encoding bug in the routing table publisher (:issue:`4453`)

- Fix NLSR not converging without content store (:issue:`4430`)

**Miscellaneous Changes**:

- Make ChronoSync's sync interest lifetime configurable (:issue:`4490`)

- Print labels on adjacency matrix when logging (:issue:`2045`)