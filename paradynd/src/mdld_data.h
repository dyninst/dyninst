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

#ifndef _MDLD_DATA_H
#define _MDLD_DATA_H

#include "pdutil/h/mdl_data.h"


class mdld_data : public mdl_data
{
public:
    virtual ~mdld_data( void )
    { }


    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( int int_literal );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( string a_str,
                                                    bool is_literal );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( T_dyninstRPC::mdl_expr* expr,
                                             pdvector<string> fields );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( string func_name,
                                     pdvector<T_dyninstRPC::mdl_expr*>* args );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( u_int bin_op,
                                             T_dyninstRPC::mdl_expr* left,
                                             T_dyninstRPC::mdl_expr* right );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( string var,
                                            u_int assign_op,
                                            T_dyninstRPC::mdl_expr* expr );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( u_int u_op,
                                            T_dyninstRPC::mdl_expr* expr,
                                            bool is_preop );
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( string var,
                                        T_dyninstRPC::mdl_expr* index_expr );

#if READY
    virtual T_dyninstRPC::mdl_v_expr* new_v_expr( u_int type,
                                        int intLiteral,
                                        string strLiteral,
                                        string var,
                                        u_int binOp,
                                        u_int unOp,
                                        T_dyninstRPC::mdl_expr* leftExpr,
                                        T_dyninstRPC::mdl_expr* rightExpr,
                                        pdvector<T_dyninstRPC::mdl_expr*>* args,
                                        const pdvector<string>& fields,
                                        const pdvector<u_int>& type_walk,
                                        bool useTypeWalk,
                                        bool isOK );
#endif // READY

    virtual T_dyninstRPC::mdl_icode* new_icode( T_dyninstRPC::mdl_expr* if_expr,
                                        T_dyninstRPC::mdl_expr* expr );

    virtual T_dyninstRPC::mdl_list_stmt* new_list_stmt( u_int type,
                                                string ident,
                                                pdvector<string>* elems,
                                                bool is_lib,
                                                pdvector<string>* flavor );

    virtual T_dyninstRPC::mdl_for_stmt* new_for_stmt( string index_name,
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
                            string id,
                            pdvector<string>* path,
                            pdvector<T_dyninstRPC::mdl_stmt*>* stmts,
                            bool replace,
                            u_int data_type);

    virtual bool new_metric(string id,
                        string name,
                        string units,
			            u_int agg,
                        u_int style,
                        u_int type,
                        string hwcntr, 
			            pdvector<T_dyninstRPC::mdl_stmt*>* stmts, 
			            pdvector<string>* flavs,
			            pdvector<T_dyninstRPC::mdl_constraint*>* cons,
		                pdvector<string>* temp_counters,
			            bool developerMode,
			            int normalized);
};


class mdld_expr : virtual public T_dyninstRPC::mdl_expr
{
public:
    virtual bool apply_be( mdl_var& ret ) = NULL;
    virtual bool apply_be( AstNode*& mn ) = NULL;
    virtual bool mk_list( pdvector<string>& funcs ) = NULL;
};


class mdld_v_expr : public mdld_expr,
                    public T_dyninstRPC::mdl_v_expr
{
public:
    mdld_v_expr( int int_literal )
      : mdl_v_expr( int_literal )
    { }

    mdld_v_expr( string a_str, bool is_literal )
      : mdl_v_expr( a_str, is_literal )
    { }

    mdld_v_expr( T_dyninstRPC::mdl_expr* expr, pdvector<string> fields )
      : mdl_v_expr( expr, fields )
    { }

    mdld_v_expr( string func_name, pdvector<T_dyninstRPC::mdl_expr*>* args )
      : mdl_v_expr( func_name, args )
    { }

    mdld_v_expr( u_int bin_op, T_dyninstRPC::mdl_expr *left,
                    T_dyninstRPC::mdl_expr *right )
      : mdl_v_expr( bin_op, left, right )
    { }

    mdld_v_expr( string var, u_int assign_op, T_dyninstRPC::mdl_expr* expr )
      : mdl_v_expr( var, assign_op, expr )
    { }

    mdld_v_expr( u_int u_op, T_dyninstRPC::mdl_expr *expr, bool is_preop )
      : mdl_v_expr( u_op, expr, is_preop )
    { }

    mdld_v_expr( string var, T_dyninstRPC::mdl_expr *index_expr )
      : mdl_v_expr( var, index_expr )
    { }

#if READY
    mdld_v_expr( u_int type,
                int intLiteral,
                string strLiteral,
                string var,
                u_int binOp,
                u_int unOp,
                T_dyninstRPC::mdl_expr* leftExpr,
                T_dyninstRPC::mdl_expr* rightExpr,
                pdvector<T_dyninstRPC::mdl_expr*>* args,
                const pdvector<string>& fields,
                const pdvector<u_int>& type_walk,
                bool useTypeWalk,
                bool isOK )
      : mdl_v_expr( type,
                intLiteral,
                strLiteral,
                var,
                binOp,
                unOp,
                leftExpr,
                rightExpr,
                args,
                fields,
                type_walk,
                useTypeWalk,
                isOK )
    { }
#endif // READY

    virtual ~mdld_v_expr( void ) { }
    virtual bool apply_be( mdl_var& ret );
    virtual bool apply_be( AstNode*& mn );
    virtual bool mk_list( pdvector<string>& funcs );
};


class mdld_icode : public T_dyninstRPC::mdl_icode
{
public:
    mdld_icode( T_dyninstRPC::mdl_expr* if_expr, 
                T_dyninstRPC::mdl_expr* expr )
      : mdl_icode( if_expr, expr )
    { }

    virtual ~mdld_icode( void ) { }
    virtual bool apply_be( AstNode*& mn,
                            bool mn_initialized,
                            pd_process* proc );
};



class mdld_stmt : virtual public T_dyninstRPC::mdl_stmt
{
public:
    virtual bool apply_be( instrCodeNode* mn,
                            pdvector<const instrDataNode*>& flags ) = NULL;
    virtual bool mk_list( pdvector<string>& funcs ) = NULL;
};


class mdld_list_stmt : public mdld_stmt,
                        public T_dyninstRPC::mdl_list_stmt
{
public:
    mdld_list_stmt( u_int type,
                     string ident,
                     pdvector<string>* elems,
                     bool is_lib,
                     pdvector<string>* flavor )
      : mdl_list_stmt( type,
                        ident,
                        elems,
                        is_lib,
                        flavor )
    { }

    virtual ~mdld_list_stmt( void ) { }
    virtual bool apply_be( instrCodeNode* mn,
                            pdvector<const instrDataNode*>& flags );
    virtual bool mk_list( pdvector<string>& funcs );
};


class mdld_for_stmt : public virtual mdld_stmt,
                        public virtual T_dyninstRPC::mdl_for_stmt
{
public:
    mdld_for_stmt( string index_name,
                    T_dyninstRPC::mdl_expr* list_exp,
                    T_dyninstRPC::mdl_stmt* body )
      : mdl_for_stmt( index_name, list_exp, body )
    { }

    virtual ~mdld_for_stmt( void ) { }
    virtual bool apply_be( instrCodeNode* mn,
                            pdvector<const instrDataNode*>& flags );
    virtual bool mk_list( pdvector<string>& funcs );
};


class mdld_if_stmt : public virtual mdld_stmt,
                        public virtual T_dyninstRPC::mdl_if_stmt
{
public:
    mdld_if_stmt( T_dyninstRPC::mdl_expr* expr,
                    T_dyninstRPC::mdl_stmt* body )
      : mdl_if_stmt( expr, body )
    { }

    virtual ~mdld_if_stmt( void ) { }
    virtual bool apply_be(instrCodeNode* mn,
                            pdvector<const instrDataNode*>& flags );
    virtual bool mk_list( pdvector<string>& funcs );
};


class mdld_seq_stmt : public virtual mdld_stmt,
                        public virtual T_dyninstRPC::mdl_seq_stmt
{
public:
    mdld_seq_stmt( pdvector<T_dyninstRPC::mdl_stmt*>* stmts )
      : mdl_seq_stmt( stmts )
    { }

    virtual ~mdld_seq_stmt( void ) { }
    virtual bool apply_be( instrCodeNode* mn,
                            pdvector<const instrDataNode*>& flags );
    virtual bool mk_list( pdvector<string>& funcs );
};


class mdld_instr_stmt : public virtual mdld_stmt,
                            public virtual T_dyninstRPC::mdl_instr_stmt
{
public:
    mdld_instr_stmt( u_int pos,
                        T_dyninstRPC::mdl_expr* point_expr,
                        pdvector<T_dyninstRPC::mdl_icode*>* i_reqs,
                        unsigned where_instr,
                        bool constrained )
      : mdl_instr_stmt( pos, point_expr, i_reqs, where_instr, constrained )
    { }

    virtual ~mdld_instr_stmt( void ) { }
    virtual bool apply_be(instrCodeNode* mn,
                            pdvector<const instrDataNode*>& flags );
    virtual bool mk_list( pdvector<string>& funcs );
};


class mdld_constraint : public T_dyninstRPC::mdl_constraint
{
public:
    mdld_constraint( string id,
                        pdvector<string>* match_path,
                        pdvector<T_dyninstRPC::mdl_stmt*>* stmts,
                        bool replace,
                        u_int d_type,
                        bool& error )
      : mdl_constraint( id,
                        match_path,
                        stmts,
                        replace,
                        d_type,
                        error )
    { }

    mdld_constraint( string id,
                        pdvector<string>* match_path,
                        pdvector<T_dyninstRPC::mdl_stmt*>* stmts,
                        bool replace,
                        u_int d_type,
                        u_int hierarchy,
                        u_int type )
      : mdl_constraint( id,
                        match_path,
                        stmts,
                        replace,
                        d_type,
                        hierarchy,
                        type )
    { }
        
    virtual ~mdld_constraint( void ) { }
    virtual bool apply_be( instrCodeNode* codeNode,
                            instrDataNode** dataNode,
                            const Hierarchy& resource,
                            pd_process* proc,
                            bool dontInsertData );

    virtual bool mk_list( pdvector<string>& funcs );
};


class mdld_metric : public T_dyninstRPC::mdl_metric
{
public:
    mdld_metric( string id,
                    string name,
                    string units,
                    u_int agg,
                    u_int style,
                    u_int type,
                    string hwcntr,
                    pdvector<T_dyninstRPC::mdl_stmt*>* mv,
                    pdvector<string>* flavs,
                    pdvector<T_dyninstRPC::mdl_constraint*>* cons,
                    pdvector<string>* temp_c,
                    bool developerMode,
                    int unitstype )
      : mdl_metric( id,
                    name,
                    units,
                    agg,
                    style,
                    type,
                    hwcntr,
                    mv,
                    flavs,
                    cons,
                    temp_c,
                    developerMode,
                    unitstype )
    { }

    virtual ~mdld_metric( void ) { }
    virtual bool apply_be( pdvector<processMetFocusNode*>* createdProcNodes,
                            const Focus& focus,
                            pdvector<pd_process*> procs,
                            bool replace_components_if_present,
                            bool dontInsertData );
#if READY
    virtual bool mk_list( pdvector<string>& funcs );
#endif // READY
};


#endif // _MDLD_DATA
