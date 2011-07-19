#!/usr/bin/python

import socket
from socket import AF_INET, SOCK_DGRAM
import struct
import sys
import os
import time
import random
from math import sin, cos, isnan
from vec2d import Vec2d


### pygame

import pygame
from pygame.locals import *
pygame.init()

def set_screen_mode(fullscreen = False):
	global screen_size, screen, is_fullscreen

	is_fullscreen = fullscreen

	if fullscreen:
		modes = pygame.display.list_modes()
		w, h = max(modes, key=lambda x: x[0]*x[1])
		screen = pygame.display.set_mode((w,h), FULLSCREEN)
	else:
		aspect = 16 / 10.
		screen = pygame.display.set_mode((640, int(640 / aspect)))

	screen_size = screen.get_size()
	#pygame.mouse.set_visible(0)

set_screen_mode()
pygame.display.set_caption('Asteroids+')

### protocol

msg_keysdown = 2
msg_status   = 7
msg_ping     = 17
msg_name     = 32


### network

host =     sys.argv[1]  if len(sys.argv) > 1 else 'munkeliv.ath.cx'
port = int(sys.argv[2]) if len(sys.argv) > 2 else 50005

server = socket.socket(AF_INET,SOCK_DGRAM)
server.setblocking(0)
buffer_size = 102400

print 'Server at %s:%d' % (host, port)


### globals

class game:
	objects = []
	keystate = {}
	t0 = 0
	time_delta = float('inf')
	min_delta = float('inf')
	avg_delta = float('nan')
	max_delta = 0
	base_delta = float('nan')
	discard_messages = False
	stars = [((random.random() - 0.5) * screen_size[0],
	          (random.random() - 0.5) * screen_size[1],
	          255 * (random.random() ** 2.2)) for i in xrange(250)]
	margin = 15
	board_width  = 640 + margin * 2
	board_height = 400 + margin * 2
	fix_delta = False
	show_info = False

font = pygame.font.Font(None, 16)


### enums

class status_flags:
	engine = 1


###

def draw(t):
	screen.fill((0, 0, 0))

	def screen_coord(pos):
		x, y = pos
		return ((x + 320.) / 640. * screen_size[0], (1 - (y + 200) / 400.) * screen_size[1])

	for x, y, lum in game.stars:
		xs, ys = screen_coord((x, y))
		screen.fill((lum, lum, lum), (xs, ys, 1, 1))

	for obj in game.objects:
		dt = t - game.t0
		if dt < 0:
			print 'Time since t0 > 0! (dt = %d)' % dt
			dt = 0

		engine_on = bool(obj['status'] & status_flags.engine)
		pos_at_t0 = Vec2d(obj['x'], obj['y'])
		ang_at_t0 = obj['ang']
		vel_at_t0 = Vec2d(obj['dx'], obj['dy'])
		dang      = obj['dang']

		def integrator_1():
			pos = pos_at_t0 + vel_at_t0 * dt
			ang = ang_at_t0 + dang * dt
			return pos, ang

		def integrator_2():
			pos = pos_at_t0
			ang = ang_at_t0
			vel = vel_at_t0

			if dt > 1000:
				print 'Bailing out from integration -- too far ahead'
				return pos, ang

			for i in xrange(dt):
				if engine_on:
					vel += 0.001 * Vec2d(cos(ang), sin(ang))
					speed = vel.get_length()
					maxSpeed = 0.3
					if speed > maxSpeed:
						vel *= maxSpeed / speed

				damp = 0.999
				vel *= damp

				pos += vel
				ang += dang
			
			def bind(value, length):
				return (length * 1.5 + value % length) % length - length / 2

			bound_pos = Vec2d(
				bind(pos[0], game.board_width),
				bind(pos[1], game.board_height)
			)

			return bound_pos, ang


		pos, ang = integrator_2()


		space = (
			Vec2d(-sin(ang), cos(ang)) * 3, # x axis / right
			Vec2d( cos(ang), sin(ang)) * 3  # y axis / forward
		)

		color = (obj['r'], obj['g'], obj['b'])

		ship_shape  = [(0, 4), (2, -3), (0, -1), (-2, -3)]
		burst_shape = [(0,-2), (1, -3), (0, -4), (-1, -3)]

		ship_shape  = [(0, 4), (2, -2), (0, -1), (-2, -2)]
		burst_shape = [(0,-1), (2, -2), (0, -3), (-2, -2)]

		def draw_shape(shape, origo, space, color):
			rebase = lambda x, y: (space[0][0] * x + space[1][0] * y, space[0][1] * x + space[1][1] * y)
			pygame.draw.polygon(screen, color, [screen_coord(origo + rebase(x, y)) for x, y in shape])
			#pygame.draw.aalines(screen, color, True, [origo + rebase(x, y) for x, y in shape])

		draw_shape(ship_shape, pos, space, color)

		if engine_on:
			y = 255 * ((t / 300) % 2)
			draw_shape(burst_shape, pos, space, (255,y,0))

	def println(lines):
		for i,text in enumerate(lines):
			antialias_text = True
			text_color = (255, 255, 0)
			text = font.render(text, antialias_text, text_color)
			screen.blit(text, (5, 5 + 16 * i))

	if game.show_info:
		lines = [
			'Objects: %d' % len(game.objects),
			'Base delta: %.1f' % (game.base_delta),
			'Min delta: %.1f' % (game.min_delta - game.base_delta),
			'Max delta: %.1f' % (game.max_delta - game.base_delta),
			'Avg delta: %.1f' % (game.avg_delta - game.base_delta),
		]

		if game.fix_delta:
			lines.append('Fixed delta: %1.f' % (game.time_delta - game.base_delta))

		println(lines)

	pygame.display.flip()

def ping():
	return struct.pack('B', msg_ping)

def status():
	keysdown = 0

	if game.keystate.get(K_UP, False):    keysdown += 1
	if game.keystate.get(K_DOWN, False):  keysdown += 2
	if game.keystate.get(K_LEFT, False):  keysdown += 4
	if game.keystate.get(K_RIGHT, False): keysdown += 8
	if game.keystate.get(K_SPACE, False): keysdown += 16

	return struct.pack('!BB', msg_keysdown, keysdown)

def name():
	local_name = os.environ['USER']
	
	return struct.pack('!BB', msg_name, len(local_name)) + local_name

def send(data):
	server.sendto(data, (host, port))

def est_server_time():
	# Assumption: remote = local + time_delta
	local = pygame.time.get_ticks()
	return local + game.time_delta

packets = 0
def parse_package(data):
	global packets

	header = ord(data[0])

	if game.keystate.get(K_d, False):
		return

	#packets += 1
	#if packets % 20 < 5:
	#	return

	if header == msg_status:
		#data_type, fmt, keys = package[header]
		#values = struct.unpack(fmt)
		timestamp = struct.unpack('!I', data[1:5])[0]
		if timestamp >= game.t0:
			game.t0 = timestamp

			local_time = pygame.time.get_ticks()

			delta = timestamp - local_time

			game.max_delta = max(delta, game.max_delta)
			game.min_delta = min(delta, game.min_delta)

			if isnan(game.avg_delta):
				game.avg_delta  = delta
				game.base_delta = delta
			else:
				weight = 0.01
				game.avg_delta = game.avg_delta * (1 - weight) + delta * weight

			if game.time_delta == float('inf'):
				game.time_delta = delta

			if not game.fix_delta:
				game.time_delta = int(game.avg_delta) + 10

			#game.time_delta = game.min_delta + 30
			#print 'Package delay: %.0f' % package_delay

			#if local_time + game.time_delta > timestamp:
			#	game.time_delta = timestamp - local_time
			#	print game.time_delta

			remaining = data[5:]
			keys = ('type','r','g','b','x','dx','y','dy','ang','dang','status')
			game.objects = []
			while len(remaining):
				values = struct.unpack('!BBBBffffffB', remaining[:29])
				game.objects.append(dict(zip(keys, values)))
				remaining = remaining[29:]
		else:
			print 'Discard outdated message'
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
loopcount = 0

while not done:
	#send(ping())
	send(status())
	if loopcount % 100 == 0:
		send(name())
	read_data()
	for e in pygame.event.get():
		if e.type == KEYDOWN or e.type == KEYUP:
			game.keystate[e.key] = e.type == KEYDOWN

			if e.type == KEYDOWN:
				if e.key == K_f:
					set_screen_mode(not is_fullscreen)
				elif e.key == K_d:
					game.fix_delta = not game.fix_delta
				elif e.key == K_i:
					game.show_info = not game.show_info

		if e.type == QUIT or (e.type == KEYDOWN and e.key == K_ESCAPE):
			done = True
			break
	draw(est_server_time())
	time.sleep(0.01)

	loopcount += 1

