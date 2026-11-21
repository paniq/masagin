

bgcolor 1,1,1

verb set:
	mul root modelcam
	move modelcam -0.5,-0.5
	scale modelcam 100% -> 300% time 16b
	move modelcam 0.5,0.5
	mul bud modelcam
	mul parent_model modelcam
	mul root
	mul bud
	mul parent_model
	focus 64b
	lifespan 64b

verb show:
	move -0.1,-0.1
	scale color 1,1,1,0.3
	lifespan 64b

verb mask:
	options zclear1+zwrite
	lifespan 64b

fader sin:
	type sine

verb fill:
	options color+alpha+zread
	move color 0.9,1.0,0.5
	move -0.5,-0.5
	rotate 0,0°,0 -> 0,360°,0 time 8b loop
	scale 0% -> 25% time 4t
	move 0.5,0.5
	move (random -2.0 to 1.5),(random -0.5 to 0.5)
	delay s+4t
	lifespan 64b

alias S maskfx

set none:
	show S
	mask S
	fill*40 S
