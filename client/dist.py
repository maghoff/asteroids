#!/usr/bin/python

from subprocess import check_call as run

dirname = 'asteroids+'
zipname = dirname + '.zip'
run(['rm', '-rf', dirname])
run(['rm', '-f',  zipname])
run(['mkdir', dirname])
run(['cp', 'asteroids+.py', dirname])
run(['cp', 'vec2d.py', dirname])
run(['cp', '-r', '/Library/Python/2.6/site-packages/pygame', dirname])
run(['zip', '-r', dirname, dirname])
run(['rm', '-rf', dirname])
