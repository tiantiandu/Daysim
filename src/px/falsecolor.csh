#!/bin/csh -f
# SCCSid "$SunId$ LBL"
#
# Create false color image with legend
#
set tempdir=/usr/tmp/fc$$
onintr quit
set mult=470
set label=Nits
set scale=1000
set redv='2*v-1'
set grnv='if(v-.5,2-2*v,2*v)'
set bluv='1-2*v'
set ndivs=8
set picture='-'
set cpict=
while ($#argv > 0)
	switch ($argv[1])
	case -m:
		shift argv
		set mult="$argv[1]"
		breaksw
	case -s:
		shift argv
		set scale="$argv[1]"
		breaksw
	case -l:
		shift argv
		set label="$argv[1]"
		breaksw
	case -r:
		shift argv
		set redv="$argv[1]"
		breaksw
	case -g:
		shift argv
		set grnv="$argv[1]"
		breaksw
	case -b:
		shift argv
		set bluv="$argv[1]"
		breaksw
	case -i:
		shift argv
		set picture="$argv[1]"
		breaksw
	case -p:
		shift argv
		set cpict="$argv[1]"
		breaksw
	case -ip:
	case -pi:
		shiftargv
		set picture="$argv[1]"
		set cpict="$argv[1]"
		breaksw
	case -cl:
		set docont=a
		breaksw
	case -cb:
		set docont=b
		breaksw
	case -n:
		shiftargv
		set ndivs="$argv[1]"
		breadsw
	default:
		echo bad option "'$argv[1]'"
		exit 1
	endsw
	shift argv
end
mkdir $tempdir
cat > $tempdir/pc.cal <<_EOF_
scale : $scale ;
mult : $mult ;
ndivs : $ndivs ;

or(a,b) : if(a,a,b);
EPS : 1e-7;
neq(a,b) : if(a-b-EPS,1,b-a-EPS);
btwn(a,b) : if(a-x,-1,b-x);
frac(x) : x - floor(x);
boundary(a,b) : neq(floor(ndivs*a),floor(ndivs*b));

red=$redv;
grn=$grnv;
blu=$bluv;

v = li(1)*(mult/scale);
vleft = li(1,-1,0)*(mult/scale);
vright = li(1,1,0)*(mult/scale);
vabove = li(1,0,1)*(mult/scale);
vbelow = li(1,0,-1)*(mult/scale);
isconta = or(boundary(vleft,vright),boundary(vabove,vbelow));
iscontb = if(btwn(0,v,1),btwn(.4,frac(ndivs*v),.6),0); 

ro = if(in,red,ra);
go = if(in,grn,ga);
bo = if(in,blu,ba);

ra = ri(nfiles);
ga = gi(nfiles);
ba = bi(nfiles);

in = 1;
_EOF_
set pcargs=(-o -f $tempdir/pc.cal)
if ($?docont) then
	set pcargs=($pcargs -e "in=iscont$docont")
endif
if ("$cpict" == "") then
	set pcargs=($pcargs -e 'ra=0;ga=0;ba=0')
else if ("$cpict" == "$picture") then
	set cpict=
endif
pcomb $pcargs -e 'v=(y+.5)/200;vleft=v;vright=v' \
		-e 'vbelow=(y-.5)/200;vabove=(y+1.5)/200' \
		-e 'ra=0;ga=0;ba=0' -x 100 -y 200 \
		> $tempdir/scol.pic
(echo $label; cnt $ndivs |rcalc -e '$1='"($scale)/$ndivs*($ndivs"'-.5-$1)') \
	| psign -cf 1 1 1 -cb 0 0 0 -h `ev "floor(200/$ndivs+.5)"` \
		> $tempdir/slab.pic
pcomb $pcargs $picture $cpict \
	| pcompos $tempdir/scol.pic 0 0 -t .5 $tempdir/slab.pic 25 0 - 100 0
quit:
rm -rf $tempdir
