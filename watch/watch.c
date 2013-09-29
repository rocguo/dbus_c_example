
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <poll.h>

#include "dbus-print-message.h"

#define MAX_WATCHES     256

static struct pollfd poll_fds[MAX_WATCHES];
static DBusWatch* dbus_watches[MAX_WATCHES];
int num_watches;

static DBusHandlerResult
dbusFilter(DBusConnection* bus, DBusMessage* msg, void* data)
{
    print_message(msg, 0);
    return DBUS_HANDLER_RESULT_HANDLED;
}

static dbus_bool_t
addWatch(DBusWatch* watch, void* data)
{
    short events = 0;
    int fd;
    unsigned flags;

    printf("DBus add watch: %p\n", watch);

    fd = dbus_watch_get_unix_fd(watch);
    printf("  FD: %d", fd);
    flags = dbus_watch_get_flags(watch);

    if (flags & DBUS_WATCH_READABLE) {
        events |= POLLIN;
        printf("  readable");
    }
    if (flags & DBUS_WATCH_WRITABLE) {
        events |= POLLOUT;
        printf("  writable");
    }
    printf("\n");

    poll_fds[num_watches].fd = fd;
    poll_fds[num_watches].events = events;
    dbus_watches[num_watches] = watch;
    num_watches++;

    printf("  add a watch, total watch number: %d\n", num_watches);

    return 1;
}

static void
removeWatch(DBusWatch* watch, void* data)
{
    int i;
    int found = 0;
    int j;

    printf("DBus remove watch: %p\n", watch);
    for (i = 0; i < num_watches; i++) {
        if (dbus_watches[i] == watch) {
            found = 1;
            break;
        }
    }
    if (!found) {
        printf("  cannot find watch in existing watches!\n");
        return;
    }

    memcpy(&dbus_watches[i], &dbus_watches[i+1],
            (num_watches - i - 1) * sizeof(DBusWatch*));
    memcpy(&poll_fds[i], &poll_fds[i+1],
            (num_watches - i - 1) * sizeof(struct pollfd));
    num_watches--;

    printf("  remove a watch, total watch number: %d\n", num_watches);
}

static void
toggleWatch(DBusWatch* watch, void* data)
{
    printf("Toggle a watch: %p\n", watch);
    printf("TODO!\n");
}

static DBusConnection*
dbusInit()
{
    DBusConnection* bus = NULL;
    DBusError err;
    int ret;

    dbus_error_init(&err);
    bus = dbus_bus_get(DBUS_BUS_SESSION, &err);

    if (bus == NULL) {
        fprintf(stderr, "Connection error: %s\n", err.message);
        dbus_error_free(&err);
        return NULL;
    }

    /* watch for all signals & method calls */
    printf("Add matches\n");
    dbus_bus_add_match(bus, "type='signal'", NULL);
    dbus_bus_add_match(bus, "type='method_call'", NULL);

    if (!dbus_connection_set_watch_functions(bus, addWatch,
                removeWatch, toggleWatch, NULL, NULL)) {
        printf("dbus_connection_set_watch_functions failed\n");
        dbus_connection_unref(bus);
        return NULL;
    }

    if (!dbus_connection_add_filter(bus, dbusFilter, NULL, NULL)) {
        printf("dbus_connection_add_filter failed\n");
        dbus_connection_unref(bus);
        return NULL;
    }

    return bus;
}

static void
handleEvent(DBusConnection* bus, short events, DBusWatch* watch)
{
    unsigned flags = 0;

    if (events & POLLIN)
        flags |= DBUS_WATCH_READABLE;
    if (events & POLLOUT)
        flags |= DBUS_WATCH_WRITABLE;
    if (events & POLLHUP)
        flags |= DBUS_WATCH_HANGUP;
    if (events & POLLERR)
        flags |= DBUS_WATCH_ERROR;

    while (!dbus_watch_handle(watch, flags)) {
        printf("dbus_watch_handle failed\n");
        exit(-1);
    }

    while (dbus_connection_dispatch(bus) == DBUS_DISPATCH_DATA_REMAINS)
        ;
}

int
main(int argc, char** argv)
{
    DBusConnection* bus = NULL;
    DBusMessage* msg = NULL;
    DBusMessageIter args;
    DBusError error;
    int n = 0;

    printf("Init dbus...\n");
    bus = dbusInit();
    if (bus == NULL)
        return -1;

    while (1) {
        int i;

        printf("polling...\n");
        if (poll(poll_fds, num_watches, -1) <= 0) {
            perror("poll");
            break;
        }

        printf("DBus dispatching...\n");
        for (i = 0; i < num_watches; i++) {
            if (poll_fds[i].revents) {
                printf("  [%d] %x\n", i, poll_fds[i].revents);
                handleEvent(bus, poll_fds[i].revents, dbus_watches[i]);
                poll_fds[i].revents = 0;
            }
        }
        sleep(1);
        n++;

        if (n >= 8)
            break;
    }

    dbus_connection_unref(bus);

    return 0;
}

