#!/usr/bin/python
import re
import sys
import math
from pcbnew import *

filename = sys.argv[1]
pcb = LoadBoard(filename)

def expand_si(s):
	m = re.search('\s*([0-9\.]+)\s*([pnumkM])', s)
	if not m:
		return 0
	number = m.group(1)
	multable = {'p': -12, 'n': -9, 'µ': -6, 'u': -6, 'm': -3, 'k': 3, 'M': 6}
	if len(m.groups()) < 2:
		return float(number)
	else:
		return float(number) * math.pow(10, multable[m.group(2)])

def module_convert(km):
	ref = km.GetReference()
	ref_match = re.search('([A-Z]+)(.*)', ref)
	ref_class = ref_match.group(1)
	ref_id = ref_match.group(2)
	val = km.GetValue()
	
	m = {
		'pos': ToMM(km.GetPosition()),
		'ref': ref,
		'val': val,
		'flipped': km.IsFlipped(),
		'ref_class': ref_class,
		'ref_id': ref_id,
		#'fp': km.GetPath(),
		'real_val': expand_si(val)
	}
	return m

def module_key(m):
	vertical_lane = 30
	return m['flipped'], m['ref_class'], m['real_val'], m['val'], m['pos'][0], math.floor(m['pos'][1] / vertical_lane) * vertical_lane
	

modules = list(map(module_convert, pcb.GetModules()))
modules.sort(key = module_key)

last_ref_class = ''
for m in modules:
	if m['ref_class'] != last_ref_class:
		print("#", m['ref_class'], sep = '\t')
	print('B' if m['flipped'] else 'F', m['ref_class'], m['ref_id'], m['ref'], "{0:0.2e}".format(m['real_val']), m['val'], m['pos'][0], m['pos'][1], sep = '\t')
	last_ref_class = m['ref_class']


