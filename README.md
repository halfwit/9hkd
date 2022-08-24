# riow

Some kind of window management experiments with rio.  What does it do?
It gives you virtual desktops to move window around to, by pressing
keys, like i3.  The window management is done by reading and writing
`/dev/wsys` for the most part.

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

Run `mk install` in this repo.

`riow` uses `/dev/kbdtap` of rio and should be placed as *last* in the chain of
`kbdtap` users (after `ktrans` and/or `reform/shortcuts`).

Running it alone:

	riow </dev/kbdtap >/dev/kbdtap >[3]/dev/null

Note that the current desktop number is printed to fd 3.

Running with [bar](https://git.sr.ht/~ft/bar) is recommended.  For
example, with additional `zuke(1)` controls and `reform/shortcuts`:

	; cat $home/rc/bin/Bar
	#!/bin/rc
	rfork ne
	
	fn bar {
		sed -u 's/$/ │ ⏮ │ ⏯ │ ⏭/g' \
		| /bin/bar \
		| awk -v 'c=plumb -d audio ''key ' '
			/⏮/{system(c"<''")}
			/⏯/{system(c"p''")}
			/⏭/{system(c">''")}
			' >[2]/dev/null
	}
	</dev/kbdtap reform/shortcuts | riow >/dev/kbdtap |[3] bar

This can be running on rio startup by adding `window Bar` to your `riostart` script.

## Keys

```
Mod4-f              toggle fullscreen for the current window
Mod4-s              toggle "sticky" mode for the current window
Mod4-enter          start a new window
Mod4-[0..9]         switch to a specific virtual desktop
Mod4-shift-[0..9]   move the current window to a specific virtual desktop

Mod4-[↑↓←→]         move the window (small steps)
Mod4-ctrl-[↑↓←→]    move the window (big steps)

Mod4-shift-[↑↓←→]       resize the window (small steps)
Mod4-shift-ctrl-[↑↓←→]  resize the window (big steps)
```

## Extras

[Rio theming](http://ftrv.se/14)

[bar](https://git.sr.ht/~ft/bar)
