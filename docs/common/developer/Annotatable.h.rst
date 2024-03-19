.. _`sec:Annotatable.h`:

Annotatable.h
#############

.. cpp:namespace:: Dyninst

.. cpp:function:: bool annotation_debug_flag()

.. cpp:type:: unsigned short AnnotationClassID
.. cpp:type:: bool (*anno_cmp_func_t)(void *, void*)

.. cpp:class:: AnnotationClassBase

  .. cpp:function:: protected AnnotationClassBase(std::string n, anno_cmp_func_t cmp_func_ = NULL)
  .. cpp:function:: protected virtual ~AnnotationClassBase()
  .. cpp:function:: static AnnotationClassBase *findAnnotationClass(unsigned int id)
  .. cpp:function:: static void dumpAnnotationClasses()
  .. cpp:function:: AnnotationClassID getID()
  .. cpp:function:: std::string &getName()
  .. cpp:function:: anno_cmp_func_t getCmpFunc()
  .. cpp:function:: virtual const char *getTypeName() = 0
  .. cpp:function:: virtual void *allocate() = 0

.. cpp:class:: template <class T> AnnotationClass : public AnnotationClassBase

  .. cpp:function:: AnnotationClass(std::string n, anno_cmp_func_t cmp_func_ = NULL)
  .. cpp:function:: const char *getTypeName()
  .. cpp:function:: void *allocate()
  .. cpp:function:: size_t size()

.. cpp:enum:: sparse_or_dense_anno_t

  .. cpp:enumerator:: sparse
  .. cpp:enumerator:: dense

.. cpp:struct:: ser_rec_t

  .. cpp:member:: AnnotationClassBase *acb
  .. cpp:member:: void *data
  .. cpp:member:: void *parent_id
  .. cpp:member:: sparse_or_dense_anno_t sod

.. cpp:enum:: ser_post_op_t

  .. cpp:enumerator:: sp_add_anno
  .. cpp:enumerator:: sp_rem_anno
  .. cpp:enumerator:: sp_add_cont_item
  .. cpp:enumerator:: sp_rem_cont_item

.. cpp:function:: const char *serPostOp2Str(ser_post_op_t)

.. cpp:class:: AnnotatableDense

  Inheriting from this class adds a pointer to each object.  Multiple
  types of annotations are stored under this pointer in a
  annotation_type -> anno_list_t map.

  .. cpp:type:: private void *anno_list_t
  .. cpp:type:: private anno_list_t anno_map_t
  .. cpp:member:: private aInfo *annotations
  .. cpp:function:: private bool addAnnotation(const void *a, AnnotationClassID id)
  .. cpp:function:: AnnotatableDense()
  .. cpp:function:: ~AnnotatableDense()
  .. cpp:function:: AnnotatableDense(const AnnotatableDense&) = default
  .. cpp:function:: AnnotatableDense& operator=(const AnnotatableDense& rhs)
  .. cpp:function:: template<class T> bool addAnnotation(const T *a, AnnotationClass<T> &a_id)
  .. cpp:function:: template<class T> inline bool getAnnotation(T *&a, AnnotationClass<T> &a_id) const
  .. cpp:function:: template<class T> inline bool removeAnnotation(AnnotationClass<T> &a_id)
  .. cpp:function:: void annotationsReport()


.. cpp:struct:: AnnotatableDense::aInfo

  .. cpp:member:: anno_map_t *data
  .. cpp:member:: AnnotationClassID max



.. cpp:class:: AnnotatableSparse

  .. cpp:type:: dyn_hash_map<void *, void *, void_ptr_hasher> annos_by_type_t
  .. cpp:type:: std::vector<annos_by_type_t *> annos_t
  .. cpp:function:: AnnotatableSparse() = default
  .. cpp:function:: ~AnnotatableSparse()
  .. cpp:function:: AnnotatableSparse(const AnnotatableSparse &) = default
  .. cpp:function:: AnnotatableSparse &operator=(const AnnotatableSparse &rhs)
  .. cpp:function:: private void ClearAnnotations(const char *reason)

    We need to remove annotations from the static map when objects are destroyed

      1. Memory may be reclaimed and reused at the same place

      2. Regardless of (1), the map can possibly explode to unmanageable sizes, with a lot of unused
         junk in it if a lot of  annotatable objects are created and destroyed.

    Alas this is kinda expensive right now, but the data structure is set up to minimize search time not
    deletion time. It could be changed if this becomes a significant time drain.

  .. cpp:member:: private static annos_t annos
  .. cpp:function:: private annos_t *getAnnos() const
  .. cpp:member:: private static dyn_hash_map<void *, unsigned short> ser_ndx_map
  .. cpp:function:: private annos_by_type_t *getAnnosOfType(AnnotationClassID aid, bool do_create = false) const
  .. cpp:function:: private template <class T> annos_by_type_t *getAnnosOfType(AnnotationClass<T> &a_id, bool do_create = false) const
  .. cpp:function:: private void *getAnnosForObject(annos_by_type_t *abt, void *obj, bool do_create = false) const
  .. cpp:function:: private bool addAnnotation(const void *a, AnnotationClassID aid)
  .. cpp:function:: bool operator==(AnnotatableSparse &cmp)
  .. cpp:function:: template <class T> bool addAnnotation(const T *a, AnnotationClass<T> &a_id)
  .. cpp:function:: template <class T> bool getAnnotation(T *&a, AnnotationClass<T> &a_id) const
  .. cpp:function:: template <class T> inline bool removeAnnotation(AnnotationClass<T> &a_id)
  .. cpp:function:: void annotationsReport()


.. cpp:struct:: AnnotatableSparse::void_ptr_hasher

  .. cpp:function:: size_t operator()(const void *a) const noexcept

