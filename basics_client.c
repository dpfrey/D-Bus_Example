#include <stdio.h>
#include <stdlib.h>
#include <gio/gio.h>
#include "basics_gen.h"
#include "config.h"


int main(int argc, char **argv)
{
    printf(
        "basics_client v%d.%d.%d\n",
        Basics_VERSION_MAJOR,
        Basics_VERSION_MINOR,
        Basics_VERSION_SUB);
    GError *error = NULL;
    IoMangohHelloWorld *proxy = io_mangoh_hello_world_proxy_new_for_bus_sync(
        G_BUS_TYPE_SESSION,
        G_DBUS_PROXY_FLAGS_NONE,
        "io.mangoh",
        "/io/mangoh/GDBUS",
        NULL,
        &error);
    guint16 num_times_called;
    if (!io_mangoh_hello_world_call_hello_world_sync(
            proxy,
            "Dave",
            &num_times_called,
            NULL,
            &error)) {
        puts("Failed to call hello world function");
        exit(1);
    }
    printf("The number of times the function has been called is %d\n", num_times_called);

    g_object_unref(proxy);

    return 0;
}
