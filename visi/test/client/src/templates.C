#include "common/h/String.h"
#pragma implementation "Vector.h"
#include "common/h/Vector.h"
#pragma implementation "Queue.h"
#include "common/h/Queue.h"
#pragma implementation "visi.xdr.h"
#include "visi.xdr.h"


template class pdvector<T_visi::buf_struct*>;
//template class queue<T_visi::buf_struct*>;
template class pdvector<T_visi::dataValue>;
template class pdvector<T_visi::visi_matrix>;
template class pdvector<T_visi::phase_info>;
template class pdvector<float>;
template class pdvector<string>;
template bool_t T_visi_P_xdr_stl(XDR*, pdvector<string>*,
	bool_t (*)(XDR*, string*), string*);
template bool_t T_visi_P_xdr_stl(XDR*, pdvector<T_visi::dataValue>*,
	bool_t (*)(XDR*, T_visi::dataValue*), T_visi::dataValue*);
template bool_t T_visi_P_xdr_stl(XDR*, pdvector<T_visi::visi_matrix>*,
	bool_t (*)(XDR*, T_visi::visi_matrix*), T_visi::visi_matrix*);
template bool_t T_visi_P_xdr_stl(XDR*, pdvector<T_visi::phase_info>*,
	bool_t (*)(XDR*, T_visi::phase_info*), T_visi::phase_info*);
template bool_t T_visi_P_xdr_stl(XDR*, pdvector<float>*,
	bool_t (*)(XDR*, float*), float*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, pdvector<string>**,
	bool_t (*)(XDR*, string*), string*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, pdvector<T_visi::dataValue>**,
	bool_t (*)(XDR*, T_visi::dataValue*), T_visi::dataValue*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, pdvector<T_visi::visi_matrix>**,
	bool_t (*)(XDR*, T_visi::visi_matrix*), T_visi::visi_matrix*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, pdvector<T_visi::phase_info>**,
	bool_t (*)(XDR*, T_visi::phase_info*), T_visi::phase_info*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, pdvector<float>**,
	bool_t (*)(XDR*, float*), float*);


