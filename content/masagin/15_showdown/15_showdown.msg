# 15_showdown
# 

bpm 130
song "15_showdown.ogg"
bgcolor 0,0,0

fader zoom:
	type spring
	k 1000
	mass 4
	friction 0.8

fader flog:
	type log
	exp1 0.1
	exp2 1

fader slowzoom:
	type spring
	k 1000
	mass 8
	friction 0.3
	
fader fastzoom:
	type spring
	k 1000
	mass 4
	friction 0.8

verb set:
	scale color 1,1,1,0 -> 1,1,1,1 time 1b->2b
	move cam 0,0 -> 1,1 seg 1b
	move cam 1,1 seg 1b->64b
	move cam 1,1 -> 0,0 seg 64b->65b
	
	mul root
	move 0,-0.5
	scale 400%
	move 0,0.5
	mul bud
	mul parent_model
	
	mul root modelcam
	move modelcam 0,-0.5
	scale modelcam 400%
	scale modelcam 50%
	move modelcam 0,0.5
	mul bud modelcam
	mul parent_model modelcam	
	scale color 0,1,0.5,1
	move color 0,0,0,0 -> (58/255),0,0,0 time 32b->33b
	scale color 1,1,1,1 -> 1,1,1,0 time 64b->65b

verb blowupi:
	scale color 1,1,1,0 -> 1,1,1,1 time 1b
	scale color 1,1,1,1 -> 1,1,1,0 time 3b->4b
	mul root
	rotate -10° -> 10° time 4b
	mul bud
	mul parent_model
	lifespan 4b

verb gyrate:
	mul root
	#scale 0% -> 100% time 4b fader zoom
	scale 1600%->120% time 16b fader slowzoom
	scale color 1,1,1,0 -> 1,1,1,1 time 2b
	rotate 360° -> 0° time 2b loop
	scale 1 -> 0 time 32b->33b
	mul bud
	mul parent_model
	
verb shine:
	mul root
	scale 0% -> 100% time 4b fader zoom
	rotate 0° -> 360° time 32b loop
	scale color 1,1,1,1 -> 1,1,1,0 time 2b->3b
	mul bud
	mul parent_model
	lifespan 3b
	
verb shine2:
	mul root
	scale 0% -> 100% time 1b fader zoom
	scale 100% -> 0% time 1b->2b
	rotate 360° -> 0° time 16b loop
	mul bud
	mul parent_model
	lifespan 2b
		
verb blowup:
	mul root
	scale 0% -> 100% time 1b fader fastzoom
	scale 100% -> 0% time 1b->2b
	rotate -10° -> 10° time 4b
	mul bud
	mul parent_model
	lifespan 2b
	
const T0 (4t*1)
const T1 (4t*3)
const T2 (4t*5)
const T3 (4t*7)
const T4 (4t*9)
const T5 (4t*11)
const TL 1b
const SC 0.99

verb blowup2:
	options color+alpha
	mul root
	scale color 1,1,1,0 -> 1,1,1,1 time 1t
	scale color 1,1,1,1 -> 1,1,1,0 time (8b-8t)->8b
	scale color 1,1,1,1 -> 1,1,1,0.5 time 5b->6b
	scale 70%
	logscale 1.5 -> 1 time 1b fader fastzoom
	logscale 1 -> 1.1 time 2b
	#scale SC -> 1 seg T0->(T0+TL) fader fastzoom
	#scale SC -> 1 seg T1->(T1+TL) fader fastzoom
	#scale SC -> 1 seg T2->(T2+TL)
	scale SC -> 1 seg T3->(T3+TL)
	scale SC -> 1 seg T4->(T4+TL)
	scale SC -> 1 seg T5->(T5+TL)
	scale 1.1 -> 1 time 2b->8b
	scale 1 -> 0.01 time 2b->8b
	rotate -5° -> 5° time 8b
	rotate 0° -> -360° time 2b->8b fader flog
	move 0.01,0 -> -0.01,0 time 8b
	mul bud
	mul parent_model
	lifespan 8b
	
alias sc texts
const GRD1 2b#(2b+4t)
const GRD2 2b#(2b-4t)
const GRD3 4b
	
set sc.bg(name=scene15end) lifespan 65b:
	shine sc.disc delay p+0b
	shine sc.disc delay p+s+GRD1
	shine sc.disc delay p+s+GRD2
	shine sc.disc delay p+s+GRD1
	shine sc.disc delay p+s+GRD2
	shine sc.disc delay p+s+GRD1
	shine sc.disc delay p+s+GRD2
	shine sc.disc delay p+s+GRD1
	shine sc.disc delay p+s+GRD2
	shine sc.disc delay p+s+GRD1
	shine sc.disc delay p+s+GRD2
	shine sc.disc delay p+s+GRD1
	shine sc.disc delay p+s+GRD2
	shine sc.disc delay p+s+GRD1
	shine sc.disc delay p+s+GRD2
	shine sc.disc delay p+s+GRD1
	shine2 sc.star delay p+0b
	shine2 sc.star delay p+s+GRD1
	shine2 sc.star delay p+s+GRD2
	shine2 sc.star delay p+s+GRD1
	shine2 sc.star delay p+s+GRD2
	shine2 sc.star delay p+s+GRD1
	shine2 sc.star delay p+s+GRD2
	shine2 sc.star delay p+s+GRD1
	shine2 sc.star delay p+s+GRD2
	shine2 sc.star delay p+s+GRD1
	shine2 sc.star delay p+s+GRD2
	shine2 sc.star delay p+s+GRD1
	shine2 sc.star delay p+s+GRD2
	shine2 sc.star delay p+s+GRD1
	shine2 sc.star delay p+s+GRD2
	shine2 sc.star delay p+s+GRD1
	blowupi sc.weadore delay p+0b
	#blowup sc.cube delay p+s+GRD1
	blowup sc.asd delay p+s+(GRD1+GRD2)
	blowup sc.blackmaiden delay p+s+GRD1
	blowup sc.conspiracy delay p+s+GRD2
	blowup sc.fairlight delay p+s+GRD1
	blowup sc.kewlers delay p+s+GRD2
	blowup sc.kolor delay p+s+GRD1
	blowup sc.limpninja delay p+s+GRD2
	blowup sc.mercury delay p+s+GRD1
	blowup sc.mfx delay p+s+GRD2
	blowup sc.moppi delay p+s+GRD1
	blowup sc.neuro delay p+s+GRD2
	blowup sc.plastic delay p+s+GRD1
	blowup sc.still delay p+s+GRD2
	blowup sc.tbl delay p+s+GRD1
	gyrate sc.hypno delay p+32b lifespan 33b
	blowup2 sc.nvscene delay p+(32b+(GRD3*0))
	blowup2 sc.wherewhen delay p+(32b+(GRD3*1))
	blowup2 sc.sizeposer delay p+(32b+(GRD3*2))
	blowup2 sc.screenposer delay p+(32b+(GRD3*3))
	blowup2 sc.soundposer delay p+(32b+(GRD3*4))
	blowup2 sc.compos delay p+(32b+(GRD3*5))
	blowup2 sc.seminars delay p+(32b+(GRD3*6))
	blowup sc.andmuch delay p+(32b+(GRD3*6+2b))
	blowup sc.much delay p+(32b+(GRD3*6+3b))
	blowup sc.much delay p+(32b+(GRD3*6+4b))
	blowup sc.much delay p+(32b+(GRD3*6+5b))
	blowup sc.much delay p+(32b+(GRD3*6+6b))
	blowup sc.more delay p+(32b+(GRD3*6+7b))
	#blowup2 sc.more delay p+(32b+(GRD3*6))

