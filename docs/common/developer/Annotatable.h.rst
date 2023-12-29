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

  .. cpp:function:: template<class T> bool addAnnotation(const T *a, AnnotationClass<T> &a_id)
  .. cpp:function:: AnnotatableDense(const AnnotatableDense&) = default
  .. cpp:function:: AnnotatableDense()
  .. cpp:function:: template<class T> inline bool getAnnotation(T *&a, AnnotationClass<T> &a_id) const
  .. cpp:function:: template<class T> inline bool removeAnnotation(AnnotationClass<T> &a_id)
  .. cpp:function:: void annotationsReport()

.. cpp:class:: AnnotatableSparse

  .. cpp:type:: dyn_hash_map<void *, void *, void_ptr_hasher> annos_by_type_t
  .. cpp:type:: std::vector<annos_by_type_t *> annos_t
  .. cpp:function:: AnnotatableSparse() = default
  .. cpp:function:: AnnotatableSparse(const AnnotatableSparse&) = default
