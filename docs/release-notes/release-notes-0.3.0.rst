NLSR version 0.3.0 (changes since version 0.2.2)
++++++++++++++++++++++++++++++++++++++++++++++++

Release date:  August 1, 2016

**Code changes**:

- Updated validation checker for LSA rule in the configuration file (:issue:`2965`)

- Do not advertise Adjacency LSAs when hyperbolic routing is enabled (:issue:`2402`)

- Do not distribute coordinate LSAs when using link-state routing. (:issue:`3661`)

- LSAs should be segmented before they are published (:issue:`2965`)

- Don't use DummyClientFace::sentDatas (:issue:`3383`)

- Remove disabled security files (:issue:`2960`)

- Lsa type should be argument for base class LSA constructor (:issue:`2788`)

- Use ndn-cxx/DummyClientFace (:issue:`2637`)

**Bug fixes**:

- Unit tests cases are installed with `./waf install` (:issue:`3359`)