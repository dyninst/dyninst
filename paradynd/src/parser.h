/*
 *  Copyright 1993 Jeff Hollingsworth.  All rights reserved.
 *
 */

/*
 * parser.h
 *
 * $Log: parser.h,v $
 * Revision 1.1  1994/01/27 20:31:33  hollings
 * Iinital version of paradynd speaking dynRPC igend protocol.
 *
 * Revision 1.2  1993/10/19  15:27:54  hollings
 * AST based mini-tramp code generator.
 *
 * Revision 1.1  1993/03/19  22:51:05  hollings
 * Initial revision
 *
 *
 */

struct parseStack {
    int                 i;
    double              f;
    char                *cp;
    resource		r;
    resourceList	rl;
    opCode		op;
    AstNode		*ast;
    dataReqNode		*dp;
};

#define YYSTYPE struct parseStack
