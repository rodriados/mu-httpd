LDIR=lib
SDIR=src
ODIR=obj
NAME=mu-http

CFILES:=$(shell find $(SDIR) -name '*.cpp')
DEPS=$(CFILES:src/%.cpp=obj/%.o)

CC=g++
LIBS=-pthread
CFLAGS=-Wall -pedantic -I$(LDIR) $(LIBS) -std=c++11

all: $(NAME)
	@echo
	@echo "    To run, use ./mu-http <port>"
	@echo

$(NAME): $(DEPS) $(NAME).cpp
	$(CC) $^ -o $@ $(CFLAGS)

$(ODIR)/%.o: src/%.cpp $(CFILES)
	$(CC) -c $< -o $@ $(CFLAGS)

.phony: clean

clean:
	rm -rf $(ODIR)/*.o $(LDIR)/*~ $(SDIR)/*~ *~ $(NAME)
