CXX	= g++
CFLAGS	= -Wall -O3 -I.. -I/usr/local/include
LIBS	= -larmadillo -lglfw -framework GLUT -framework OpenGL
#LIBS	= -lglut -lGL -lpthread # Linux
SRCS 	= $(wildcard *.cc)
OBJS	= $(SRCS:.cc=.o)
EXES  	= $(SRCS:.cc=)

.PHONY: clean

all: $(EXES)

.cc.o: 
	$(CXX) $(CFLAGS) -c $< -o $@

define TEST_BUILD_template
.PHONY: $(1)-clean

$(1): $(1).o
	$$(CXX) $$(CFLAGS) $$< $$(LIBS) -o $$@ 

$(1)-clean:
	$$(RM) -f $(1) $(1).o


CLEANS += $(1)-clean

endef

$(foreach testname,$(EXES),$(eval $(call TEST_BUILD_template,$(testname))))

clean: $(CLEANS)

-include .depend

.depend: $(wildcard *.cc) $(wildcard *.h)
	@echo 'compute dependencies...'
	@$(CXX) $(CFLAGS) -MM $(SRCS) > $@

