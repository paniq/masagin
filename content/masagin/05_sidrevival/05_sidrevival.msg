# 05_sidrevival

bpm 130
song "05_sidrevival.ogg"
bgcolor 0,0,0

const P 0.05
const P2 0.1
alias S trail

fader spring:
	type spring
	k 2000.0
	mass 1.0
	friction 0.6

verb stravela:
	mul root
	route@2 turn P,P,0 -> P,P,1 time 32b
	mul bud
	route@2 move 0 -> 1 time 32b
	mul parent

verb stravelb:
	move color 0.6,0,0,0
	mul root
	route@1 turn P,P,0 -> P,P,1 time 32b
	mul bud
	route@1 move 0 -> 1 time 32b
	mul parent

verb stravelc:
	scale 0.25
	move color 0,0,1
	mul root
	route@1 turn P,P,1 -> P,P,0 time 12b loop
	mul bud
	route@1 move 1 -> 0 time 12b loop
	mul parent
	
verb initzoom:
	move -0.8,0
	move cam 1,1 -> 0,0 seg 1b
	
verb travel:
	mul root
	move -0.5,-0.5
	route@1 turn P,P,0 -> P,P,1 time 1b->32b
	scale 50%
	move 0.0,0.5
	mul bud
	route@1 move 0.0 -> 1.0 time 1b->32b
	mul parent
	move cam 0,0 -> 1,1 seg 1b
	move cam 1,1 -> 1,1 seg 1b->32b
	move cam 1,1 -> 0,0 seg 32b->33b
	move color 0,0,1 time 2b
	set bgcolor color

verb travel2:
	mul root
	move -0.5,-0.5
	rotate -60°
	route@1 turn P2,P2,0 -> P2,P2,1 time 1b->32b
	scale 50%
	move 0.0,0.5
	mul bud
	route@1 move 0.0 -> 1.0 time 1b->32b
	mul parent
	move cam 0,0 -> 1,1 seg 1b
	move cam 1,1 -> 1,1 seg 1b->32b
	move cam 1,1 -> 0,0 seg 32b->33b
	
verb trailgrow:
	scale texture 0,0,1 -> 1,0,1 time 32b

verb trailgrow2:
	options zclear1+zwrite+alpha+texture
	scale texture 0,0,1 -> 1,0,1 time 32b

verb set1:
	scale 0,1 -> 1,1 fader spring
	lifespan 16b

verb set2:
	scale 0,1 -> 1,1 fader spring
	lifespan 16b

verb set3:
	scale 0,1 -> 1,1 fader spring
	lifespan 16b

verb set4:
	scale 0,1 -> 1,1 fader spring
	lifespan 16b

verb ground:

verb ground2:
	options color+zread+alpha

trailgrow S.starpath lifespan 64b:	
	ground!@8 S.u1 lifespan (64b+8b):
		initzoom@1 none lifespan 16b
	travel@1 none lifespan 33b
	stravela@2 star lifespan 32b
	stravelb@3 star lifespan 32b
	trailgrow2@8 S.amigapath delay p+32b lifespan (32b+8b):
		travel2@1 none delay p+0b lifespan 33b
	ground2@8 S.amigamask delay p+32b lifespan (32b+8b)
	ground@8 S.u2(name=scene05end) lifespan (64b+8b)
