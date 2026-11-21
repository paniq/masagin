# 03_mysticdub

bpm 130
song "03_mysticdub.ogg"
bgcolor 1,1,1

fader blowup:
	type spring
	k 2000
	mass 1.0
	friction 0.7
	
fader slide:
	type spring
	k 100
	mass 4.0
	friction 0.7

fader colorfade:
	type sine

fader logf:
	type log
	exp1 0
	exp2 1

fader slurp:
	type polynome
	exp1 -0.5
	exp2 -0.5

verb colorify:
	move color 0,0,0.4,0 -> 0,0,0,0 time 1b loop
	move color 0,0,0,0 -> 0,0,-0.1,0 time 4t loop
	move color 0,0,0 -> 1,0,0 time 4b loop
	#~ move color 0.02,0,0,0 -> 0.12,0,0,0 time 4b loop fader colorfade
	#~ move color -0.16,0,0,0 -> 0.083,0,0,0 time 2.79b loop fader colorfade

verb grow:
	like colorify
	
	move -0.5,-0.5
	scale 0%->50% time 2b fader slide
	#rotate 0°->360° time 16b
	move 0.5,0.5
	mul root
	mul bud
	mul parent_model
	
	move modelcam -0.5,-0.5
	scale modelcam 50%
	scale modelcam 100%->1600% time 4b fader slurp
	move modelcam 0.5,0.5
	mul root modelcam
	mul bud modelcam
	mul parent_model modelcam
	scale color 1,1,1,1 -> 1,1,1,0 time 5b->6b
	lifespan 6b
	
verb grow3:
	like grow
	move cam 0,0,1 -> 1,1,1 seg 1b fader slurp
	move cam 1,1,1 -> 0,0,1 seg 1b->3b fader slurp

verb grow2:
	like grow
	move cam 0,0,1 -> 1,1,1 seg 1b fader slurp
	move cam 1,1,1 -> 0,0,1 seg 1b->2b fader slurp

verb growc:
	like colorify
	
	move -0.5,-0.5
	scale 0%->50% time 2b fader slide
	rotate 0°->120° time 5b
	move 0.5,0.5
	mul root
	mul bud
	mul parent_model
	
	move modelcam -0.5,-0.5
	scale modelcam 50%
	scale modelcam 100%->1600% time 4b fader slurp
	rotate modelcam 0°->-120° time 5b
	move modelcam 0.5,0.5
	mul root modelcam
	mul bud modelcam
	mul parent_model modelcam
	scale color 1,1,1,1 -> 1,1,1,0 time 5b->6b
	lifespan 6b
	
verb grow3c:
	like growc
	move cam 0,0,1 -> 1,1,1 seg 2b
	move cam 1,1,1 -> 0,0,1 seg 2b->4b

verb grow4c:
	like growc
	move cam 0,0,1 -> 1,1,1 seg 1b
	move cam 1,1,1 -> 0,0,1 seg 1b->4b

verb grow2c:
	like growc
	move cam 0,0,1 -> 1,1,1 seg 1b
	move cam 1,1,1 -> 0,0,1 seg 1b->2b

verb grow5c:
	like growc
	move cam 0,0,1 -> 1,1,1 seg 1b
	move cam 1,1,1 -> 0,0,1 seg 1b->3b
	
verb grow6c:
	like growc
	move cam 0,0,1 -> 1,1,1 seg 2b
	move cam 1,1,1 -> 0,0,1 seg 2b->3b
	
verb gyrate:
	mul root
	rotate 0° -> 360° time 8b loop
	mul bud
	mul parent_model
	move cam 0,0,1 -> 0,1,1 seg 1b
	move cam 0,1,1 -> 0,1,1 seg 1b->32b
	lifespan 32b

alias kick mysticdub3
alias whosh mysticdub
alias snare mysticdub2
alias kick2 spermdub
alias whosh2 dotswirldub
alias snare2 swirldub

const D -2t

verb wind:
	like colorify
	move -0.5,-0.5
	scale 0%->50% time 2b fader slide
	move 0.5,0.5
	mul root
	mul bud
	mul parent_model
	
	move modelcam -0.5,-0.5
	rotate modelcam 0°->360° time 20b
	rotate modelcam 0°->(360°*8) time 20b fader logf
	scale modelcam 20%
	scale modelcam 50%->25% time 16b fader slurp
	move modelcam 0.5,0.5
	mul root modelcam
	mul bud modelcam
	mul parent_model modelcam
	scale color 1,1,1,1 -> 1,1,1,0 time 18b->20b
	lifespan 20b
	move cam 0,0,1 -> 1,1,1 seg 2b
	move cam 1,1,1 -> 1,1,1 seg 2b->16b
	move cam 1,1,1 -> 0,0,1 seg 16b->17b

verb colorblend:
	move color 0,0,1 -> 0,0,0 time 3b->4b
	set bgcolor color
	lifespan 16b

branch loop:
	grow3 kick2:
		colorblend none
		grow3 whosh2 delay p+(2b+D):
			grow2 kick delay p+2b:
				grow2 kick2 delay p+1b:
					grow3 snare delay p+1b

branch loop2:
	grow3 kick delay p+2b:
		grow3 whosh delay p+2b:
			grow2 kick delay p+2b:
				grow2 kick2 delay p+1b:
					grow3 snare delay p+1b

branch loop3:
	grow4c kick delay p+2b:
		grow6c whosh2 delay p+2b:
			grow2c kick2 delay p+2b:
				grow5c kick delay p+1b:
					grow3c snare2 delay p+1b

branch loop4:
	grow3c kick delay p+2b:
		grow6c whosh2 delay p+2b:
			grow2c kick delay p+2b:
				grow5c kick2 delay p+1b:
					grow3c snare2 delay p+1b

branch windup:
	wind kick delay p+2b

ref loop:
	ref loop2:
		ref loop2:
			ref loop2:
				ref loop3:
					ref loop4:
						ref windup
