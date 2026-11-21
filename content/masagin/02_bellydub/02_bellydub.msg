# 02_bellydub

bpm 130
song "02_bellydub.ogg"
bgcolor 1,1,1

alias S plants

fader sin:
	type sine

fader bounce1:
	type spring
	k 2000
	mass 1
	friction 0.7

fader slowdown2:
	type log
	exp1 0.1
	exp2 1.0
	
fader slowdown:
	type log
	exp1 1.0
	exp2 0.1

fader slowdown3:
	type log
	exp1 1.0
	exp2 0.01

verb set:

verb wait:
	lifespan 16b

verb rest:
	lifespan 1t

verb grow:
	scale 1,1,0% -> 1,1,(random 50% to 100%) uniform time 2b fader slowdown
	scale 1 -> 0 seg 19b->20b
	lifespan 20b
	
verb growtt:
	scale 1,1,0% -> 1,1,(random 50% to 100%) uniform time 2b fader slowdown
	scale 1 -> 0 seg 6b->7b
	lifespan 7b
	
const G 4
const GD 6b # stem grow duration
const LT 8t # leaves timing
const LC (GD / LT) # leaves per stem

verb stembase:
	scale texture 0,1,1,G -> 1,1,1,G time GD fader slowdown
	scale texture 1,1,1,1 -> 0,1,1,1 seg 23b->24b fader slowdown

verb stem:
	like stembase
	lifespan 24b

verb stemx:
	scale -1,1
	like stembase
	lifespan 24b

verb stemy:
	scale 1,-1
	like stembase
	lifespan 24b
	
verb stemlongarm:
	scale texture 0,1,1,(G*4) -> 1,1,1,(G*4) time 30b fader slowdown
	rotate 32°
	lifespan 38b


const TOPCAM 2.4

verb dolly:
	mul root
	move 1,0 -> 1,0 seg 32b->65b
	mul bud
	mul parent_model
	
	mul root modelcam
	scale modelcam 100%->25% time 8b fader slowdown
	move modelcam 0,-0.1 -> 0,0 time 8b
	move modelcam 0,0 -> 1,0 seg 32b
	move modelcam 1,0 -> 1,0 seg 32b->35b
	move modelcam 1,0 -> 1,TOPCAM seg 35b->65b
	mul bud modelcam
	mul parent_model modelcam
	
	move cam 1,1,0 -> 1,1,0 seg 0b -> 38b
	move cam 1,1,0 -> 1,1,1 seg 38b -> 48b
	move cam 1,1,1 -> 1,1,1 seg 48b -> 64b
	move cam 1,1,1 -> 0,0,1 seg 64b->65b
	lifespan 65b

const RP 0.01 # route precision
const RP2 0.01 # route precision

verb travel1:
	mul root
	route@1 turn RP,RP,0 -> RP,RP,1 time GD fader slowdown
	scale 100% -> 0% time GD
	#turn -120°
	turn -90°
	mul bud
	route@1 move 0 -> 1 time GD fader slowdown
	mul parent_model
	lifespan 20b

verb travel2:
	mul root
	route@1 turn RP,RP,0 -> RP,RP,1 time GD
	#turn -120°
	turn -90°
	mul bud
	route@1 move 0 -> 1 time GD
	mul parent_model
	lifespan 20b
	
verb travel3:
	mul root
	route@1 turn RP2,RP2,0 -> RP2,RP2,1 time 30b
	#turn -120°
	#turn -90°
	rotate -120°
	mul bud
	route@1 move 0 -> 1 time 30b
	mul parent_model
	lifespan 30b
	
verb rosetravel:
	mul root
	route@1 turn RP,RP,1 -> RP,RP,1
	#turn -120°
	mul bud
	route@1 move 1
	mul parent_model
	lifespan 20b

branch axis:
	rest none delay p+s+LT:
		grow S.leaf delay p+0b
	rest none delay p+s+LT:
		grow S.leaf delay p+0b


branch axis2:
	rest none delay p+s+(LT*0.2):
		grow S.thornl delay p+0b
	rest none delay p+s+(LT*0.8):
		grow S.thornr delay p+0b

branch axisb:
	rest none delay p+s+LT:
		growtt S.leaf delay p+0b
	rest none delay p+s+LT:
		growtt S.leaf delay p+0b


branch axis2b:
	rest none delay p+s+(LT*0.2):
		growtt S.thornl delay p+0b
	rest none delay p+s+(LT*0.8):
		growtt S.thornr delay p+0b
		
branch leaves:
	travel1 S.leafaxis delay p+0b:
		ref*LC axis

branch thorns:
	travel1 S.thornaxis delay p+0b:
		ref*LC axis2

branch leaves2:
	travel3@1 S.leafaxis delay p+0b:
		ref*21 axisb

branch thorns2:
	travel3@1 S.thornaxis delay p+0b:
		ref*42 axis2b

verb baserosegrow:
	options color+mulcolor
	mul root
	scale 1.0 -> 0.93 time 8b fader slowdown3
	scale 0,0 -> 1,1 time 8b fader slowdown3
	scale 1 -> 0 seg 15b->16b
	rotate 150°
	mul bud
	mul parent_model
	lifespan 16b

verb rosegrow:
	options color+mulcolor
	mul root
	scale 1.0 -> 0.93 time 8b fader slowdown3
	rotate 150°
	mul bud
	mul parent_model
	lifespan 16b

verb lowscale:
	scale 1,1,(random 25% to 50%) uniform
	lifespan 16b

verb lowscale2:
	scale 1,1,10% uniform
	move 0,0.5
	lifespan 16b
	
branch rose:
	lowscale none delay p+0b:
		baserosegrow S.rose delay p+0b:
			rule baserosegrow S.rose -> branch:
				rosegrow S.rose delay p+0b
			rule rosegrow S.rose -> branch:
				rosegrow S.rose delay p+0b
			iterate 15
	
branch roses:
	rosetravel none delay p+GD:		
		ref rose

branch leavesnthorns:
	ref thorns
	ref leaves
	ref roses

branch leavesnthorns2:
	ref thorns2
	ref leaves2
	
branch part3:
	rest none delay 23.6b:
		wait none delay p+0b:
			stemy S.snailarm delay p+0b:
				ref leavesnthorns

branch part2:
	rest none delay (4b+9t):
		wait none delay p+0b:
			stem S.snailarm delay (9b+6t):
				ref leavesnthorns
	rest none delay (5b+8t):
		wait none delay p+0b:
			stemy S.snailarm delay 15b:
				ref leavesnthorns
				travel2 none delay p+0b:
					rest none delay 17.75b:
						wait none delay p+0b:
							stemy S.snailarm delay 20.92b:
								ref leavesnthorns
								travel2 none delay p+0b:
									ref part3

dolly S.stage:
	stemlongarm S.longarm(name=scene02end) delay 32b:
		ref leavesnthorns2
		lowscale2@2 none delay 63b:
			baserosegrow S.rose delay p+0b:
				rule baserosegrow S.rose -> branch:
					rosegrow S.rose delay p+0b
				rule rosegrow S.rose -> branch:
					rosegrow S.rose delay p+0b
				iterate 8
	stem S.snailarm:
		travel2 none:
			rest none delay 4t:
				stemy S.snailarm delay p+0b:
					ref leavesnthorns
					travel2 none delay p+0b:
						rest none delay (3b+1t):
							wait none delay p+0b:
								stemy S.snailarm delay p+0b:
									ref leavesnthorns
									travel2 none delay p+0b:
										ref part2
