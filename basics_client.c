#include <stdio.h>
#include <stdlib.h>
#include <gio/gio.h>
#include "basics_gen.h"
#include "config.h"

static struct {
    GMainLoop *loop;
} _;


static void on_handle_timer_expired(
    Greetable *object,
    guint16 remaining,
    gpointer user_data)
{
    printf("Received timer-expired signal with remaining=%d\n", remaining);
}

int main(int argc, char **argv)
{
    printf(
        "basics_client v%d.%d.%d\n",
        Basics_VERSION_MAJOR,
        Basics_VERSION_MINOR,
        Basics_VERSION_SUB);
    GError *error = NULL;
    Greetable *proxy = greetable_proxy_new_for_bus_sync(
        G_BUS_TYPE_SESSION,
        G_DBUS_PROXY_FLAGS_NONE,
        "io.mangoh",
        "/Testing/Greeter",
        NULL,
        &error);
    g_signal_connect(proxy, "timer-expired", G_CALLBACK(on_handle_timer_expired), NULL);
    gchar *old_greeting = greetable_dup_greeting(proxy);
    printf("The old greeting was: %s\n", old_greeting);
    greetable_set_greeting(proxy, "Salutations");
    guint16 num_times_called;
    if (!greetable_call_greet_sync(
            proxy,
            "Dave",
            &num_times_called,
            NULL,
            &error)) {
        puts("Failed to call greet function");
        exit(1);
    }
    printf("The number of times the function has been called is %d\n", num_times_called);

    // Restore the old greeting
    greetable_set_greeting(proxy, old_greeting);
    g_free(old_greeting);

    _.loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(_.loop);

    g_object_unref(proxy);

    return 0;
}
