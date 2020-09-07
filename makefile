NAME = mu-httpd

IDIR = src
SDIR = src
ODIR = obj

GCC  ?= gcc
STDC ?= c11
LIBS ?= -lm -pthread

CFLAGS = -std=$(STDC) -I$(IDIR) -Wall $(LIBS)

CFILES := $(shell find $(SDIR) -name '*.c')
DEPS    = $(CFILES:src/%.c=obj/%.o)

all: install $(NAME)

install:
	@mkdir -p $(ODIR)

clean:
	@rm -rf $(ODIR)/*.o $(ODIR)/*~ $(SDIR)/*~ *~ $(NAME)

ifneq ($(wildcard $(ODIR)/.),)
-include $(shell find $(ODIR) -name '*.d')
endif

$(NAME): $(DEPS)
	$(GCC) $(CFLAGS) $^ -o $@

$(ODIR)/%.o: $(SDIR)/%.c
	$(GCC) $(CFLAGS) -MMD -c $< -o $@

.PHONY: all install clean
