# 04_stallingangle

bpm 130
song "04_stallingangle.ogg"
bgcolor 0,0,0


fader logf:
	type log
	exp1 1.0
	exp2 0.0

fader logf2:
	type log
	exp1 0.0
	exp2 1.0

fader blinkfader:
	type square

verb pan:
	move color 0.666667,1,0 -> 0.666667,1,0.25098 seg 62b -> 64b
	set bgcolor color
	move modelcam -0.5,-0.5
	scale modelcam 400%
	move modelcam 0.5,0.5
	mul root modelcam
	move modelcam 0,0 -> 34,0 time 32b
	mul bud modelcam
	mul parent_model modelcam

	mul root
	mul bud
	mul parent_model
	store aux0
	
	move cam 0,0,0 -> 1,1,0 seg 0b->1b
	move cam 1,1,0 -> 1,1,0 seg 1b->32b
	
	move cam 0,0,0 -> 0,0,1 seg 1b->2b
	move cam 0,0,1 -> 0,0,1 seg 2b->31b
	move cam 0,0,1 -> 0,0,0 seg 31b->32b
	lifespan 65b

fader sb:
	type sine
	cycles 32b

verb grow:
	options color+alpha+texture+addcolor
	move 0,-0.5
	move 0,0 -> 2,0 time 32b
	move 0,0 -> -16,0 time 32b->42b
	scale 400%
	scale texture 0,0,0.01,1 -> 1,0,0.01,1 time 32b
	scale color 1,1,1,0.4
	lifespan 42b

verb displacer:
	rotate (random -10° to 10°)
	move cam 0,0 -> 0,1 seg 2b fader logf
	move cam 0,1 -> 0,0 seg 2b->4b fader logf2
	lifespan 2b
	delay p+s+1b

verb defop:
	identity model
	mul root
	mul bud
	mul parent_aux0
	store aux0

	identity model
	move -0.5,-0.5
	scale 16 -> 1 time 1b fader logf
	scale 1 -> 0 time 3b->4b
	move 0,0 -> -16,0 time 2b->4b
	move 0.5,0.5
	mul root
	mul bud
	mul parent_aux0	

verb off:
	options color+alpha+addcolor
	
	mul root modelcam	
	mul bud modelcam
	mul parent_model modelcam
	
	like defop
	
	move color 0,0,0,-0.8 -> 0,0,0,-0.6 time 1b loop
	scale color 1,1,1,0 -> 1,1,1,1 time 4t
	delay parent+siblings+0.01b
	lifespan 4b

verb blink:
	options color+alpha+addcolor+subcolor
	
	mul root modelcam
	mul bud modelcam
	mul parent_model modelcam

	like defop
	
	move color 0,0,0,0 -> 0,0,-0.8,0 time 4t loop fader blinkfader
	scale color 1,1,1,0 -> 1,1,1,1 time 4t
	delay parent+siblings+2t
	lifespan 4b

alias S sevendigs
alias T part2

branch t303:
	off S.n1:
		off@2 S.n2
		off@2 S.n3
		off@2 S.n4
		off@2 S.n5
		off@2 S.n6
		off@2 S.n7:
			blink S.n1:
				off@2 S.n2
				blink@2 S.n3
				blink@2 S.n4
				off@2 S.n5
				blink@2 S.n6
				blink@2 S.n7:
					blink S.dots:
						blink S.n1:
							blink@2 S.n2
							blink@2 S.n3
							off@2 S.n4
							blink@2 S.n5
							blink@2 S.n6
							blink@2 S.n7:
								blink S.n1:
									off@2 S.n2
									blink@2 S.n3
									blink@2 S.n4
									off@2 S.n5
									blink@2 S.n6
									blink@2 S.n7

verb pan2:
	move -0.5,-0.5
	scale 400%
	move 0.5,0.5
	mul root
	move 34,0
	store modelcam
	mul bud
	mul parent_model
	
	scale modelcam 100% -> 1% time 30b->32b
	move modelcam 0,0 -> 60,6 time 0b->32b
	mul bud modelcam
	mul parent_model modelcam

	move cam 0,0,0 -> 1,1,0 seg 0b->2b
	move cam 1,1,0 -> 1,1,0 seg 2b->32b
	
	move cam 0,0,0 -> 0,0,1 seg 14b->24b
	move cam 0,0,1 -> 0,0,1 seg 24b->31b
	move cam 0,0,1 -> 0,0,0 seg 31b->32b
	lifespan 33b

verb base:
	mul root
	skew 0,-10°
	rotate 90°
	scale 30%
	move 1,0.5
	mul bud
	mul parent_model
	scale color 1,1,1,0 -> 1,1,1,1 time 8t->1b
	
verb go:
	options color+alpha+addcolor+subcolor

	mul root
	scale (random 4 to 6) -> 1 time 1b->(1b 4t) fader logf
	#move 0,(random 0 to 1)->0,0 time 1b->2b fader logf
	scale -1,1
	mul bud
	mul parent_model

	load parent_aux1 color
	move color (0.66/96),0
	store aux1 color
	scale color 1,1,1,0 -> 1,1,1,1 time 8t->1b
	delay p+4t
	lifespan 10b

verb go1:
	options color+alpha+addcolor

	mul root
	#scale (random 4 to 4) -> 1 time 1b->(1b 4t) fader logf
	move 0,(random -4 to 4)->0,0 time 1b->2b fader logf
	scale -1,1
	mul bud
	mul parent_model
	
	load parent_aux1 color
	move color (0.66/96),0
	store aux1 color
	scale color 1,1,1,0 -> 1,1,1,(random 0.1 to 0.5) time 8t->1b
	delay p+4t
	lifespan 10b

verb flyby:
	options color+addcolor+alpha
	rotate -90°
	move 0.1,0 -> -1,0 time 8b loop
	scale 32
	move 0,0.5
	move 0,0 -> 0,1.5 time 0b->32b
	move color 0,0,-0.3
	#move color 0,0 -> 1.0,0 time 32b
	lifespan 32b
	
verb flyby2:
	options color+addcolor+alpha
	rotate -90°
	move 0.1,0 -> -1,0 time 6b loop
	scale 32
	move 0,0.5
	move 0,0 -> 0,1.5 time 0b->32b
	move color 0,0,-0.3
	move color 0,0 -> 0.66,0 time 32b
	lifespan 28b

branch T1:
	go1@2 T.numeric:
		go@3 T.numeric:
			go1@1 T.numeric

branch T2:
	go@4 T.numeric:
		go1@3 T.numeric:
			go@1 T.numeric

branch T3:
	go1@2 T.numeric:
		go@3 T.numeric:
			go1@1 T.numeric

branch T4:
	go@4 T.numeric:
		go1@2 T.numeric:
			go@4 T.numeric

branch T5:
	go@4 T.numeric:
		go1@2 T.numeric:
			go1@4 T.numeric
			
branch TN1:
	ref T1:
		ref T2:
			ref T3:
				ref T4:
					ref T3:
						ref T1:
							ref T3:
								ref T4

branch TN2:
	ref T1:
		ref T2:
			ref T3:
				ref T4:
					ref T3:
						ref T1:
							ref T3:
								ref T5

verb stand:
	lifespan 16b
	
verb placex:
	mul root
	mul bud
	mul parent_modelcam

branch digiwood:
	pan2 none delay 32b:
		flyby T.lines delay 32b
		flyby2 T.lines delay 36b
		placex none(name=scene04end) delay 32b lifespan 32b
		base T.numeric delay 31b lifespan 33b:
			ref TN1:
				ref TN1:
					ref TN1:
						ref TN2

pan none:
	grow arrows
	ref t303:
		ref t303:
			ref t303:
				ref t303:
					ref t303:
						ref t303:
							ref t303:
								ref t303:
									ref t303:
										ref t303
	ref digiwood
	displacer*29 none
