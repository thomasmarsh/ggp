SUBDIRS = games test

.PHONY: clean $(SUBDIRS)

all: $(SUBDIRS)

games:
	make -j 8 -C games

test:
	make -j 8 -C test

clean:
	make -C games clean
	make -C test clean
