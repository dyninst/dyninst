/*
 * Copyright (c) 1993, 1994 Barton P. Miller, Jeff Hollingsworth,
 *     Bruce Irvin, Jon Cargille, Krishna Kunchithapadam, Karen
 *     Karavanic, Tia Newhall, Mark Callaghan.  All rights reserved.
 *
 * This software is furnished under the condition that it may not be
 * provided or otherwise made available to, or used by, any other
 * person, except as provided for by the terms of applicable license
 * agreements.  No title to or ownership of the software is hereby
 * transferred.  The name of the principals may not be used in any
 * advertising or publicity related to this software without specific,
 * written prior authorization.  Any use of this software must include
 * the above copyright notice.
 *
 */
#include "DMphase.h"
#include "DMperfstream.h"
#include "DMmetric.h"
#include "util/h/hist.h"

phaseInfo::phaseInfo(timeStamp s, timeStamp e, timeStamp b, const string &n){

    startTime = s;
    endTime = e;
    bucketWidth = b;
    handle = dm_phases.size();
    if(!n.string_of()){
        char temp[20];
        sprintf(temp,"%s%d","phase_",handle);
        name = string(temp);
    }
    else
        name = n;
    phaseInfo *p = this;
    dm_phases += p;
}

void phaseInfo::startPhase(timeStamp start_Time, const string &name){
    // update the histogram data structs assoc with each MI
    // return a start time for the phase

    // create a new phaseInfo object 
    timeStamp bin_width = (Histogram::bucketSize);

    // **** kludge: the PC search screws up the lastGlobalBin value in
    // the Histogram class, so search all active DM histograms for
    // the lastBucket value.  when the PC is fixed replace with  
    // this:   timeStamp start_time = bin_width*(Histogram::lastGlobalBin);

    dictionary_hash_iter<metricInstanceHandle,metricInstance *> 
			allMIs(metricInstance::allMetricInstances);
    metricInstanceHandle mi_h; metricInstance *next = 0;
    int lastBin = Histogram::numBins;
    while(allMIs.next(mi_h,next)){
        if(next->data){
            if(next->data->getCurrBin() < lastBin){
		lastBin = next->data->getCurrBin();
    }}}
    timeStamp start_time = bin_width*lastBin;
    // ****



    phaseInfo *p = new phaseInfo(start_time, (timeStamp)-1.0, bin_width, name);
    // invoke newPhaseCallback for all perf. streams
    dictionary_hash_iter<perfStreamHandle,performanceStream*>
			allS(performanceStream::allStreams);
     perfStreamHandle h;
     performanceStream *ps;
     while(allS.next(h,ps)){
         ps->callPhaseFunc(*p);
     }
     p = 0;
}
