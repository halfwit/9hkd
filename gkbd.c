#include <u.h>
#include <libc.h>
#include <keyboard.h>
#include <ctype.h>

static struct {
	Rune r;
	char *k;
}keys[] = {
	{'\t', "tab"},
	{0x0a, "enter"},
	{0x20, "space"},
	{Kalt, "alt"},
	{Kctl, "ctl"},
	{Kdel, "del"},
	{Kdown, "down"},
	{Kesc, "esc"},
	{Kmod4, "mod4"},
	{Kleft, "left"},
	{Kright, "right"},
	{Kshift, "shift"},
	{Kup, "up"},
};

void
main(int argc, char **argv)
{
	char k[32], *s, out[128];
	Rune r;
	int n, i;

	USED(argc); USED(argv);

	for(;;){
		s = k;
		if((n = read(0, k, sizeof(k)-1)) <= 0)
			break;
		k[n] = 0;
		n = 0;
		out[n++] = *s++;
		out[n++] = ' ';
		while(*s){
			s += chartorune(&r, s);
			if(r == Runeerror){
				n = -1;
				break;
			}

			for(i = 0; i < nelem(keys); i++){
				if(keys[i].r == r){
					n += sprint(out+n, "%s ", keys[i].k);
					break;
				}
			}

			if(i >= nelem(keys)){
				if(isprint(r))
					n += sprint(out+n, "%C ", r);
				else
					n += sprint(out+n, "0x%x ", r);
			}
		}

		if(n > 0){
			out[n-1] = '\n';
			if(write(1, out, n) != n)
				break;
		}
	}

	exits(nil);
}
