CC=gcc
SDL_CFLAGS = `sdl-config --cflags`
SDL_LIBS = `sdl-config --libs`
CFLAGS=-ggdb -DUNIX -DGAMEMOD -Wall $(SDL_CFLAGS)
OBJS=actor.o \
	animations.o \
	collision.o \
	debug.o \
	debug.grid.o \
	debug.scene.o \
	extra.o \
	filereader.o \
	gamestate.o \
	grid.o \
	holomap.o \
	hqrdepack.o \
	images.o \
	interface.o \
	keyboard.o \
	lbaengine.o \
	main.o \
	menu.o \
	menuoptions.o \
	movements.o \
	movies.o \
	music.o \
	redraw.o \
	renderer.o \
	resources.o \
	scene.o \
	script.life.o \
	script.move.o \
	sdlengine.o \
	sound.o \
	text.o

LIBS=$(SDL_LIBS) -lSDL_mixer -lSDL_ttf

all: prequengine

prequengine: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm $(OBJS)
