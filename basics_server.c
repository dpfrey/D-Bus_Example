#include <stdio.h>
#include <stdlib.h>
#include <gio/gio.h>
#include <assert.h>
#include "basics_gen.h"
#include "config.h"

struct TestingState {
    GDBusObjectManagerServer *object_manager;
    Greetable *greetable_intf;
    guint16 num_times_called;
    unsigned int timer_count;
};

static struct {
    GMainLoop *loop;
} _;


void terminate(gpointer user_data)
{
    puts("Terminating the main loop");
    g_main_loop_quit(_.loop);
}

gboolean timeout_handler(gpointer user_data)
{
    struct TestingState *ts = user_data;
    ts->timer_count--;
    printf("timeout with remaining timeouts=%d\n", ts->timer_count);
    greetable_emit_timer_expired(ts->greetable_intf, ts->timer_count);
    return ts->timer_count != 0;
}

static gboolean on_handle_greet(
    Greetable *interface,
    GDBusMethodInvocation *invocation,
    const gchar *recipient,
    gpointer user_data)
{
    struct TestingState *ts = user_data;
    printf("%s %s\n", greetable_get_greeting(interface), recipient);
    ts->num_times_called++;
    greetable_complete_greet(interface, invocation, ts->num_times_called);

    return TRUE;
}


static void on_bus_acquired(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
    printf("Acquired bus \"%s\"\n", name);

    struct TestingState *ts = user_data;

    ts->object_manager = g_dbus_object_manager_server_new("/Testing");

    ObjectSkeleton *object_skeleton;
    Greetable *greetable;
    object_skeleton = object_skeleton_new("/Testing/Greeter");
    greetable = greetable_skeleton_new();
    ts->greetable_intf = greetable;
    object_skeleton_set_greetable(object_skeleton, greetable);
    // Release the reference since it's owned by the object now
    g_object_unref(greetable);

    greetable_set_greeting(greetable, "Hello");
    g_signal_connect(greetable, "handle-greet", G_CALLBACK(on_handle_greet), ts);

    // Export the object.  The object manager takes its own reference of the object, so we unref.
    g_dbus_object_manager_server_export(ts->object_manager, G_DBUS_OBJECT_SKELETON(object_skeleton));
    g_object_unref(object_skeleton);


    // Export all objects
    g_dbus_object_manager_server_set_connection(ts->object_manager, conn);
}

static void on_name_acquired(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
    printf("Acquired name \"%s\"\n", name);
}

static void on_name_lost(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
    if (!conn) {
        puts("Couldn't establish connection to bus");
    } else {
        printf("Lost name \"%s\"\n", name);
    }
}

int main(int argc, char **argv)
{
    printf(
        "basics_server v%d.%d.%d\n",
        Basics_VERSION_MAJOR,
        Basics_VERSION_MINOR,
        Basics_VERSION_SUB);

    struct TestingState *ts = calloc(sizeof(*ts), 1);
    assert(ts);

    const guint name_ref = g_bus_own_name(
        G_BUS_TYPE_SESSION,
        "io.mangoh",
        G_BUS_NAME_OWNER_FLAGS_NONE,
        on_bus_acquired,
        on_name_acquired,
        on_name_lost,
        ts,
        NULL);

    const guint interval_ms = 5 * 1000;
    ts->timer_count = 3;
    const guint timeout_event_id = g_timeout_add_full(
        G_PRIORITY_DEFAULT, interval_ms, timeout_handler, ts, terminate);
    printf(
        "Registered timer with id=%d, timeout=%d, count=%d\n",
        timeout_event_id,
        interval_ms,
        ts->timer_count);

    // TODO: by passing NULL as the first parameter, we are setting the GMainContext* as the
    // "default context". What is the purpose of this context parameter?
    _.loop = g_main_loop_new(NULL, FALSE);
    puts("Running the main loop");
    g_main_loop_run(_.loop);
    puts("The main loop has completed");

    g_bus_unown_name(name_ref);

    g_main_loop_unref(_.loop);

    return 0;
}
