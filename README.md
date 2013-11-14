# Pebble VLC Remote

Pebble VLC Remote is a remote control app for VLC Media Player that uses VLC's native web interface API and Pebble SDK 2.0's JavaScript Framework for HTTP requests from Pebble.

For now, VLC server IP/port are hard coded, but will soon update to set that via the new in-app configuration.

## Features

* Play/pause with the select button.
* Volume up 5% with the up button.
* Volume down 5% with the down button.
* Display filename when available.

## Configuration

Modify `src/vlcremote.c` with your own VLC server IP/port and password.

## Install

	$ pebble build
	$ pebble install
