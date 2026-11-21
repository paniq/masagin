# 13_evolution

bpm 130
song "13_evolution.ogg"
bgcolor 0.66,0.3,0.1
	
fader bob:
	type sine
	phase -90°
	
fader bump:
	type spring
	k 250
	mass 4
	friction 0.6	
	
fader bump2:
	type spring
	k 250
	mass 16
	friction 0.3
	
alias S capitalist

fader tru:
	type triangle

verb blendin:
	scale color 1,1,1,0 -> 1,1,1,1 time 4b->8b
	scale color 1,1,1,1 -> 1,1,1,0 time 50b->54b
	scale texture 1,0,8,4 -> 0,0,8,4 time 8t loop fader tru

verb profit1:
	options color+alpha+addcolor+texture
	scale texture 0,1,1,32 -> 0.5,1,1,32 time 32b

verb profit2:
	options color+alpha+addcolor+texture
	scale texture 0.5,1,1,32 -> 1,1,1,32 time 32b
	move 0,0 -> 0.05,0.05 time 3t loop fader tru
	#move color 0,0,0 -> 1,0,0 time 64b

fader bumpy:
	type sine
	cycles 8

verb dolly:
	move -0.5,-0.5
	#move ,0.0
	scale 200%
	rotate -90° -> 90° time 64b
	mul root
	mul bud
	route@1 move 0 -> 0.23 seg 16b
	route@1 move 0.23 -> 0.45 seg 16b->32b
	route@1 move 0.45 -> 0.71 seg 32b->48b
	route@1 move 0.71 -> 1.0 seg 48b->64b
	mul parent_model
	move cam 0,0 -> 1,1 seg 8b
	move cam 1,1 seg 8b->63b
	move cam 1,1 -> 0,0 seg 63b->64b

verb stage:
	move -0.5,-0.5
	scale 50%
	move 0.5,0.5
	mul root
	move 0.25,0.25
	mul bud
	mul parent_model

verb set0:
	lifespan 4b

fader rotfade:
	type log
	exp1 0.1
	exp2 1

fader rotfade2:
	type log
	exp1 1
	exp2 0.1
	
verb set1:
	rotate 0° -> 90° time 4b
	rotate 0° -> (360°*4) time 2b->8b fader rotfade
	lifespan 4b

verb set2:
	rotate 0° -> 90° time 4b
	rotate 0° -> (-360°*4) time 2b->8b fader rotfade
	lifespan 4b
	
verb xplode1:
	mul root
	move 0,-0.5
	#rotate 0°->-360° time 4b fader rotfade2
	scale 0% -> 80% time 4b fader bump
	move 0,0.5
	move 0,-1 -> 0,0.4 fader bump
	scale color 1,1,1,1 -> 1,1,1,0 time 2b->4b
	mul bud
	mul parent_model
	delay p+0b
	lifespan 4b

verb xplode2:
	options color+alpha+addcolor
	mul root
	move 0,-0.5
	rotate 0°->-360° time 4b fader rotfade2
	scale 0% -> 200% time 1b fader bump
	move 0,0.5
	move 0,-1 -> 0,0.3 fader bump
	scale color 1,1,1,1 -> 1,1,1,0 time 2b->4b
	mul bud
	mul parent_model
	delay p+0b
	lifespan 4b

verb colorize:
	move color 0.66,0.3,0 -> 0.66,0.3,0.1 time 1b
	set bgcolor color
	lifespan 16b
	
verb placement:
	move cam 0,0 -> 1,1 seg 7b->8b
	move cam 1,1 -> 0,0 seg 8b->9b
	lifespan 9b

verb roll:
	move 0,0 -> -1.1,0 time 64b
	move 0,-0.5
	scale 8
	move 0,0.9
	move 1.9,0
	scale color 1,1,1,1 -> 1,1,1,0 time 62b->63b
	lifespan 63b

blendin S.stockgrid lifespan 64b:
	colorize none	
	profit1@1 S.stock lifespan 32b
	profit2@1 S.stock delay 32b lifespan 35b
	placement@2 none(name=scene13end) delay (64b-8b)
	dolly@1 none lifespan 65b:
		stage none lifespan 64b:
			set1 S.firework delay (4b*0):
				xplode2@12,10,8,6,4,2 S.star
			set1 S.firework delay (4b*0):
				xplode1@12,10,8,6,4,2 redruby
				
			set2 S.firework delay (4b*1):
				xplode2@12,10,8,6,4,2 S.star2
			set2 S.firework delay (4b*1):
				xplode1@12,10,8,6,4,2 S.diamond
				
			set1 S.firework delay (4b*2):
				xplode2@12,10,8,6,4,2 S.star3
			set1 S.firework delay (4b*2):
				xplode1@12,10,8,6,4,2 greenemerald
				
			set2 S.firework delay (4b*3):
				xplode2@12,10,8,6,4,2 S.star4
			set2 S.firework delay (4b*3):
				xplode1@12,10,8,6,4,2 redruby

			set1 S.firework delay (4b*4):
				xplode2@12,10,8,6,4,2 S.star5
			set1 S.firework delay (4b*4):
				xplode1@12,10,8,6,4,2 S.diamond
				
			set2 S.firework delay (4b*5):
				xplode2@12,10,8,6,4,2 S.star6
			set2 S.firework delay (4b*5):
				xplode1@12,10,8,6,4,2 greenemerald
				
			set1 S.firework delay (4b*6):
				xplode2@12,10,8,6,4,2 S.star7
			set1 S.firework delay (4b*6):
				xplode1@12,10,8,6,4,2 redruby
				
			set2 S.firework delay (4b*7):
				xplode2@12,10,8,6,4,2 S.star8
			set2 S.firework delay (4b*7):
				xplode1@12,10,8,6,4,2 S.diamond

			set1 S.firework delay (4b*8):
				xplode2@12,10,8,6,4,2 S.star2
			set1 S.firework delay (4b*8):
				xplode1@12,10,8,6,4,2 redruby
				
			set2 S.firework delay (4b*9):
				xplode2@12,10,8,6,4,2 S.star3
			set2 S.firework delay (4b*9):
				xplode1@12,10,8,6,4,2 greenemerald
				
			set1 S.firework delay (4b*10):
				xplode2@12,10,8,6,4,2 S.star4
			set1 S.firework delay (4b*10):
				xplode1@12,10,8,6,4,2 S.diamond
				
			set2 S.firework delay (4b*11):
				xplode2@12,10,8,6,4,2 S.star5
			set2 S.firework delay (4b*11):
				xplode1@12,10,8,6,4,2 redruby

			set1 S.firework delay (4b*12):
				xplode2@12,10,8,6,4,2 S.star6
			set1 S.firework delay (4b*12):
				xplode1@12,10,8,6,4,2 greenemerald
				
			set2 S.firework delay (4b*13):
				xplode2@12,10,8,6,4,2 S.star7
			set2 S.firework delay (4b*13):
				xplode1@12,10,8,6,4,2 S.diamond
				
			set1 S.firework delay (4b*14):
				xplode2@12,10,8,6,4,2 S.star8
			set1 S.firework delay (4b*14):
				xplode1@12,10,8,6,4,2 redruby
				
			set2 S.firework delay (4b*15):
				xplode2@12,10,8,6,4,2 S.star
			set2 S.firework delay (4b*15):
				xplode1@12,10,8,6,4,2 greenemerald
			roll ticker
