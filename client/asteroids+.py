#!/usr/bin/python

import socket
from socket import AF_INET, SOCK_DGRAM
import struct
import sys
import time
from math import sin, cos
from vec2d import Vec2d


### pygame

import pygame
from pygame.locals import *
pygame.init()
#screen = pygame.display.set_mode((0,0), FULLSCREEN)
screen = pygame.display.set_mode((640, 480))
screen_size = screen.get_size()
pygame.display.set_caption('Asteroids+')
#pygame.mouse.set_visible(0)


### protocol

msg_keysdown = 2
msg_status   = 7
msg_ping     = 17


### network

host = sys.argv[1] if len(sys.argv) > 1 else "localhost"
port = sys.argv[2] if len(sys.argv) > 2 else 50005

server = socket.socket(AF_INET,SOCK_DGRAM)
server.setblocking(0)
buffer_size = 102400

print 'Server at %s:%d' % (host, port)


### globals

objects = []
keystate = {}

def draw():
	global objects
	screen.fill((0, 0, 0))
	for obj in objects:
		pos = Vec2d(obj['x'] + 50, -obj['y'] + 50)

		fwd = Vec2d( cos(obj['ang']), -sin(obj['ang'])) * 10
		rgt = Vec2d(-sin(obj['ang']), -cos(obj['ang'])) * 7

		color = (obj['r'], obj['g'], obj['b'])

		pointlist = [
			pos + fwd,
			pos + rgt - fwd,
			pos - 0.5 * fwd,
			pos - rgt - fwd,
		]

		pygame.draw.polygon(screen, color, pointlist)

	pygame.display.flip()

def ping():
	return struct.pack('B', msg_ping)

def status():
	keysdown = 0

	if keystate.get(K_UP, False):    keysdown += 1
	if keystate.get(K_DOWN, False):  keysdown += 2
	if keystate.get(K_LEFT, False):  keysdown += 4
	if keystate.get(K_RIGHT, False): keysdown += 8

	return struct.pack('!BB', msg_keysdown, keysdown)

def send(data):
	server.sendto(data, (host, port))

def parse_package(data):
	global objects
	header = ord(data[0])
	if header == msg_status:
		#data_type, fmt, keys = package[header]
		#values = struct.unpack(fmt)
		timestamp = struct.unpack('!I', data[1:5])
		remaining = data[5:]
		keys = ('type','r','g','b','x','dx','y','dy','ang','dang','status')
		objects = []
		while len(remaining):
			values = struct.unpack('!BBBBffffffB', remaining[:29])
			objects.append(dict(zip(keys, values)))
			remaining = remaining[29:]
		#print objects
	elif header == msg_ping:
		print 'pong'
	else:
		print 'Unknown header: %d' % header

def read_data():
	try:
		while True:
			data,addr = server.recvfrom(buffer_size)
			#print len(data)
			parse_package(data)
	except socket.error:
		return

done = False

while not done:
	#send(ping())
	send(status())
	read_data()
	for e in pygame.event.get():
		if e.type == KEYDOWN or e.type == KEYUP:
			keystate[e.key] = e.type == KEYDOWN

		if e.type == QUIT or (e.type == KEYDOWN and e.key == K_ESCAPE):
			done = True
			break
	draw()
	time.sleep(0.01)

