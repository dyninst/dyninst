.. _`sec:Type-mem.h`:

Type-mem.h
##########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:function:: template<class T> T *upgradePlaceholder(Type *placeholder, T *new_type)

.. cpp:namespace-push:: dev

.. cpp:function:: template<class T> typename boost::enable_if<\
                    boost::integral_constant<bool, !bool(boost::is_same<Type, T>::value)>, \
                    boost::shared_ptr<Type>>::type \
                    typeCollection::addOrUpdateType(boost::shared_ptr<T> type)

  Instantiating this function for ``Type`` would be a mistake, which
  the following assert tries to guard against.  If you trigger this,
  then a caller to this function is likely using ``Type``.  Change
  this to a more specific call, e.g. :cpp:class:`typeFunction` instead of ``Type``.

.. cpp:namespace-pop::
