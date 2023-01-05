NLSR version 0.6.0
------------------

Improvements and bug fixes
^^^^^^^^^^^^^^^^^^^^^^^^^^

- *(breaking change)* Major changes in the TLV structure and classes. Reduced codebase size and
  improved compilation times. LSA de/serialize functions are replaced by wireEncode/Decode.
  Updated LSA wire formats and TLV assignments. Updated nlsrc to print using the new encoding.
  (:issue:`4787`, :issue:`5116`)
- LSDB refactor: switch to ``boost::multi_index`` to replace 3 LSA lists (:issue:`4127`)
- Fix wrong reaction on hello Interest timeout (:issue:`5139`)
