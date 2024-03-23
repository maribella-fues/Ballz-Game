# Makefile for lab11

project: project.c
	gcc project.c gfx.o -lX11 -lm -o project

clean:
	rm project 

