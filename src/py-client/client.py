#! /usr/bin/python

import socket
import sys
import time
import struct
import encodings
from Tkinter import *
from threading import Thread

class proto:
	def convert_to_send(self, typ, data):
		ret = self.add_layer_2(typ, data)
		ret = self.add_layer_1(ret)
		ret = self.add_layer_0(ret)
		return ret

	def convert_to_resv(self, data):
		pass

	def encode_uft32_no_bom_be(self, text):
		ret = []
		for c in text:
			ret.append(chr((ord(c)>>24)&0x000000ff))
			ret.append(chr((ord(c)>>16)&0x000000ff))
			ret.append(chr((ord(c)>>8)&0x000000ff))
			ret.append(chr((ord(c)>>0)&0x000000ff))
		return ''.join(ret)
	
	def encode_uft32_no_bom_le(self, text):
		ret = []
		for c in text:
			ret.append(chr((ord(c)>>0)&0x000000ff))
			ret.append(chr((ord(c)>>8)&0x000000ff))
			ret.append(chr((ord(c)>>16)&0x000000ff))
			ret.append(chr((ord(c)>>24)&0x000000ff))
		return ''.join(ret)



	def add_layer_0(self, data):
		size = len(data)
		format = "4B%ds"%size #0x80,size(short),0x00,data
		ret = struct.pack(format,0x80,((size+4))&0x00ff,(size+4>>8)&0x00ff,0x00,data)
		return ret

	def add_layer_1(self,data):
		return data

	def add_layer_2(self, typ, data):
		ret = ""
		if(typ == "utf-32_be"):
			ret = self.encode_uft32_no_bom_be(data)
		elif(typ == "utf-32_le"):
			ret = self.encode_uft32_no_bom_le(data)
		elif(typ == "uft-8"):
			ret = data.encode('uft-8')
		elif(typ =="binary"):
			pass

		return ret


class client:

	def __init__(self):
		self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)


	def connect(self, host, port):
		try:
			self.s.connect((host, port))
		except socket.error, msg:
			self.s.close()
			print msg
			return 0
		return 1


	def send_text(self, text):
		p = proto()
		to_send = p.convert_to_send("utf-32_le",text)
		self.send(to_send)


	def send(self, data):
		f = self.s.makefile('w+')
		f.write(data)

	def start_recv(self):
		self.stop = 0
		self.recv_thread = Thread(target = self.recv).start()

	def stop_recv(self):
		self.stop = 1

	def recv(self):
		self.s.setblocking(0)
		while not self.stop:
			try:
				data = self.s.recv(1024)
				self.g.out.insert(END,">>"+data[4::4]+"\n","a")
			except:
				time.sleep(0.1)
		print "recv thread stop"


	def close(self):
		self.s.close()


	def run(self):
		host = raw_input("host(localhost):")
		if(host==""):
			host="localhost"

		port = raw_input("port(34443):")
		if(port==""):
			port = 34443
		else:
			port = ord(port)

		self.user = raw_input("user:")
		if(self.user==""):
			self.user = "nobody"
		if(self.connect(host,port)):
			print "connected"
			self.g = gui_client()
			self.g.input.bind("<Return>",self.send_gui)
			print "instance gui"
			self.start_recv()
			print "recv start"
			self.g.run()
			self.stop_recv()
			self.close()

	def send_gui(self,event):
		text = self.user + "@python-client:" + self.g.input.get() 
		self.g.out.insert(END,">>"+text+"\n","a")
		self.send_text(text)
		self.g.input.delete(0,END)



class gui_client:
	def __init__(self):
		self.root=Tk()
		self.frame = Frame(self.root)
		self.frame.pack(fill=BOTH, expand=1)
		self.out = Text(self.frame)
		self.out.pack(expand=1, fill=BOTH)
		self.input = Entry(self.frame)
		self.input.pack(fill=X)

	def run(self):
		self.root.mainloop()


c = client()
try:
	c.run()
except:
	c.stop_recv()
	c.close()
