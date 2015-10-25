CC=gcc
CSTD=c99
CLIB=-lm -lSDL2 -lSDL2_image -lSDL2_gfx -lSDL2_mixer
CWARN=-Wall -pedantic
CSRC=lib/duktape.c lib/lodepng.c
CO=-O0 -ggdb
CINCLUDE=lib/

src=src/
prog=main

all:
	${CC} -std=${CSTD} ${CO} ${CLIB} ${CWARN} -I${CINCLUDE} ${src}${prog}.c ${CSRC} -o${prog}

