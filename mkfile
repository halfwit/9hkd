</$objtype/mkfile

TARG=gkbd
BIN=/$objtype/bin

OFILES=\
	gkbd.$O\

default:V:	all

</sys/src/cmd/mkone

install:V: $BIN/$TARG $BIN/riow

$BIN/riow: riow
	cp $prereq $target
