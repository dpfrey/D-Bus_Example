#include <stdio.h>
#include <stdlib.h>
#include <gio/gio.h>
#include "basics_gen.h"
#include "config.h"

static struct {
    GMainLoop *loop;
    guint16 num_times_called;
} _;

void terminate(gpointer user_data)
{
    puts("Terminating the main loop");
    g_main_loop_quit(_.loop);
}

gboolean timeout_printer(gpointer user_data)
{
    unsigned int *timer_count = user_data;
    (*timer_count)--;
    printf("timeout with remaining timeouts=%d\n", *timer_count);
    return (*timer_count) != 0;
}

static gboolean on_handle_hello_world(
    IoMangohHelloWorld *interface,
    GDBusMethodInvocation *invocation,
    const gchar *greeting,
    gpointer user_data)
{
    printf("Hello %s\n", greeting);
    _.num_times_called++;
    io_mangoh_hello_world_complete_hello_world(interface, invocation, _.num_times_called);

    return TRUE;
}


static void on_bus_acquired(GDBusConnection *conn, const gchar *name, gpointer user_data)
{
    printf("Acquired bus \"%s\"\n", name);

    IoMangohHelloWorld *interface = io_mangoh_hello_world_skeleton_new();
    g_signal_connect(interface, "handle-hello-world", G_CALLBACK(on_handle_hello_world), NULL);

    GError *error = NULL;
    if (!g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(interface), conn, "/io/mangoh/GDBUS", &error)) {
        printf("Couldn't export skeleton\n");
        exit(1);
    }
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

    const guint interval_ms = 5 * 1000;
    unsigned int timer_count = 3;
    const guint timeout_event_id = g_timeout_add_full(
        G_PRIORITY_DEFAULT, interval_ms, timeout_printer, &timer_count, terminate);
    printf(
        "Registered timer with id=%d, timeout=%d, count=%d\n",
        timeout_event_id,
        interval_ms,
        timer_count);

    const guint name_ref = g_bus_own_name(
        G_BUS_TYPE_SESSION,
        "io.mangoh",
        G_BUS_NAME_OWNER_FLAGS_NONE,
        on_bus_acquired,
        on_name_acquired,
        on_name_lost,
        NULL,
        NULL);

    // TODO: by passing NULL as the first parameter, we are setting the GMainContext* as the
    // "default context". What is the purpose of this context parameter?
    _.loop = g_main_loop_new(NULL, FALSE);
    puts("Running the main loop");
    g_main_loop_run(_.loop);
    puts("The main loop has completed");

    return 0;
}
