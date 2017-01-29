#! /usr/bin/gawk -f

# parameter from csh
#  * selch  : selected channel
#  * com    : command for selected channel
#  * comoth : command for other channel
#  * target : target charges for threshold [fC]

BEGIN{
}

{
ch = $1
th = ceil($7*target+$5);
if     ( th < -31 ) th = -31
else if( th >  31 ) th = 31

printf( "%-3d    ", 127-$1)
printf "LLLLL"

if( th<0 ){ printf "L"; th = -th }
else      { printf "H" }

printf dec2bin(th);

if( ch==selch ) print com
else            print comoth
}

END{
}


function ceil(num){

    if( num > 0 ){
	return int(num+0.5);
    }else{
	return int(num-0.5);
    }
}

# dec2bin - translate decimal to binary
function dec2bin(num,rem,str) {
    cnt = 0;
    while(num > 0){
	rem = int(num % 2);
	if( rem==1 ){
	    str = "H" str;
	}else{
	    str = "L" str;
	}
	cnt += 1
	num = int(num / 2);
    }

    while( 5-cnt > 0 ){
	str = "L" str;
	cnt += 1;
    }
    return str;
}
