#include "common/h/String.h"
#pragma implementation "Vector.h"
#include "common/h/Vector.h"
#pragma implementation "Queue.h"
#include "common/h/Queue.h"
#pragma implementation "visi.xdr.h"
#include "visi.xdr.h"


template class vector<T_visi::buf_struct*>;
//template class queue<T_visi::buf_struct*>;
template class vector<T_visi::dataValue>;
template class vector<T_visi::visi_matrix>;
template class vector<T_visi::phase_info>;
template class vector<float>;
template class vector<string>;
template bool_t T_visi_P_xdr_stl(XDR*, vector<string>*,
	bool_t (*)(XDR*, string*), string*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<T_visi::dataValue>*,
	bool_t (*)(XDR*, T_visi::dataValue*), T_visi::dataValue*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<T_visi::visi_matrix>*,
	bool_t (*)(XDR*, T_visi::visi_matrix*), T_visi::visi_matrix*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<T_visi::phase_info>*,
	bool_t (*)(XDR*, T_visi::phase_info*), T_visi::phase_info*);
template bool_t T_visi_P_xdr_stl(XDR*, vector<float>*,
	bool_t (*)(XDR*, float*), float*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<string>**,
	bool_t (*)(XDR*, string*), string*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<T_visi::dataValue>**,
	bool_t (*)(XDR*, T_visi::dataValue*), T_visi::dataValue*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<T_visi::visi_matrix>**,
	bool_t (*)(XDR*, T_visi::visi_matrix*), T_visi::visi_matrix*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<T_visi::phase_info>**,
	bool_t (*)(XDR*, T_visi::phase_info*), T_visi::phase_info*);
template bool_t T_visi_P_xdr_stl_PTR(XDR*, vector<float>**,
	bool_t (*)(XDR*, float*), float*);


