
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//#define WAIT_FOR_REPLY

#define NOTIFY_TARGET       "org.freedesktop.Notifications"
#define NOTIFY_OBJ_PATH     "/org/freedesktop/Notifications"
#define NOTIFY_INTERFACE    "org.freedesktop.Notifications"
#define NOTIFY_METHOD       "Notify"

static void
terminateOnError(const char* msg, DBusError* error)
{
    assert(msg != NULL);
    assert(error != NULL);

    if (dbus_error_is_set(error)) {
        fprintf(stderr, "%s\n", msg);
        fprintf(stderr, "DBusError.name: %s\n", error->name);
        fprintf(stderr, "DBusError.message: %s\n", error->message);

        dbus_error_free(error);
        exit(EXIT_FAILURE);
    }
}

static void
fillArgs(DBusMessage* msg)
{
    const char* app = "a.out";
    const char* icon = "/usr/share/icons/gnome/32x32/status/sunny.png";
    const char* summary = "DBus C example";
    const char* body = "Hello world!";
    unsigned id = 0;
    int timeout = -1;
    DBusMessageIter iter;
    DBusMessageIter array;

    /*
     * Notify(String app_name, UInt32 id, String icon,
     *  String summary, String body,
     *  Array of [String] actions,
     *  Dict of {String, Variant} hints,
     *  Int32 timeout)
     */
    dbus_message_iter_init_append(msg, &iter);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &app);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &id);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &icon);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &summary);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &body);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
            DBUS_TYPE_STRING_AS_STRING, &array);
    dbus_message_iter_close_container(&iter, &array);
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
            "{sv}", &array);
    dbus_message_iter_close_container(&iter, &array);
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &timeout);
}

static void
recvReply(DBusPendingCall* pending)
{
    DBusMessage* msg = NULL;
    DBusMessageIter args;

    printf("Wait for call complete\n");
    dbus_pending_call_block(pending);

    msg = dbus_pending_call_steal_reply(pending);
    if (msg == NULL) {
        printf("Reply NULL\n");
        exit(-1);
    }
 
    dbus_pending_call_unref(pending);

    printf("Get reply message\n");
    if (!dbus_message_iter_init(msg, &args))
        printf("Message has no arguments!\n");
    else {
        int type = dbus_message_iter_get_arg_type(&args);

        switch (type) {
        case DBUS_TYPE_STRING:
        {
            char* tmp;
            dbus_message_iter_get_basic(&args, &tmp);
            printf("Reply: %c - %s\n", type, tmp);
            break;
        }

        case DBUS_TYPE_UINT32:
        {
            dbus_uint32_t id;
            dbus_message_iter_get_basic(&args, &id);
            printf("Reply: %c - %d\n", type, id);
            break;
        }

        default:
            printf("TODO: handle more dbus data type\n");
        }
    }

    if (dbus_message_iter_next(&args)){
        printf("Message has more arguments\n");
    }

    printf("Cleaning up reply message\n");
    dbus_message_unref(msg);
}

int
main(int argc, char** argv)
{
    DBusConnection* bus = NULL;
    DBusMessage* msg = NULL;
    DBusError error;
    DBusPendingCall* pending;

    dbus_error_init(&error);

    printf("Connecting to Session D-Bus\n");
    bus = dbus_bus_get(DBUS_BUS_SESSION, &error);
    terminateOnError("Failed to open Session bus\n", &error);
    assert(bus != NULL);

    printf("Creating a message object\n");
    msg = dbus_message_new_method_call(
            NOTIFY_TARGET,
            NOTIFY_OBJ_PATH,
            NOTIFY_INTERFACE,
            NOTIFY_METHOD);

    assert(msg != NULL);

    printf("Appending arguments to the message\n");
    fillArgs(msg);

#ifndef WAIT_FOR_REPLY
    dbus_message_set_no_reply(msg, TRUE);

    printf("Adding message to client send-queue\n");
    dbus_connection_send(bus, msg, NULL);

    printf("Waiting for send-queue to be send out\n");
    dbus_connection_flush(bus);

    printf("Cleaning up message\n");
    dbus_message_unref(msg);
#else
    printf("Adding message to client send-queue\n");

    dbus_connection_send_with_reply(bus, msg, &pending, -1);

    printf("Waiting for send-queue to be send out\n");
    dbus_connection_flush(bus);

    printf("Cleaning up message\n");
    dbus_message_unref(msg);

    recvReply(pending);
#endif

    printf("Cleaning up connection\n");
    dbus_connection_unref(bus);
    return 0;
}

