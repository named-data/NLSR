NLSR version 0.4.3
++++++++++++++++++

**Changes**:

- **breaking change** Change NLSR to nlsr for various prefixes (:issue:`4543`)

**Bug Fixes**:

- Fix dry-run mode not working (:issue:`4394`)
- Fix convergence w/o content store (:issue:`4430`)
- Register router prefix to fix remote requests (:issue:`4543`)
- Fix build with clang-5.0 (:issue:`4547`)
- Avoid misaligned memory access (:issue:`4609`)
