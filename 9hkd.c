#include <u.h>
#include <libc.h>
#include <draw.h>
#include <bio.h>
#include <ndb.h>
#include <keyboard.h>
#include <ctype.h>

typedef struct W W;

enum {
    Mmod4 = 1<<0,
    Mctl = 1<<1,
	Mshift = 1<<2,

	Step = 16,
	Stepbig = 64,
	Threshold = 64,

	Fcurrent = 1<<0,
	Ffullscreen = 1<<1,
	Fresizable = 1<<2,
	Fvisible = 1<<3,

};

struct W {
    int id;
    Rectangle r;
    Rectangle cap;
    int tg;
    int flags;
	char label[32];
};

static Point *pts;
static int npoints;
/*static int gap;*/
static W *ws, *wcur;
static Ndb *db;
static int wsn;
static int wsys; /* /dev/wsys fd */
static int mod; /* modmask mode currently */
static void wsupdate(void);
static void binpack(void);

static int
wwctl(int id, int mode)
{
	char s[64];

	snprint(s, sizeof(s), "/dev/wsys/%d/wctl", id);

	return open(s, mode);
}

static struct {
	int x, y;
}cyclectx;

static int
cyclecmp(void *a_, void *b_)
{
	W *a = a_, *b = b_;

	return cyclectx.x*(a->r.min.x - b->r.min.x) + cyclectx.y*(a->r.min.y - b->r.min.y);
}

static void
cycleaction(int x, int y)
{
	int wcurid, i, f;
	W *w, *w₀;

	wsupdate();
	wcurid = wcur == nil ? -1 : wcur->id;
	cyclectx.x = x;
	cyclectx.y = y;
	qsort(ws, wsn, sizeof(*ws), cyclecmp);
	w₀ = nil;
	wcur = nil;
	for(i = 0, w = ws; i < wsn; i++, w++){
		if(w->id == wcurid){
			wcur = w;
			continue;
		}
		if(w₀ == nil)
			w₀ = w;
		if(wcur != nil)
			break;
	}
	if(i >= wsn)
		w = w₀;
	if(w == nil || (f = wwctl(w->id, OWRITE)) < 0)
		return;
	fprint(f, "top");
	fprint(f, "current");
	close(f);
	wcur = w;
}

static void
wsupdate(void)
{
	int i, k, n, f, tn, dsn;
	char s[256], *t[8];
	Ndbtuple *nt;
	Ndbs ndbs;
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
		w->tg = -1;
		w->flags = 0;

		/* Get old values */
		for(k = 0; k < wsn; k++){
			if(ws[k].id == w->id){
				w->tg = ws[k].tg;
				snprint(w->label, sizeof(ws[k].label), ws[k].label);
				w->flags = ws[k].flags & ~(Fvisible|Fcurrent);
				if(w->flags &  Ffullscreen)
					w->r = ws[k].r;
				break;
			}
		}
		for(k = 4; k < tn; k++){
			if(strcmp(t[k], "current") == 0){
				w->flags |= Fcurrent;
				w->flags |= Fvisible;
				wcur = w;
				break;
			} else if(strcmp(t[k], "visible") == 0){
				w->flags |= Fvisible;
			}
		}

		snprint(s, sizeof(s), "/dev/wsys/%d/label", w->id);
		if((f = open(s, OREAD)) >= 0){
			n = read(f, s, sizeof(s)-1);
			close(f);
			if(n > 0){
				s[n] = 0;
				/* Label is different, parse */
				if(strcmp(w->label, s) != 0){
					w->cap.min.x = 0;
					w->cap.min.y = 0;
					w->cap.max.x = 0;
					w->cap.max.y = 0;
					w->tg = -1;
					/* Parse min/max, resizable flag, and tag group for label */
					nt = ndbsearch(db, &ndbs, "label", s);
					for(; nt != nil; nt = nt->entry){
						if(strcmp(nt->attr, "maxx") == 0)
							w->cap.max.x = atoi(nt->val);
						if(strcmp(nt->attr, "maxy") == 0)
							w->cap.max.y = atoi(nt->val);
						if(strcmp(nt->attr, "minx") == 0)
							w->cap.min.x = atoi(nt->val);
						if(strcmp(nt->attr, "miny") == 0)
							w->cap.min.y = atoi(nt->val);
						if(strcmp(nt->attr, "tag") == 0)
							w->tg = atoi(nt->val);
					}
					ndbfree(nt);
					if(w->cap.min.x == 0 && w->cap.min.y == 0 && w->cap.max.x == 0 && w->cap.max.y == 0){
						w->flags |= Fresizable;
						w->cap.min.y = screen->r.min.y / 2;
						w->cap.max.y = screen->r.max.y;
						w->cap.min.x = w->r.min.y;
						w->cap.max.x = w->r.max.x;
					}
				}
			}
			snprint(w->label, sizeof s, s);
		}
		w++;
	}
	
	free(ds);
	free(ws);
	ws = newws;
	wsn = w - newws;
}

static void
binpack(void)
{
	Rectangle r, rt;
    /**
     * [x] Sort wins
	 * [ ] set bounds.min to win1
	 * [ ] start walking down point array
	 * [ ] check if arena can accommodate, check virt then check horizontal
	 * [ ] adjust rt.min.x + rt.min.y as needed and attempt pack
	 * [ ] set r = rt if we fit
	 * [ ] update points, if Pn+2 is up and to left by under Threshold in each direction, remove points
     */
	qsort(ws, wsn, sizeof(*ws), cyclecmp);
	r = screen->r;
	r.min.y = ws->cap.min.y;
	r.min.x = ws->cap.min.x;
	
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

	wsupdate();
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
widthaction(int x)
{
	int f;

	wsupdate();
	if(wcur == nil || (f = wwctl(wcur->id, OWRITE)) < 0)
		return;

	if((wcur->flags & Fresizable) == 0)
		return;

	x *= (mod & Mctl) ? Stepbig : Step;
	fprint(f, "resize -maxx %+d -minx %+d", x, -x);
	close(f);
	binpack();
}

static void
tagaction(int nvd)
{
	int f, wcurf;
	W *w;
	wsupdate();
	if((mod & Mshift) != 0){
		wcur->tg = nvd;
	} else {
		for(w = ws; w < ws+wsn; w++){
			if(w->tg == nvd){
				if((f = wwctl(w->id, OWRITE)) >= 0){
					fprint(f, w->flags & Fvisible ? "hide" : "unhide");
					w->flags ^= Fvisible;
					close(f);
				}
				/* Make sure we keep a visible window */
				if(wcur == nil || w->id == wcur->id && (wcur->flags & Fvisible) == 0)
					cycleaction(1, 0);
			}
		}
		binpack();
	}
}

static void
closewindow(void)
{
	int f;

	wsupdate();
	if(wcur == nil || (f = wwctl(wcur->id, OWRITE)) < 0)
		return;
	fprint(f, "delete");
	close(f);
	binpack();
	/* Try to focus the next visible window */
	cycleaction(1, 0);
}

static int
keyevent(char c, Rune r)
{
	if(c == 'c' && (mod & Mmod4) != 0){
		/* TODO: This becomes a lookup */
		switch(r) {
		case 'n': spawn("fmenu"); break;
		case 'd': spawn("dmenu"); break;
		case 't': spawn("window"); break;
		case 'c': closewindow(); break;
		case 'f': togglefullscreen(); break;
		case 'h': cycleaction(-1, 0); break;
		case 'l': cycleaction(1, 0); break;
		case 'j': cycleaction(0, 1); break;
		case 'k': cycleaction(0, -1); break;
		default:
			if (r >= '0' && r <= '9'){
				tagaction(r - '0');
				return 0;
			}
			return -1;
		}
		return 0;
	}
	if(c == 'k' && (mod & Mshift) == 0)
		return 0;
	if(c == 'k' && mod == (Mmod4|Mshift)) {
		if (r == 'l' || r == 'L'){
			widthaction(1);
			return 0;
		}
		if (r == 'h' || r == 'H'){
			widthaction(-1);
			return 0;
		}
		if (r >= '0' && r <= '9'){
			tagaction(r - '0');
			return 0;
		}
	}

	/* don't bother to properly deal with handling shifted digit keys */
	if((mod & Mshift) != 0 && (c == 'c' || c == 'k' || c == 'K'))
		return ispunct(r) ? 0 : -1;

	return -1;
}

static int
process(char *s)
{
	char b[128], *p;
	int n, o;
	Rune r;

	o = 0;
	b[o++] = *s;
	if(*s == 'k' || *s == 'K'){
		mod = 0;
		if(utfrune(s+1, Kmod4) != nil)
			mod |= Mmod4;
		if(utfrune(s+1, Kctl) != nil)
			mod |= Mctl;
		if(utfrune(s+1, Kshift) != nil)
			mod |= Mshift;
	}

	for(p = s+1; *p != 0; p += n){
		if((n = chartorune(&r, p)) == 1 && r == Runeerror){
			/* bail out */
			n = strlen(p);
			memmove(b+o, p, n);
			o += n;
			break;
		}

		if((mod & Mmod4) == 0 || keyevent(*s, r) != 0){
			memmove(b+o, p, n);
			o += n;
		}
	}

	b[o++] = 0;
	return (o > 1 && write(1, b, o) <= 0) ? -1 : 0;
}

static void
usage(void)
{
	fprint(2, "usage: %s [-d wdb]\n", argv0);
	exits("usage");
}

void
main(int argc, char **argv)
{
	char b[128], s[64];
	int i, j, n;

	ARGBEGIN{
	case 'd':
		s = EARGF(usage());
		break;
	}ARGEND

	if((wsys = open("/dev/wsys", OREAD)) < 0)
		sysfatal("%r");

	if(s == nil)
		snprint(s, sizeof(s), "%s/lib/wdb", getenv("home"));
	db = ndbopen(s);
	if(db == nil)
		sysfatal("no db found");

	/* Set our initial state */
	wsupdate();
	//printtags();
	
	for(i = 0;;){
		if((n = read(0, b, sizeof(b)-1)) <= 0)
			break;
		b[n] = 0;
		if(process(b) != 0)
			break;
	}
	exits(nil);
}

