namespace boost { 
  template<class T> class shared_ptr { T *ptr; }; 
  template<class T> class weak_ptr { T *ptr; }; 
}

namespace Sync {
  struct DiffStateContainer { DiffState *multi_index_container; };
  struct LeafContainer { Leaf *multi_index_container; }
  struct CcnxWrapperPtr { CcnxWrapper *ptr; }
}
