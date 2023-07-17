CFLAGS ?= -Werror -Wall -Wpedantic
LDFLAGS ?= -Werror -Wall -Wpedantic

SRCS := src/pvrun.c
OBJS := $(patsubst %.c,%.o,$(SRCS))
TARGET := src/pvrun

all: $(TARGET)

$(TARGET): $(OBJS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: install
install: $(TARGET)
	install -Dm 755 $(TARGET) $(DESTDIR)/usr/local/bin/$(TARGET)
