# DBus Example

This repository contains code to build a simple DBus client and server program. The client passes a
"greeting" to the server which will print "Hello [greeting]" and fill in an output parameter
indicating how many times the `HelloWorld` function has been called. The server also registers a
timer which will expire a few times and then terminate the server.

## How to Build
1. `mkdir build && cd build`
1. `cmake -G Ninja ..`
1. `ninja`

## How to Run
1. `./basics_server`
1. `./basics_client`

## Things I am Not
1. A DBus expert
1. A GLib/GIO/GObject expert
1. A CMake expert

What I'm trying to say is that this example works, but I can't guarantee that it's an idiomatic or
optimal implementation.
