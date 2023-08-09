# mu-HTTPd: A very very simple HTTP server.
# @file Makefile for compiling, installing and automatically testing.
# @author Rodrigo Siqueira <rodriados@gmail.com>
# @copyright 2014-present Rodrigo Siqueira
NAME = mu-httpd

INCDIR = src
SRCDIR = src
OBJDIR = obj
BINDIR = bin

CC   ?= gcc
STDC ?= c11

LIBS ?= -lm -pthread

# Defining macros inside code at compile time. This can be used to enable or disable
# certain features on code or affect the projects compilation.
FLAGS   ?= -Wall
CCFLAGS ?= -std=$(STDC) -I$(INCDIR) $(FLAGS) $(LIBS)

SRCFILES := $(shell find $(SRCDIR) -name '*.c')
OBJFILES = $(SRCFILES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: build

debug: override CFLAGS := -ggdb $(CFLAGS)
debug: build

build: prepare-build $(BINDIR)/$(NAME)

prepare-build:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

clean:
	@rm -rf $(OBJDIR)
	@rm -fr $(BINDIR)

.PHONY: all clean debug build
.PHONY: prepare-build

# Creates dependency on header files. This is valuable so that whenever a header
# file is changed, all objects depending on it will be forced to recompile.
ifneq ($(wildcard $(OBJDIR)/.),)
-include $(shell find $(OBJDIR) -name '*.d')
endif

$(BINDIR)/$(NAME): $(OBJFILES)
	$(CC) $(CCFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CCFLAGS) -MMD -c $< -o $@
