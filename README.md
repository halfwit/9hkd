# riow

Some kind of window management experiments with rio.

Apply `9front.diff`, run `mk install`.  It seems you also need to
rebuild your kernel after that since there are changes made to
`kbdfs`.

The "super"/"windows"/"meta" keys is called "glenda", duh.

```
What's all this racket going up here, son. Are you playing with your rio's color scheme again?
It's not COLORS, it's ADVANCED WINDOWS MANAGEMENT SYSTEM! Here, watch this:

gkbd < /srv/*gkbd* | awk '/k glenda enter/ { system("window"); }
```

## WARNING

No guarantees, use at your own risk and blah.
