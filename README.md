# riow

Some kind of window management experiments with rio.  What does it do?
It gives you virtual desktops to move window around to, by pressing
keys, like i3. The window management code is written in `rc` and is done
by reading and writing `/dev/wsys` for the most part. There are minimal
changes to `kbdfs` (to add support for an extra key) and `rio` (to provide
an additional `/srv/riogkbd.*` file).

*No guarantees, use at your own risk and blah. This isn't supposed to work with drawterm.*

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

Apply `9front.diff` on your sources, rebuild `kbdfs` and `rio`.
Run `mk install` in this repo. You also need to rebuild your kernel
after so it picks up new `kbdfs`, reboot.

The `super/windows/meta/mod` keys are now called `glenda` here, duh.

To start the actual window management thingy, open a new window in
`rio` and type `riow < /srv/riogkbd*`. See *Keys* section and it you're
happy with everything you can start `riow` automatically next time.
Or not, you can use it when you need it.

Modify `riow` to your own needs.

## Keys

```
G-f              toggle fullscreen for the current window
G-s              toggle "sticky" mode for the current window
G-enter          start a new window
G-[0..9]         switch to a specific virtual desktop
G-shift-[0..9]   move the current window to a specific virtual desktop
```
