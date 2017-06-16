#!/usr/bin/env python

import sys
import heapq
import numpy as np
import argparse as ap
import itertools as it

from os import path
from summe import *

''' PATHS ''' 
HOMEDATA='GT/';
SUMMARYDATA='summary/';

class priority_queue:
    def __init__(self):
        self.pq = []                         # list of entries arranged in a heap
        self.entry_finder = {}               # mapping of tasks to entries
        self.REMOVED = '<removed-item>'      # placeholder for a removed item
        self.counter = it.count()            # unique sequence count

    def add(self, item, priority=0):
        'Add a new item or update the priority of an existing item'
        if item in self.entry_finder:
            self.remove(item)
        count = next(self.counter)
        entry = [priority, count, item]
        self.entry_finder[item] = entry
        heapq.heappush(self.pq, entry)

    def remove(self, item):
        'Mark an existing item as REMOVED.  Raise KeyError if not found.'
        entry = self.entry_finder.pop(item)
        entry[-1] = self.REMOVED

    def empty(self):
        return len(self.entry_finder) == 0
        
    def pop(self):
        'Remove and return the lowest priority item. Raise KeyError if empty.'
        while self.pq:
            priority, count, item = heapq.heappop(self.pq)
            if item is not self.REMOVED:
                del self.entry_finder[item]
                return (item, priority)
        raise KeyError('pop from an empty priority queue')
   
def strip_end(text, suffix):
    if not text.endswith(suffix):
        raise AssertionError('filename does not ends with %s' % suffix)
    return text[:len(text)-len(suffix)]
        
def update_progress(cur_val, end_val, bar_length=40):
	percent = float(cur_val) / end_val
	hashes = '#' * int(round(percent * bar_length))
	spaces = ' ' * (bar_length - len(hashes))
	sys.stderr.write("\rProgress: [{0}] {1}% = {2} / {3}".format(hashes + spaces, int(round(percent * 100)), cur_val, end_val))
	sys.stderr.flush()

parser = ap.ArgumentParser()
parser.add_argument('filenames', metavar='file', type=str, nargs='+',
                   help='input file with a video summary')
parser.add_argument('-s', '--show', dest='show', action='store_true', default=False,
                   help='show the images on the screen')
parser.add_argument('-k', '--keyframes', dest='keyframes', action='store_true', default=False,
                   help='input is a list with keyframes')
parser.add_argument('-ext', metavar='extension', type=str, dest='extension', action='store', default=False, required=True, 
                   help='input the extension of the files')
args = parser.parse_args()

for videoFile in args.filenames:
    print "File: {0}".format(path.basename(videoFile))
    videoName = strip_end(path.basename(videoFile), args.extension)
    videoSummary = np.genfromtxt(videoFile, dtype=int)
    
    #In this example we need to do this to now how long the summary selection needs to be
    gt_file=HOMEDATA+'/'+videoName+'.mat'
    if not path.isfile(gt_file):
        raise IOError('%s is not a video from the SumMe dataset' % videoName)
        
    gt_data = scipy.io.loadmat(gt_file)
    nFrames=gt_data.get('nFrames')
    
    '''Example summary vector'''
    #selected frames set to n (where n is the rank of selection) and the rest to 0
    summary_selections={};
    
    if args.keyframes:
        summary_selections[0]=nFrames[0][0] * np.ones(nFrames)
        summary_selections[0][videoSummary] = 1        
    
        pq = priority_queue()
        for keyFrame in videoSummary:
            pq.add(keyFrame, 1)
    
        while not pq.empty():
            frame, rank = pq.pop()
            if frame > 0 and summary_selections[0][frame - 1] > rank + 1:
                summary_selections[0][frame - 1] = rank + 1
                pq.add(frame - 1, rank + 1)
            if frame < nFrames[0][0] - 1 and summary_selections[0][frame + 1] > rank + 1:
                summary_selections[0][frame + 1] = rank + 1
                pq.add(frame + 1, rank + 1)
        summary_selections[0]=map(lambda q: (round(q) if (q <= np.percentile(summary_selections[0],15)) else 0),summary_selections[0])
    else:
        summary_selections[0] = videoSummary        

    '''Evaluate'''
    #get f-measure at 15% summary length
    [f_measure,summary_length]=evaluateSummary(summary_selections[0],videoName,HOMEDATA)
    print('F-measure : %.3f at length %.2f' % (f_measure, summary_length))
    
    #saving fmeasure result in a '.res' output file
    result = open(SUMMARYDATA+videoName+'.res', 'w')
    #result.write('F-measure\t\tSummary length\n')
    result.write('%.3f %.2f' % (f_measure, summary_length))
    result.close()

    if args.show:
        '''plotting'''
        methodNames={'computer method'};
        plotAllResults(summary_selections,methodNames,videoName,HOMEDATA);
