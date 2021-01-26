# riow

Some kind of window management experiments with rio.  What does it do?
It gives you virtual desktops to move window around to, by pressing
keys, like i3. The window management code is written in `rc` and is done
by reading and writing `/dev/wsys` for the most part. There are minimal
changes to `rio` (to provide an additional `/srv/riogkbd.*` file).

*No guarantees, use at your own risk and blah.  This isn't supposed to
work with drawterm.*

## Features

 * virtual desktops from 1 to 0
 * switch between desktops
 * move windows between desktops
 * toggle fullscreen on the current window
 * start a new window
 * "sticky" programs, by default `stats`, `kbmap` etc are shown on every desktop
 * toggle "sticky" mode for current window

All that with simple shortcuts.

## Installation and usage

Run `mk install` in this repo.  Apply `9front.diff` on your sources
 (`hg import --no-commit 9front.diff`), rebuild `rio`.

To start `riow`, either through `riostart`, or by hand:

	window -hide -scroll riow

Modify `riow` to your own needs.

## Keys

```
Mod4-f              toggle fullscreen for the current window
Mod4-s              toggle "sticky" mode for the current window
Mod4-enter          start a new window
Mod4-[0..9]         switch to a specific virtual desktop
Mod4-shift-[0..9]   move the current window to a specific virtual desktop
```

## Extras

[Rio theming](http://ftrv.se/14)

[bar](https://git.sr.ht/~ft/bar)
