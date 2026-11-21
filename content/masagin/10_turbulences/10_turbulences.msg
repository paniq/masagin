# 10_turbulences

bpm 130
song "10_turbulences.ogg"
bgcolor 1,0,1

alias S turbu

fader lfade:
	type log
	exp1 1
	exp2 0.1

fader lfade2:
	type log
	exp1 0.1
	exp2 1
	
fader lfade3:
	type log
	exp1 1
	exp2 0
	
fader spr:
	type spring
	k 1000.0
	mass 10.0
	friction 0.7

fader sl2a:
	type sine
	phase 0.3333
	cycles 8
fader sl2b:
	type sine
	phase 0.6666
	cycles 8
fader sl2c:
	type sine
	cycles 8

fader sl:
	type sine
	cycles (32*3*10)

verb g:
	options color+alpha+subcolor+addcolor
	
	mul root
	rotate 0° -> 90° time 64b
	scale 0->8 time 8b fader lfade
	mul bud
	mul parent_model
	
	mul root modelcam
	mul bud modelcam
	mul parent_model modelcam
	
	scale color 1,1,1,0 -> 1,1,1,0 seg 32b->64b
	
	move cam 1,0 seg 64b
	lifespan 64b

verb e:
	options color+alpha+subcolor+addcolor
	rotate 0° -> -90° time 64b
	scale 1 -> 0 time 31b->32b
	lifespan 32b

verb er:
	options color+alpha+addcolor
	rotate 0° -> -90° time 32b
	move color 0,1,-0.5
	scale 0.9->1 time 32b fader sl2a
	scale 1->16 seg 31b->32b
	lifespan 32b

verb eg:
	options color+alpha+addcolor
	rotate 0° -> 90° time 32b
	move color 0.3333,1,-0.5
	scale 0.9->1 time 32b fader sl2b
	scale 1->16 seg 31b->32b
	lifespan 32b

verb eb:
	options color+alpha+addcolor
	rotate 0° -> -30° time 32b
	move color 0.6666,1,-0.5
	scale 0.9->1 time 32b fader sl2c
	scale 1->16 seg 31b->32b
	lifespan 32b

verb f1:
	options color+alpha+subcolor+addcolor

verb f2:
	options color+alpha+subcolor

verb f3:
	options color+alpha+addcolor

verb fcube:
	like f3
	rotate 0°->(-360°*2) time 16b
	scale 2%
	lifespan 4b
	
verb fc2:
	like f1
	move 0,0->2.5,0 time 1b fader spr
	rotate 0°->30° time 1b
	scale 90%
	delay p+3t
	lifespan 4b

verb fc3:
	like f1
	mul root
	move 0,0->2.5,0 time 1b fader spr
	rotate 180°
	rotate 0°->30° time 1b
	scale 90%
	mul bud
	mul parent_model
	delay p+3t
	lifespan 4b

verb fc4:
	like f1
	rotate 0->10° time 0b->24b
	scale 1%->90% time 1b fader spr
	delay p+3t
	lifespan 4b

branch cube:
	fcube S.cube delay 16b:
		rule fcube S.cube -> branch:
			fc2 S.cube
			fc3 S.cube
		rule fc2 S.cube -> branch:
			fc2 S.cube
		rule fc3 S.cube -> branch:
			fc2 S.cube
		iterate 32

verb fcore1: 
	like f1
	rotate (random 0° to 360°)
	rotate 0°->-360° time 4b
	scale 1,1,(random 10% to 20%) uniform
	scale 0->1 time 2b fader lfade
	scale 1->0 time (3b+8t)->4b
	lifespan 4b

verb fcore: 
	like f1
	scale 1->0 time 2b fader lfade2
	rotate 0°->360° time 2b
	delay p+2b
	lifespan 2b

branch arrows:
	fcore1 S.coreball delay p+0b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball
	fcore1 S.coreball delay p+s+2b:
		fcore S.coreball

verb flash2:
	like f2
	scale 1,1,(random 1% to 20%) uniform
	rotate (random 0° to 360°)
	lifespan 3t

verb flash3:
	like f3
	scale 1,1,(random 1% to 20%) uniform
	rotate (random 0° to 360°)
	lifespan 3t

verb flash4:
	like f1
	scale 1,1,(random -10% to 10%) uniform fader spr
	scale 1 -> 0 time (1b+8t)->2b
	rotate (random 0° to 360°)
	rotate 0°->(random 180° to -180°) time 2b fader lfade
	lifespan 2b
	delay p+s+4t
	
fader logf:
	type log
	exp1 1.0
	exp2 0.0

fader logf2:
	type log
	exp1 0.0
	exp2 1.0

verb displacer:
	rotate (random -30° to 30°)
	move cam 0,1 -> 0,1 seg 3t
	lifespan 3t
	delay p+s+3t

verb colorize:
	move color 0.56,1,0.8117 -> 0.56,1,0 time 1b
	set bgcolor color
	lifespan 4b

verb grouper:
	scale 100%->0% time 31b->32b
	lifespan 33b

g S.star(name=scene10end):
	e S.star delay p+0b
	ref cube
	ref arrows
	flash2 S.stroke1 delay (4b+4t)
	flash3 S.stroke2 delay 6b
	flash2 S.stroke3 delay 7b
	flash3 S.stroke1 delay 8b
	flash2 S.stroke2 delay (8b+4t)
	flash3 S.stroke3 delay 9b
	flash2 S.stroke1 delay (10b+4t)
	flash3 S.stroke2 delay 11b
	flash2 S.stroke3 delay (12b+3t)
	flash3 S.stroke1 delay (12b+6t)
	flash2 S.stroke2 delay (13b+3t)
	flash3 S.stroke3 delay (13b+6t)
	flash2 S.stroke1 delay (20b+8t)
	flash3 S.stroke2 delay (21b+4t)
	flash2 S.stroke3 delay (24b+4t)
	flash3 S.stroke1 delay (26b+8t)
	flash2 S.stroke2 delay (27b+4t)
	#~ f2 S.tears
	#~ f1 S.stroke1
	#~ f2 S.stroke2
	#~ f3 S.stroke3
	displacer*((32b/3t)-1) none
	er S.star delay 32b
	eg S.star delay 32b
	eb S.star delay 32b
	grouper none delay 32b:
		flash4*95 S.tears
	colorize none
