#include <stdio.h>
#include <stdlib.h>
#include <gio/gio.h>
#include "basics_gen.h"
#include "config.h"


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
    GDBusObjectManager *object_manager = object_manager_client_new_for_bus_sync(
        G_BUS_TYPE_SESSION,
        G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
        "io.mangoh",
        "/Testing",
        NULL, /* GCancellable */
        &error);
    if (!object_manager) {
        puts("Couldn't get object manager");
        exit(1);
    }

    gchar *name_owner = g_dbus_object_manager_client_get_name_owner(G_DBUS_OBJECT_MANAGER_CLIENT(object_manager));
    printf("name-owner: %s\n", name_owner);
    g_free(name_owner);

    GDBusInterface *greetable_intf = g_dbus_object_manager_get_interface(
        object_manager, "/Testing/Greeter", "io.mangoh.Testing.Greetable");
    if (!greetable_intf) {
        printf("Couldn't get greetable interface\n");
        exit(1);
    }
    Greetable *greetable = GREETABLE(greetable_intf);
    g_signal_connect(greetable, "timer-expired", G_CALLBACK(on_handle_timer_expired), NULL);
    gchar *old_greeting = greetable_dup_greeting(greetable);
    printf("The old greeting was: %s\n", old_greeting);
    greetable_set_greeting(greetable, "Salutations");
    guint16 num_times_called;
    if (!greetable_call_greet_sync(
            greetable,
            "Dave",
            &num_times_called,
            NULL,
            &error)) {
        puts("Failed to call greet function");
        exit(1);
    }
    printf("The number of times the function has been called is %d\n", num_times_called);

    // Restore the old greeting
    greetable_set_greeting(greetable, old_greeting);
    g_free(old_greeting);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    g_object_unref(greetable);
    g_object_unref(object_manager);
    g_main_loop_unref(loop);

    return 0;
}
