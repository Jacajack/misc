#!/usr/bin/python3
import sys
from phue import Bridge, Group

import dbus
import dbus.service
import dbus.mainloop.glib
from gi.repository import GObject as gobject

class HueDbusService(dbus.service.Object):
	def __init__(self):
		self.br = None
		self.room_group = None
		self.scene_num = 0
		self.ROOM_NAME = "J"
		self.SCENE_NAMES = ["Czytanie", "Koncentracja", "Początki", "Disturbia", "Zorza polarna"]
		
	def run(self):
		dbus.mainloop.glib.DBusGMainLoop(set_as_default = True)
		bus_name = dbus.service.BusName("com.jmw.hue", dbus.SessionBus())
		dbus.service.Object.__init__(self, bus_name, "/com/jmw/hue")
		
		self.br = Bridge('hue')
		self.br.connect()
		self.br.get_api()
		self.room_group = Group(self.br, self.ROOM_NAME)
		
		#print("service start")
		gobject.MainLoop().run()
		#print("service dead")
		
	@dbus.service.method("com.jmw.hue.Control", in_signature="", out_signature="")
	def next_scene(self):
		#print("changing scene")
		self.br.run_scene(self.ROOM_NAME, self.SCENE_NAMES[self.scene_num % len(self.SCENE_NAMES)])
		self.scene_num += 1
		
	@dbus.service.method("com.jmw.hue.Control", in_signature="", out_signature="")
	def lights_off(self):
		#print("lights off")
		self.room_group.on = False
		self.scene_num = 0

if __name__ == "__main__":
	HueDbusService().run()
