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

	mk install
	man riow

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

## Extras

[Rio theming](http://ftrv.se/14)

[bar](https://git.sr.ht/~ft/bar)
