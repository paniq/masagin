# 12_readysteadygo

bpm 130
song "12_readysteadygo.ogg"

alias s fiesta

verb set:
	scale color 1,1,1,0
	mul root
	scale 200%
	move -1.0,-0.5
	store modelcam
	mul bud
	mul parent_model
	
	move cam 0,0 -> 1,1 seg 8b
	move cam 1,1 seg 8b->64b
	move cam 1,1 -> 0,0 seg 64b->(64b+8b)
	
	move modelcam 0,-0.5
	scale modelcam 80%
	move modelcam 0,0.5
	mul bud modelcam
	mul parent_model modelcam

verb show:


fader bounce:
	type spring
	k 1000
	mass 250
	friction 0.9

verb b1:
	mul root
	scale 8%,100%->15%,100% time 8b
	mul bud
	mul parent_model
	scale color 0,0,0,0
	identity aux0
	identity aux1
	move aux0 (250/256),1,0.5
	move aux1 (250/256),1,0.5
	lifespan 6b

verb b2:
	mul root
	scale 15%,100%->20%,100% time 8b
	mul bud
	mul parent_model
	scale color 0,0,0,0
	identity aux0
	identity aux1
	move aux0 (192/256),1,0.5
	move aux1 (192/256),1,0.5
	lifespan 6b

verb b3:
	mul root
	scale 30%,100%->20%,100% time 8b
	mul bud
	mul parent_model
	scale color 0,0,0,0
	identity aux0
	identity aux1
	move aux0 (24/256),1,0.5
	move aux1 (24/256),1,0.5
	lifespan 6b

verb b4:
	mul root
	scale 15%,100%->12%,100% time 8b
	mul bud
	mul parent_model
	scale color 0,0,0,0
	identity aux0
	identity aux1
	move aux0 (128/256),1,0.5
	move aux1 (128/256),1,0.5
	lifespan 6b

verb b5:
	mul root
	scale 8%,100%->15%,100% time 8b
	mul bud
	mul parent_model
	scale color 0,0,0,0
	identity aux0
	identity aux1
	move aux0 (250/256),1,1.0
	move aux1 (250/256),1,0.3
	lifespan 6b

verb b6:
	mul root
	scale 15%,100%->20%,100% time 8b
	mul bud
	mul parent_model
	scale color 0,0,0,0
	identity aux0
	identity aux1
	move aux1 (192/256),1,0.8
	move aux0 (192/256),1,0.4
	lifespan 6b

verb b7:
	mul root
	scale 30%,100%->20%,100% time 8b
	mul bud
	mul parent_model
	scale color 0,0,0,0
	identity aux0
	identity aux1
	move aux0 (24/256),1,1.0
	move aux1 (24/256),1,0.3
	lifespan 6b

verb b8:
	mul root
	scale 15%,100%->12%,100% time 8b
	mul bud
	mul parent_model
	scale color 0,0,0,0
	identity aux0
	identity aux1
	move aux1 (128/256),1,0.8
	move aux0 (128/256),1,0.4
	lifespan 6b

verb boink0:
	mul root
	scale 100%,0% -> 100%,120% time 4b fader bounce
	#move 0,0 -> 0,1 time 2b->4b
	mul bud
	mul parent_model
	load parent_aux0 color
	lifespan 6b

verb boink1:
	mul root
	scale 100%,0% -> 100%,120% time 4b fader bounce
	#move 0,0 -> 0,1 time 2b->4b
	mul bud
	mul parent_model
	load parent_aux1 color
	lifespan 6b

verb boinkC:
	like boink1
	move color 0,0,0.1,0

verb boinkL1:
	move -0.5,0.0
	like boink0
	move color 0,0,-0.1,0

verb boinkR1:
	move 0.5,0.0
	like boink0
	move color 0,0,-0.1,0

verb boinkL2:
	move -1.0,0.0
	like boink1
	move color 0,0,0.2,0

verb boinkR2:
	move 1.0,0.0
	like boink1
	move color 0,0,0.2,0

verb boinkL3:
	move -1.5,0.0
	like boink0
	move color 0,0,-0.2,0

verb boinkR3:
	move 1.5,0.0
	like boink0
	move color 0,0,-0.2,0

branch gb:
	boinkL3 s.block delay p+1b
	boinkL2 s.block delay p+4t
	boinkL1 s.block delay p+8t
	boinkC s.block delay p+0b
	boinkR1 s.block delay p+8t
	boinkR2 s.block delay p+4t
	boinkR3 s.block delay p+1b

branch gb2:
	boinkL3 s.block delay p+0b
	boinkL2 s.block delay p+0b
	boinkL1 s.block delay p+4t
	boinkC s.block delay p+4t
	boinkR1 s.block delay p+8t
	boinkR2 s.block delay p+8t
	boinkR3 s.block delay p+1b

verb grow:	
	mul root
	move 0.0,0.0 -> 0.0,0.1 time 6b
	scale texture 0,1,1,4 -> 1,1,1,4 time 6b
	scale 100%->120% time 6b
	mul bud
	mul parent_model
	lifespan 6b

verb growinv:	
	mul root
	rotate 180°
	move 0.0,1.0
	move 0.0,0.0 -> 0.0,0.1 time 6b
	scale texture 0,1,1,4 -> 1,1,1,4 time 6b
	scale 100%->120% time 6b
	mul bud
	mul parent_model
	lifespan 6b

fader punch:
	type spring
	k 1000.0
	mass 1.0
	friction 0.6

fader punch2:
	type spring
	k 1000.0
	mass 1.0
	friction 0.3

verb slidebase:
	move -0.5,-0.5
	rotate 0° -> 10° seg (2b+4t)->3b fader punch2
	rotate 10° -> -20° seg 3b->(3b+8t) fader punch2
	rotate -20° -> 30° seg (3b+8t)->4b fader punch2
	rotate 30° seg 4b->6b
	scale 100% -> 120% seg (2b+4t)->3b fader punch2
	scale 120% -> 140% seg 3b->(3b+8t) fader punch2
	scale 140% -> 180% seg (3b+8t)->4b fader punch2
	scale 180% seg 4b->6b
	move -0.1,0 -> 0.1,0 time 6b
	move 0.5,0.5

verb slidebase2:
	move -0.5,-0.5
	rotate 0° -> 10° seg (2b+4t)->3b fader punch2
	rotate 10° -> -20° seg 3b->(3b+8t) fader punch2
	rotate -20° -> 30° seg (3b+8t)->4b fader punch2
	rotate 30° seg 4b->6b
	scale 180% seg (2b+4t)
	scale 180% -> 140% seg (2b+4t)->3b fader punch2
	scale 140% -> 120% seg 3b->(3b+8t) fader punch2
	scale 120% -> 100% seg (3b+8t)->4b fader punch2
	scale 100% seg 4b->12b
	move -0.1,0 -> 0.1,0 time 6b
	move 0.5,0.5
	
verb slidebase2b:
	move -0.5,-0.5
	rotate 0° -> 10° seg (2b+4t)->3b fader punch2
	rotate 10° -> -20° seg 3b->(3b+8t) fader punch2
	rotate -20° -> 30° seg (3b+8t)->4b fader punch2
	rotate 30° seg 4b->6b
	scale 180% seg (2b+4t)
	scale 180% -> 140% seg (2b+4t)->3b fader punch2
	scale 140% -> 120% seg 3b->(3b+8t) fader punch2
	scale 120% -> 100% seg (3b+8t)->4b fader punch2
	scale 100% seg 4b->12b
	move -0.05,0 -> 0.05,0 time 12b
	move 0.5,0.5
	
verb slide1:
	like slidebase
	mul root
	move 0,-1.2 -> 0,-0.1 time 2b fader punch
	scale 100% -> 120% time 6b
	mul bud
	mul parent_model
	lifespan 6b

verb slide1b:
	like slidebase
	mul root
	move 0,-1.2 -> 0,0 time 2b fader punch
	scale 100% -> 120% time 6b
	mul bud
	mul parent_model
	lifespan 6b
	
verb slide2:
	like slidebase
	mul root
	move 0,-1.2 -> 0,-0.1 time 2b fader punch
	scale 120% -> 140% time 6b
	mul bud
	mul parent_model
	lifespan 6b
	
verb slide3:
	like slidebase
	mul root
	move 0,-1.2 -> 0,-0.1 time 2b fader punch
	scale 220% -> 240% time 6b
	mul bud
	mul parent_model
	lifespan 6b
	
verb slide4:
	options color+zread
	like slidebase2
	mul root
	move 0,-1.2 -> 0,-0.1 time 2b fader punch
	scale 160% -> 130% time 6b
	mul bud
	mul parent_model
	lifespan 6b

verb slide4b:
	options color+zread
	like slidebase2b
	mul root
	move 0,-1.2 -> 0,-0.1 time 2b fader punch
	scale 200%
	mul bud
	mul parent_model
	lifespan 12b
	
verb driveinL:
	options color+zread
	scale 75%
	move -0.7,0.13 -> -0.5,0.13 time 2b fader punch
	lifespan 40b

verb driveinR:
	options color+zread
	scale 80%
	move 0.7, 0.1 -> 0.52,0.1 time 2b fader punch
	lifespan 40b

fader armplay:
	type triangle

verb strum:
	options color+zread
	rotate -10° -> 0° time 3t loop fader armplay
	lifespan 40b

fader sqdance:
	type square

verb dance1:
	options color+zread
	lifespan 8b

verb dance2:
	options color+zread
	#rotate -2° -> 5° time 6t loop
	rotate -30° -> 0° time 8t loop fader sqdance
	lifespan 24b

alias C muertoguitar

verb fadeout:
	options zclear1+zwrite
	lifespan 64b
	move -0.5,-0.5
	logscale 800% -> 0% time 4b
	move 0,-0.4 -> 0,0 time 4b
	move 0.5,0.5
	mul root
	mul bud
	mul parent_model
	lifespan 4b

set s.bg(name=scene12end) lifespan 72b:
	b1@4 s.block:
		ref gb
	b2@2 s.block delay 4b:
		ref gb
	b3@3 s.block delay (4b*2):
		ref gb
	b4@1 s.block delay (4b*3):
		ref gb
	b5@4 s.block delay (4b*4):
		ref gb2
	grow@4 s.stripes2 delay (4b*4)
	b6@2 s.block delay (4b*5):
		ref gb2
	grow@2 s.stripes1 delay (4b*5)
	b7@3 s.block delay (4b*6):
		ref gb2
	growinv@3 s.stripes4 delay (4b*6)
	b8@1 s.block delay (4b*7):
		ref gb2
	growinv@1 s.stripes3 delay (4b*7)
	fadeout@3 mexico delay 64b
	slide1b@4 towel delay (4b*0+32b)
	slide2@2 towel delay (4b*1+32b)
	slide3@3 towel delay (4b*2+32b)
	slide4@1 towel delay (4b*3+32b)
	slide1@4 towel delay (4b*4+32b)
	slide2@2 towel delay (4b*5+32b)
	slide3@3 towel delay (4b*6+32b)
	slide4b@1 towel delay (4b*7+32b)
	driveinL@3 C.musicus delay 32b:
		strum C.musicarm delay 32b
	driveinR@3 C.dancer delay 40b
		dance1 C.dancearm delay 40b
		dance2 C.dancearm delay 48b
