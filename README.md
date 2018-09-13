# lavapt
lavapt - a Vulkan Installable Client Driver that redirects calls to another device

## What the hell is this?
This is an ICD that "simply" redirects Vulkan calls to another device.

## Why?
I have a desktop project that uses Vulkan, but my laptop is way too old to even think on
handling it. On the other hand, my phone is new enough so its SoC supports the shiny API.

This is a huge jerry rig, and absolutely will never reach realtime speeds (unless we find
a way to mimic PCIe speeds through sockets)!

## Architecture
The client is the ICD itself. The Vulkan Loader calls it, and it redirects it to the server.

The server runs on the Vulkan-capable device. It handles the requests, executes them, and
sends the answers.

All the communication is done via sockets. Since the server, for now, runs on Android, the
`adb forward` command will be used to forward sockets through USB.
