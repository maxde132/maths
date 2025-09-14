#!/usr/bin/env sh
if [ "$1" = "" ] || [ "$2" = "" ] || [ "$3" = "" ]; then
	echo "usage: $0 a b c"
	echo "solves a quadratic equation in the form 'ax^2+bx+c = 0'"
	exit
fi
build/mml -E 'x=(-$B+~csqrt{$B^2-4*$A*$C})/(2$A); print{x.0, x.1};' --set_var:A="$1" --set_var:B="$2" --set_var:C="$3"
