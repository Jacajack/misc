#!/usr/bin/python3
import sys
import openrazer.client
from PIL import Image

# From stdin
def read_stdin(kbd):
	for line in sys.stdin:
		command = line[0]
		numbers = [int(s) for s in line.split() if s.isdigit()]
		if "w" in line:
			kbd.fx.advanced.draw()
		else:
			row, col, r, g, b = numbers
			kbd.fx.advanced.matrix[row, col] = [r, g, b];

# From image
def read_image(kbd, path):
	im = Image.open(path)
	pix = im.load()
	w, h = im.size
	if (w < 22 or h < 6):
		return
	for y in range(0, 6):
		for x in range(0, 22):
			kbd.fx.advanced.matrix[y, x] = pix[x, y][:3]
	kbd.fx.advanced.draw()
	

# Open first device
devman = openrazer.client.DeviceManager()
kbd = devman.devices[0]

if (len(sys.argv) == 1):
	read_stdin(kbd);
elif (len(sys.argv) >= 2):
	read_image(kbd, sys.argv[1])
	
