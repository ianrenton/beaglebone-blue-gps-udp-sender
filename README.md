# Beaglebone Blue GPS UDP Sender
Reads lines from a serial port, and sends them via UDP to two ports. Maybe useful if you have several programs needing the data that can't both access the serial port, or you want to forward the data to another PC.

Pretty sure you can do this with `socat`, `netcat` and `tee` in some combination but here at least it's one process that can be restarted as needed with no hassle.

Apologies for code quality, it's been a while since I last wrote any C.

`make` does exactly what you expect. `make install` will put it in `/usr/local/bin` and install a systemd service for it to run in the background.
