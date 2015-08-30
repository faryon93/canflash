#!/bin/bash
rmmod mcp251x && modprobe mcp251x && ip link set can0 up type can bitrate 100000
./uflash "$@"
rmmod mcp251x && modprobe mcp251x && ip link set can0 up type can bitrate 500000
