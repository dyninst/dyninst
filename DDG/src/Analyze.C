/*
 * Copyright (c) 2007-2008 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "Annotatable.h"
#include "Analyzer.h"
#include "analyzeDDG.h"
#include "analyzeCDG.h"
#include "analyzeFDG.h"
#include "analyzePDG.h"

#include "Graph.h"

#include "BPatch_function.h"
#include "BPatch_flowGraph.h"
#include "BPatch_basicBlock.h"

using namespace Dyninst;
using namespace Dyninst::DepGraphAPI;
using namespace Dyninst::InstructionAPI;
using namespace dyn_detail::boost;

AnnotationClass <Graph::Ptr> FDGAnno(std::string("FDGAnno"));
AnnotationClass <Graph::Ptr> PDGAnno(std::string("PDGAnno"));



Analyzer::Analyzer(Function *func) : 
    func_(func) {};

Analyzer Analyzer::createAnalyzer(BPatch_function *func) {
    // We need to strip out what we need and create (and return)
    // an Analyzer object. For now, what we need is the image_func
    // since it's our base representation.

    if (!func) return Analyzer();

    return Analyzer(func);
}

DDG::Ptr Analyzer::createDDG() {
    if (func_ == NULL) return DDG::Ptr();

    DDGAnalyzer ddgA(func_);
    return ddgA.analyze();
}

Graph::Ptr Analyzer::createCDG() {
    if (func_ == NULL) return Graph::Ptr();

    CDGAnalyzer cdgA(func_);
    return cdgA.analyze();
}


Graph::Ptr Analyzer::createFDG() {
    if (func_ == NULL) return Graph::Ptr();

    FDGAnalyzer fdgA(func_);
    return fdgA.analyze();
    
}


Graph::Ptr Analyzer::createPDG() {
    if (func_ == NULL) return Graph::Ptr();

    // Common code: if we already have the FDG as an annotation, 
    // return it.
    
    Graph::Ptr *ret;
    func_->getAnnotation(ret, FDGAnno);
    if (ret) return *ret;

    // Perform analysis

    // The PDG is the union of the CDG, DDG, and FDG...
    DDG::Ptr DDG = createDDG();
    Graph::Ptr CDG = createCDG();
    Graph::Ptr FDG = createFDG();

    PDGAnalyzer pdgCreator(DDG, CDG, FDG);
    Graph::Ptr PDG = pdgCreator.analyze();

    // Failure...
    if (PDG == NULL) return PDG;

    // Store as an annotation and return
    Graph::Ptr *ptr = new Graph::Ptr(PDG);
    func_->addAnnotation(ptr, PDGAnno);

    return PDG;
}



