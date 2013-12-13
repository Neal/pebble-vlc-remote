# VLC Remote

VLC Remote is a remote control watch app for VLC Media Player for Pebble which allows you to toggle play/pause, change volume, seek, and get recent status of what's playing and how far you've watched - all right on your wrist!

It uses PebbleKit JS to make HTTP requests to VLC's native web interface API.

## Installation

* Stable release:

	Get the latest `pbw` from [releases](https://github.com/Neal/pebble-vlc-remote/releases).

* Compile your own:

		$ pebble build && pebble install

## Configuration

* Open Pebble mobile app and tap the gear icon to list all the watchapps.
* Tap VLC Remote to open the configuration page.
* Enter your VLC server host and password; then tap submit.

## Controls

| Button                                             | Function                  |
| -------------------------------------------------- | ------------------------- |
| Single click select                                | Toggle play/pause         |
| Long click select                                  | Switch between option 1/2 |
|                                                    |                           |
| Single click up <sub><sup>(option 1)</sup></sub>   | Increment volume 5%       |
| Single click down <sub><sup>(option 1)</sup></sub> | Decrement volume 5%       |
| Long click up <sub><sup>(option 1)</sup></sub>     | Set volume to 0%          |
| Long click down <sub><sup>(option 1)</sup></sub>   | Set volume to 200%        |
|                                                    |                           |
| Single click up <sub><sup>(option 2)</sup></sub>   | Seek +10 seconds          |
| Single click down <sub><sup>(option 2)</sup></sub> | Seek -10 seconds          |
| Long click up <sub><sup>(option 2)</sup></sub>     | Seek +1 minute            |
| Long click down <sub><sup>(option 2)</sup></sub>   | Seek -1 minute            |

## Features

* Toggle play/pause.
* Increment or decrement volume 5% and set it to lowest or highest.
* Seek +- 10 seconds and +- 1 minute.
* Title text layer on top displays "Refreshing..." until it fetches data, then to the filename VLC provides, or an error if one occurs.
* Status gets the status from VLC - should be "Playing", "Paused", "Stopped", or "Unknown" when not known.
* Volume is displayed in a progress bar and as text; both range from 0% to 200% (0% when unknown).
* Seek progress bar shows how far in the movie you are.

## Screenshots

![](http://f.cl.ly/items/46132p210B1F132L3q0H/vlc-remote-screenshot-1.png)&nbsp;&nbsp;
![](http://f.cl.ly/items/2O1J1I0S3U1z3U3M272E/vlc-remote-screenshot-2.png)&nbsp;&nbsp;
![](http://f.cl.ly/items/153x1F092u2n0P0K3R14/vlc-remote-screenshot-3.png)&nbsp;&nbsp;
![](http://f.cl.ly/items/2G1n27310o2h41310u2C/vlc-remote-screenshot-4.png)&nbsp;&nbsp;
