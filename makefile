LDIR=lib
SDIR=src
ODIR=obj
NAME=httpd

CFILES:=$(shell find $(SDIR) -name '*.cpp')
DEPS=$(CFILES:src/%.cpp=obj/%.o)

CC=g++
LIBS=-pthread
CFLAGS=-Wall -pedantic -I$(LDIR) $(LIBS) -std=c++11

all: $(NAME)
	@echo
	@echo "    To run, use ./httpd <porta>"
	@echo

$(NAME): $(DEPS) $(NAME).cpp
	$(CC) $^ -o $@ $(CFLAGS)

$(ODIR)/%.o: src/%.cpp $(CFILES)
	$(CC) -c $< -o $@ $(CFLAGS)

.phony: clean

clean:
	rm -rf $(ODIR)/*.o $(LDIR)/*~ $(SDIR)/*~ *~ $(NAME)
