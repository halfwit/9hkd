#include <u.h>
#include <libc.h>
#include <keyboard.h>
#include <ctype.h>

static struct {
	Rune r;
	char *k;
}keys[] = {
	{Kglenda, "glenda"},
	{Kshift, "shift"},
	{Kctl, "ctl"},
	{Kalt, "alt"},
	{Kleft, "left"},
	{Kright, "right"},
	{Kdown, "down"},
	{Kup, "up"},
	{10, "enter"},
	{Kdel, "del"},
	{0x20, "space"},
	{Kesc, "esc"},
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
			if(r == 0xfffd){
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
				if(isalnum(r))
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
