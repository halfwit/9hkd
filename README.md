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

Run `mk install` in this repo.  Apply `9front.diff` and rebuild/reinstall `rio`:

	cat 9front.diff | @{cd /sys/src/cmd/rio && ape/patch -p5 && mk install}

As a matter of fact, you can copy the original `rio` directory
somewhere, apply the patch, and install under your user's `bin`
instead.

To start `riow`, either through `riostart`, or by hand:

	window -scroll riow

If you're *NOT* using [bar](https://git.sr.ht/~ft/bar), run with
`-hide` as well.

Modify `riow` to your own needs.

## Keys

```
Mod4-f              toggle fullscreen for the current window
Mod4-s              toggle "sticky" mode for the current window
Mod4-enter          start a new window
Mod4-[0..9]         switch to a specific virtual desktop
Mod4-shift-[0..9]   move the current window to a specific virtual desktop

Mod4-[↑↓←→]         move the window (big steps)
Mod4-shift-[↑↓←→]   move the window (small steps)

Mod4-ctrl-[↑↓←→]        drag bottom-right of the window (big steps)
Mod4-ctrl-shift-[↑↓←→]  drag bottom-right of the window (small steps)

Mod4-alt-[↑↓←→]        drag top-left of the window (big steps)
Mod4-alt-shift-[↑↓←→]  drag top-left of the window (small steps)
```

## Extras

[Rio theming](http://ftrv.se/14)

[bar](https://git.sr.ht/~ft/bar)
