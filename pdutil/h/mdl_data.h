/*
 * Copyright (c) 1996-2003 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef _MDL_DATA_H
#define _MDL_DATA_H

// The daemon needs to include this file at times.


#include "dyninstRPC.xdr.h"
#include "pdutil/h/mdl.h"

inline unsigned int ui_hash( const unsigned int &u) { return u; }


class mdl_data {
public:
  dictionary_hash<unsigned, pdvector<mdl_type_desc> > fields;
  pdvector<mdl_focus_element> foci;
  pdvector<T_dyninstRPC::mdl_stmt*> stmts;
  pdvector<T_dyninstRPC::mdl_constraint*> all_constraints;
  pdvector<pdstring> lib_constraints;
  pdvector<unsigned> lib_constraint_flags;
  pdvector<T_dyninstRPC::mdl_metric*> all_metrics;
  mdl_env* env;


    mdl_data( void )
    : fields( ui_hash ),
      env( new mdl_env )
    { }

    virtual ~mdl_data( void )
    {
        delete env;
        env = NULL;
    }


    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( int int_literal );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( pdstring a_str,
                                                    bool is_literal );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( T_dyninstRPC::mdl_expr* expr,
                                             pdvector<pdstring> fields );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( pdstring func_name,
                                     pdvector<T_dyninstRPC::mdl_expr*>* args );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( u_int bin_op,
                                             T_dyninstRPC::mdl_expr* left,
                                             T_dyninstRPC::mdl_expr* right );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( pdstring var,
                                            u_int assign_op,
                                            T_dyninstRPC::mdl_expr* expr );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( u_int u_op,
                                            T_dyninstRPC::mdl_expr* expr,
                                            bool is_preop );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( pdstring var,
                                        T_dyninstRPC::mdl_expr* index_expr );
#if READY
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( u_int type,
                                        int intLiteral,
                                        pdstring strLiteral,
                                        pdstring var,
                                        u_int binOp,
                                        u_int unOp,
                                        T_dyninstRPC::mdl_expr* leftExpr,
                                        T_dyninstRPC::mdl_expr* rightExpr,
                                        pdvector<T_dyninstRPC::mdl_expr*>* args,
                                        const pdvector<pdstring>& fields,
                                        const pdvector<u_int>& type_walk,
                                        bool useTypeWalk,
                                        bool isOK );
#endif // READY

    virtual T_dyninstRPC::mdl_icode* new_icode( T_dyninstRPC::mdl_expr* if_expr,
                                        T_dyninstRPC::mdl_expr* expr );

    virtual T_dyninstRPC::mdl_list_stmt* new_list_stmt( u_int type,
                                                pdstring ident,
                                                pdvector<pdstring>* elems,
                                                bool is_lib,
                                                pdvector<pdstring>* flavor );

    virtual T_dyninstRPC::mdl_for_stmt* new_for_stmt( pdstring index_name,
                                            T_dyninstRPC::mdl_expr* list_exp,
                                            T_dyninstRPC::mdl_stmt* body );


    virtual T_dyninstRPC::mdl_if_stmt* new_if_stmt(
                                            T_dyninstRPC::mdl_expr* expr,
                                            T_dyninstRPC::mdl_stmt* body );

    virtual T_dyninstRPC::mdl_seq_stmt* new_seq_stmt( 
                                    pdvector<T_dyninstRPC::mdl_stmt*>* stmts );

    virtual T_dyninstRPC::mdl_instr_stmt* new_instr_stmt( u_int pos,
                                    T_dyninstRPC::mdl_expr* point_expr,
                                    pdvector<T_dyninstRPC::mdl_icode*>* i_reqs,
                                    unsigned where_instr,
                                    bool constrained );


    virtual T_dyninstRPC::mdl_constraint* new_constraint(
                            pdstring id,
                            pdvector<pdstring>* path,
                            pdvector<T_dyninstRPC::mdl_stmt*>* stmts,
                            bool replace,
                            u_int data_type);

    virtual bool new_metric(pdstring id,
                        pdstring name,
                        pdstring units,
			            u_int agg,
                        u_int style,
                        u_int type,
                        pdstring hwcntr, 
			            pdvector<T_dyninstRPC::mdl_stmt*>* stmts, 
			            pdvector<pdstring>* flavs,
			            pdvector<T_dyninstRPC::mdl_constraint*>* cons,
		                pdvector<pdstring>* temp_counters,
			            bool developerMode,
			            int normalized);




    // unique_name: prepares for the declaration of a new mdl object,
    // by deleting all objects previously declared that have the same
    // name.
    void unique_name(pdstring name);

    static mdl_data* cur_mdl_data;
};


#endif
