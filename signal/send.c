
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

    return bus;
}

int
main(int argc, char** argv)
{
    DBusConnection* bus = NULL;
    DBusMessage* msg = NULL;
    DBusMessageIter args;
    dbus_uint32_t serial = 0;
    //const char* sigvalue = "Hello world";
    char buf[128];
    char* sigvalue = buf;
    int i;

    printf("Init dbus...\n");
    bus = dbusInit();

    for (i = 0; i < 4; i++) {
        msg = dbus_message_new_signal("/test/signal/Object",
                "test.signal.Type",
                "Test");
        dbus_message_iter_init_append(msg, &args);
        sprintf(buf, "Hello world [%d]", i);
        dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &sigvalue);

        printf("Sending signal...\n");
        dbus_connection_send(bus, msg, &serial);
        dbus_message_unref(msg);
    }

    dbus_connection_flush(bus);

    printf("Cleaning up connection\n");
    dbus_connection_unref(bus);

    return 0;
}

