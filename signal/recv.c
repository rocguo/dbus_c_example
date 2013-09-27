
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dbus-print-message.h"

static DBusConnection*
dbusInit()
{
    DBusConnection* bus = NULL;
    DBusError err;
    int ret;

    dbus_error_init(&err);
    bus = dbus_bus_get(DBUS_BUS_SESSION, &err);

    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Connection error: %s\n", err.message);
        dbus_error_free(&err);
    }

    dbus_bus_add_match(bus, "type='signal',interface='test.signal.Type'",
            &err);
    dbus_connection_flush(bus);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "Match error: %s\n", err.message);
        dbus_error_free(&err);
        exit(1);
    }

    return bus;
}

int
main(int argc, char** argv)
{
    DBusConnection* bus = NULL;
    DBusMessage* msg = NULL;
    DBusMessageIter args;
    DBusError error;

    printf("Init dbus...\n");
    bus = dbusInit();

    printf("Waiting...\n");
    //while (dbus_connection_read_write(bus, -1))
    while (dbus_connection_read_write_dispatch(bus, -1)) {
        char* sigvalue;

        msg = dbus_connection_pop_message(bus);
        if (msg == NULL)
            continue;
        do {
            printf("Received message:\n");
            print_message(msg, 0);
            printf("\n");

            if (dbus_message_is_signal(msg, "test.signal.Type", "Test")) {
                if (!dbus_message_iter_init(msg, &args))
                    fprintf(stderr, "Message has no arguments!\n");
                else
                if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
                    fprintf(stderr, "Argument is not string!\n");
                else {
                    dbus_message_iter_get_basic(&args, &sigvalue);
                    printf("Got signal with value %s\n", sigvalue);
                }
            } else {
                printf("Message filtered out\n");
            }

            dbus_message_unref(msg);
        } while ((msg = dbus_connection_pop_message(bus)) != NULL);
        printf("\nWaiting...\n");
    }
}

