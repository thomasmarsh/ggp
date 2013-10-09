CXX	= g++
CFLAGS	= -Wall -pipe -g -I../engine -I.. -I/usr/local/include
LDFLAGS	= -pipe
LIBS	= -larmadillo -lglfw -framework OpenGL -framework Cocoa -framework Accelerate # OS X
#LIBS	= -lglfw -lGL -larmadillo -lpthread # Linux
SRCS 	= $(wildcard *.cc)
OBJS	= $(SRCS:.cc=.o)
EXES  	= $(SRCS:.cc=)

.PHONY: clean

all: $(EXES)

.cc.o: 
	$(CXX) $(CFLAGS) -c $< -o $@

define BUILD_template
.PHONY: $(1)-clean

$(1): $(1).o
	$$(CXX) $$(LDFLAGS) $$< $$(LIBS) -o $$@ 

$(1)-clean:
	$$(RM) -f $(1) $(1).o

CLEANS += $(1)-clean
endef

$(foreach exe,$(EXES),$(eval $(call BUILD_template,$(exe))))

clean: $(CLEANS)

-include .depend

.depend: $(wildcard *.cc) $(wildcard *.h)
	@echo 'compute dependencies...'
	@$(CXX) $(CFLAGS) -MM $(SRCS) > $@

