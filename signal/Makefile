
pkg_packages := dbus-1

#PKG_CFLAGS  := $(shell pkg-config --cflags $(pkg_packages))
#PKG_LDFLAGS := $(shell pkg-config --libs $(pkg_packages))

DBUS_DIR 	:= $(HOME)/local/dbus
PKG_CFLAGS  := -I$(DBUS_DIR)/include/dbus-1.0 \
				-I$(DBUS_DIR)/lib/dbus-1.0/include
PKG_LDFLAGS := $(DBUS_DIR)/lib/libdbus-1.a -lpthread -lrt

ADD_CFLAGS := -g

CFLAGS  := $(PKG_CFLAGS) $(ADD_CFLAGS) $(CFLAGS)
LDFLAGS := $(PKG_LDFLAGS) $(LDFLAGS)

targets = send recv

.PHONY: all clean
all: $(targets)

send: send.c dbus-print-message.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

recv: recv.c dbus-print-message.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)


clean:
	$(RM) $(targets)
