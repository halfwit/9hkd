#include <u.h>
#include <libc.h>
#include <draw.h>
#include <keyboard.h>

typedef struct W W;

enum {
	Mmod4 = 1<<0,
	Mctl = 1<<1,
	Mshift = 1<<2,

	Step = 16,
	Stepbig = 64,

	Fvisible = 1<<0,
	Fcurrent = 1<<1,
	Fsticky = 1<<2,
	Ffullscreen = 1<<3,
};

struct W {
	int id;
	Rectangle r;
	int vd;
	int flags;
};

static int vd = 1; /* current virtual desktop */
static int wsys; /* rios /dev/wsys fd */
static int mod;
static W *ws, *wcur;
static int wsn;

static char *sticky[] = {
	"bar", "cat clock", "clock", "faces", "kbmap", "stats", "winwatch",
};

static int
wwctl(int id, int mode)
{
	char s[64];

	snprint(s, sizeof(s), "/dev/wsys/%d/wctl", id);

	return open(s, mode);
}

static void
wsupdate(void)
{
	int i, k, n, f, seen, tn, dsn;
	char s[256], *t[8];
	W *newws, *w;
	Dir *ds, *d;

	seek(wsys, 0, 0);
	if((dsn = dirreadall(wsys, &ds)) < 0)
		sysfatal("/dev/wsys: %r");

	newws = malloc(sizeof(W)*dsn);
	wcur = nil;
	for(i = 0, d = ds, w = newws; i < dsn; i++, d++){
		if((f = wwctl(atoi(d->name), OREAD)) < 0)
			continue;
		n = read(f, s, sizeof(s)-1);
		close(f);
		if(n < 12)
			continue;
		s[n] = 0;
		if((tn = tokenize(s, t, nelem(t))) < 6)
			continue;

		w->id = atoi(d->name);
		w->r.min.x = atoi(t[0]);
		w->r.min.y = atoi(t[1]);
		w->r.max.x = atoi(t[2]);
		w->r.max.y = atoi(t[3]);
		w->vd = -1;
		w->flags = 0;

		/* move over the current state of the window */
		for(k = 0, seen = 0; k < wsn; k++){
			if(ws[k].id == w->id){
				w->vd = ws[k].vd;
				w->flags = ws[k].flags & ~Fvisible;
				if(w->flags & Ffullscreen)
					w->r = ws[k].r;
				seen = 1;
				break;
			}
		}

		/* update current state */
		for(k = 4; k < tn; k++){
			if(strcmp(t[k], "current") == 0){
				w->flags |= Fcurrent;
				wcur = w;
			}else if(strcmp(t[k], "visible") == 0){
				w->flags |= Fvisible;
			}
		}

		if(!seen){
			/* not seen previously - set the new state for it */
			w->vd = vd;
			snprint(s, sizeof(s), "/dev/wsys/%d/label", w->id);
			if((f = open(s, OREAD)) >= 0){
				n = read(f, s, sizeof(s)-1);
				close(f);
				if(n > 0){
					s[n] = 0;
					for(k = 0; k < nelem(sticky); k++){
						if(strcmp(sticky[k], s) == 0){
							w->flags |= Fsticky;
							break;
						}
					}
				}
			}
		}
		w++;
	}

	free(ds);
	free(ws);
	ws = newws;
	wsn = w - newws;
}

static void
spawn(char *s)
{
	if(rfork(RFPROC|RFNOWAIT|RFNAMEG|RFENVG|RFCFDG|RFREND) == 0)
		execl("/bin/rc", "rc", s, nil);
}

static void
togglefullscreen(void)
{
	int f;

	if(wcur == nil || (f = wwctl(wcur->id, OWRITE)) < 0)
		return;
	wcur->flags ^= Ffullscreen;
	if(wcur->flags & Ffullscreen)
		fprint(f, "resize -r 0 0 9999 9999");
	else
		fprint(f, "resize -r %d %d %d %d", wcur->r.min.x, wcur->r.min.y, wcur->r.max.x, wcur->r.max.y);
	close(f);
}

static void
togglesticky(void)
{
	if(wcur != nil)
		wcur->flags ^= Fsticky;
}

static void
vdaction(int nvd)
{
	int f, wcurf;
	W *w;

	if(mod == Mmod4){
		wcur = nil;
		wcurf = -1;
		for(w = ws; w < ws+wsn; w++){
			if((f = wwctl(w->id, OWRITE)) < 0)
				continue;
			if(w->flags & Fvisible)
				w->vd = vd;
			if(w->vd != nvd && (w->flags & Fsticky) == 0)
				fprint(f, "hide");
			else{
				fprint(f, "unhide");
				if((w->flags & Fcurrent) && wcurf < 0){
					wcur = w;
					wcurf = f;
					f = -1;
				}
			}
			if(f >= 0)
				close(f);
		}
		if(wcur != nil){
			fprint(wcurf, "top");
			fprint(wcurf, "current");
			close(wcurf);
		}
		vd = nvd;
		fprint(3, "%d\n", vd);
	}else if(mod == (Mmod4 | Mshift) && wcur != nil){
		if((f = wwctl(wcur->id, OWRITE)) >= 0){
			fprint(f, "hide");
			wcur->vd = nvd;
			wcur = nil;
			close(f);
		}
	}
}

static void
arrowaction(int x, int y)
{
	int f;

	if(wcur == nil || (f = wwctl(wcur->id, OWRITE)) < 0)
		return;

	x *= (mod & Mctl) ? Stepbig : Step;
	y *= (mod & Mctl) ? Stepbig : Step;
	if((mod & Mshift) == 0)
		fprint(f, "move -minx %+d -miny %+d", x, y);
	else
		fprint(f, "resize -maxx %+d -maxy %+d -minx %+d -miny %+d", x, y, -x, -y);
	close(f);
}

static void
keyevent(Rune r)
{
	wsupdate();

	if(r == '\n')
		spawn("window");
	else if(r == 'f')
		togglefullscreen();
	else if(r == 's')
		togglesticky();
	else if(r >= '0' && r <= '9')
		vdaction(r - '0');
	else if(r == Kup)
		arrowaction(0, -1);
	else if(r == Kdown)
		arrowaction(0, 1);
	else if(r == Kleft)
		arrowaction(-1, 0);
	else if(r == Kright)
		arrowaction(1, 0);
}

static void
process(char *s)
{
	char b[128], *p;
	int n, o;
	Rune r;

	if(*s == 'K' && s[1] == 0)
		mod = 0;

	o = 0;
	b[o++] = *s;
	for(p = s+1; *p != 0; p += n){
		if((n = chartorune(&r, p)) == 1 && r == Runeerror){
			/* bail out */
			n = strlen(p);
			memmove(b+o, p, n);
			o += n;
			p += n;
			break;
		}

		if(*s == 'c' && (mod & Mmod4) != 0){
			keyevent(r);
			continue;
		}

		if(*s == 'k'){
			if(r == Kmod4)
				mod |= Mmod4;
			else if(r == Kctl)
				mod |= Mctl;
			else if(r == Kshift)
				mod |= Mshift;
			else if(r >= '0' && r <= '9' && (mod & (Mshift|Mmod4)) == (Mshift|Mmod4)){
				keyevent(r);
				continue;
			}
		}else if(*s == 'K'){
			if(r == Kmod4)
				mod &= ~Mmod4;
			else if(r == Kctl)
				mod &= ~Mctl;
			else if(r == Kshift)
				mod &= ~Mshift;
		}

		memmove(b+o, p, n);
		o += n;
	}

	/* all runes filtered out - ignore completely */
	if(o == 1 && p-s > 1)
		return;

	b[o++] = 0;
	if(write(1, b, o) != o)
		exits(nil);
}

static void
usage(void)
{
	fprint(2, "usage: [-l light_step] [-v vol_step] %s\n", argv0);
	exits("usage");
}

void
main(int argc, char **argv)
{
	char b[128];
	int i, j, n;

	ARGBEGIN{
	default:
		usage();
	}ARGEND

	if((wsys = open("/dev/wsys", OREAD)) < 0)
		sysfatal("%r");

	/* initial state */
	wsupdate();
	for(i = 0; i < wsn; i++)
		ws[i].vd = vd;
	fprint(3, "%d\n", vd);

	for(i = 0;;){
		if((n = read(0, b+i, sizeof(b)-i)) <= 0)
			break;
		n += i;
		for(j = 0; j < n; j++){
			if(b[j] == 0){
				process(b+i);
				i = j+1;
			}
		}
		memmove(b, b+i, j-i);
		i -= j;
	}

	exits(nil);
}