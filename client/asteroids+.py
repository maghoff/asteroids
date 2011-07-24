#!/usr/bin/python

import socket
from socket import AF_INET, SOCK_DGRAM
import struct
import sys
import os
import time
import random
from collections import defaultdict
from math import sin, cos, isnan
from vec2d import Vec2d


### pygame

import pygame
from pygame.locals import *
pygame.init()
pygame.mixer.init()

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

msg_keysdown  = 2
msg_status    = 7
msg_ping      = 17
msg_name      = 32
msg_ship_info = 59


### network

host =     sys.argv[1]  if len(sys.argv) > 1 else 'munkeliv.ath.cx'
port = int(sys.argv[2]) if len(sys.argv) > 2 else 50005

server = socket.socket(AF_INET,SOCK_DGRAM)
server.setblocking(0)
buffer_size = 102400

print 'Server at %s:%d' % (host, port)


### globals

class game:
	objects   = []
	keystate  = {}
	ship_info = {}
	seen_ids  = defaultdict(lambda: set())

	t0 = 0
	time_delta = float('inf')
	min_delta  = float('inf')
	avg_delta  = float('nan')
	base_delta = float('nan')
	max_delta  = 0

	stars = [((random.random() - 0.5) * screen_size[0],
	          (random.random() - 0.5) * screen_size[1],
	          255 * (random.random() ** 2.2)) for i in xrange(250)]

	local_name = os.environ.get('USER', '')
	
	margin       = 15
	board_width  = 640 + margin * 2
	board_height = 400 + margin * 2

	fix_delta    = False
	show_info    = False
	show_name    = True
	sound_on     = True
	arcade_style = True

font = pygame.font.Font(None, 16) if pygame.font else None


### enums

class status_flags:
	engine = 1


###

def screen_coord(pos):
	x, y = pos
	return ((x + 320.) / 640. * screen_size[0], (1 - (y + 200) / 400.) * screen_size[1])

def draw_text(text, pos, text_color, centered=False):
	antialias_text = True
	renederd_text = font.render(text, antialias_text, text_color)
	if centered:
		pos = renederd_text.get_rect(centerx = pos[0], centery = pos[1])

	screen.blit(renederd_text, pos)

def draw_ship(obj, dt, t):
	engine_on = bool(obj['status'] & status_flags.engine)
	pos_at_t0 = Vec2d(obj['x'], obj['y'])
	ang_at_t0 = obj['ang']
	vel_at_t0 = Vec2d(obj['dx'], obj['dy'])
	dang      = obj['dang']
	if 'id' in obj:
		info  = game.ship_info.get(obj['id'], {})
	else:
		info  = None

	def integrator():
		pos = pos_at_t0
		ang = ang_at_t0
		vel = vel_at_t0

		if dt > 1000:
			print 'Bailing out from integration -- too far ahead'
			return pos, ang

		for i in xrange(dt):
			if engine_on:
				vel += 0.001 * Vec2d(cos(ang), sin(ang))

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


	pos, ang = integrator()


	space = (
		Vec2d(-sin(ang), cos(ang)) * 3, # x axis / right
		Vec2d( cos(ang), sin(ang)) * 3  # y axis / forward
	)

	if info is not None:
		color = (info.get('r', 255), info.get('g', 255), info.get('b', 255))
	else:
		color = (obj['r'], obj['g'], obj['b'])

	ship_shape  = [(0, 4), (2, -3), (0, -1), (-2, -3)]
	burst_shape = [(0,-2), (1, -3), (0, -4), (-1, -3)]

	ship_shape  = [(0, 4), (2, -2), (0, -1), (-2, -2)]
	burst_shape = [(0,-1), (2, -2), (0, -3), (-2, -2)]

	def draw_shape(shape, origo, space, color):
		rebase = lambda x, y: (space[0][0] * x + space[1][0] * y, space[0][1] * x + space[1][1] * y)
		path = [screen_coord(origo + rebase(x, y)) for x, y in shape]
		if game.arcade_style:
			pygame.draw.lines(screen, color, True, path)
		else:
			pygame.draw.polygon(screen, color, path)

	draw_shape(ship_shape, pos, space, color)

	if engine_on:
		y = 255 * ((t / 300) % 2)
		draw_shape(burst_shape, pos, space, (255,y,0))

	if game.show_name and font and info is not None:
		draw_text(info.get('name', '?'), screen_coord(pos - Vec2d(0, 20)), color, True)

def draw_bullet(obj, dt, t):
	pos_at_t0 = Vec2d(obj['x'], obj['y'])
	vel_at_t0 = Vec2d(obj['dx'], obj['dy'])

	pos = pos_at_t0 + vel_at_t0 * dt

	if game.arcade_style:
		pygame.draw.line(screen, (255, 255, 255), screen_coord(pos - Vec2d(3, 0)), screen_coord(pos + Vec2d(3, 0)))
		pygame.draw.line(screen, (255, 255, 255), screen_coord(pos - Vec2d(0, 3)), screen_coord(pos + Vec2d(0, 3)))
	else:
		pygame.draw.circle(screen, (255, 255, 255), screen_coord(pos), 2)

def sound_bullet(obj):
	if game.sound_on:
		sound  = pygame.mixer.Sound('pewpew.wav')
		channel = sound.play()

def sound_spawn(obj):
	if game.sound_on:
		sound  = pygame.mixer.Sound('spawn.wav')
		channel = sound.play()

# Functions for each type. Tuple of
#   - draw - how to draw the object
#   - encounter - what to do when first encountering an
#                 object with this type/id
type_functions = {
	 7: (draw_ship,   None),
	 8: (draw_ship,   sound_spawn),
	 9: (draw_bullet, None),
	10: (draw_bullet, sound_bullet),
}

def draw(t):
	screen.fill((0, 0, 0))

	for x, y, lum in game.stars:
		xs, ys = screen_coord((x, y))
		screen.fill((lum, lum, lum), (xs, ys, 1, 1))

	for obj in game.objects:
		dt = t - game.t0
		if dt < 0:
			#print 'Time since t0 > 0! (dt = %d)' % dt
			dt = 0

		draw_func, encounter = type_functions[obj['type']]
		
		draw_func(obj, dt, t)


	def println(lines):
		for i,text in enumerate(lines):
			draw_text(text, (5, 5 + 16 * i), (255, 255, 0))

	if game.show_info and font:
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

def ping_message():
	return struct.pack('B', msg_ping)

def status_message():
	keysdown = 0

	if game.keystate.get(K_UP, False):    keysdown += 1
	if game.keystate.get(K_DOWN, False):  keysdown += 2
	if game.keystate.get(K_LEFT, False):  keysdown += 4
	if game.keystate.get(K_RIGHT, False): keysdown += 8
	if game.keystate.get(K_SPACE, False): keysdown += 16

	return struct.pack('!BB', msg_keysdown, keysdown)

def serialize_string(string):
	return struct.pack('!I', len(string)) + string

def name_message():
	name = game.local_name.encode('utf-16-be')
	return struct.pack('!B', msg_name) + serialize_string(name)

def send(data):
	server.sendto(data, (host, port))

def est_server_time():
	# Assumption: remote = local + time_delta
	local = pygame.time.get_ticks()
	return local + game.time_delta

def format_string_length(s):
	'''Calculate the size of the string needed to unpack a format string
	as specified in http://docs.python.org/library/struct.html'''

	char_size = {
		'b': 1,
		'B': 1,
		'?': 1,
		'h': 2,
		'H': 2,
		'i': 4,
		'I': 4,
		'l': 4,
		'L': 4,
		'q': 8,
		'Q': 8,
		'f': 4,
		'd': 8,
	}
	return sum(char_size[char] for char in s)

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

			remaining = data[5:]

			deserializer = {
				 7: ('BBBffffffB', ('r','g','b','x','dx','y','dy','ang','dang','status')),
				 8: (  'BffffffB', ('id',       'x','dx','y','dy','ang','dang','status')),
				 9: (      'ffff', (            'x','dx','y','dy'                      )),
				10: (     'Hffff', ('id',       'x','dx','y','dy'                      )),
			}

			game.objects = []
			while len(remaining):
				obj_type = struct.unpack('!B', remaining[0])[0]
				remaining = remaining[1:]
				
				format_string, keys = deserializer[obj_type]

				substring_size = format_string_length(format_string)
				values = struct.unpack('!' + format_string, remaining[:substring_size])
				obj = dict(zip(keys, values) + [('type', obj_type)])
				game.objects.append(obj)
				remaining = remaining[substring_size:]

				draw_func, encounter = type_functions[obj_type]
				if encounter is not None:
					if obj['id'] not in game.seen_ids[obj_type]:
						game.seen_ids[obj_type].add(obj['id'])
						encounter(obj)
		else:
			print 'Discard outdated message'
	elif header == msg_ping:
		print 'pong'
	elif header == msg_ship_info:
		remaining = data[1:]
		while len(remaining):
			id_, r, g, b, name_len = struct.unpack('!BBBBI', remaining[:8])
			end_of_name = 8 + name_len
			name = remaining[8:end_of_name].decode('utf-16-be')
			remaining = remaining[end_of_name:]
			game.ship_info[id_] = {
				'r': r,
				'g': g,
				'b': b,
				'name': name,
			}
	else:
		print 'Unknown header: %d (msg: %s)' % (header, ' '.join('%02x' % ord(c) for c in data))

def read_data():
	try:
		while True:
			data,addr = server.recvfrom(buffer_size)
			parse_package(data)
	except socket.error:
		return

done = False
loopcount = 0

while not done:
	send(status_message())
	if loopcount % 100 == 0:
		send(name_message())
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
				elif e.key == K_c:
					game.ship_info = {}
				elif e.key == K_s:
					game.sound_on = not game.sound_on
				elif e.key == K_t:
					game.arcade_style = not game.arcade_style

		if e.type == QUIT or (e.type == KEYDOWN and e.key == K_ESCAPE):
			done = True
			break
	draw(est_server_time())
	time.sleep(0.01)

	loopcount += 1

