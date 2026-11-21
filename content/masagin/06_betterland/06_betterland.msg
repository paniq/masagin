# 06_betterland

bpm 130
song "06_betterland.ogg" # use -nosound for no sound :)
bgcolor 1,1,1

verb stage:
	mul root
	scale 0->1 time 1b
	mul bud
	mul parent_model
	mul root modelcam
	scale modelcam 4->1 time 8b
	rotate modelcam (random 10° to 20°)->(random -20° to -10°) time 9b
	mul bud modelcam
	mul parent_model modelcam
	move cam 0,0 -> 1,1 seg 0b->1b
	move cam 1,1 seg 1b->8b
	move cam 1,1 -> 0,0 seg 8b->9b
	lifespan 16b
	
verb stage2:
	mul root
	scale 0->1 time 1b
	mul bud
	mul parent_model
	mul root modelcam
	scale modelcam 1.2->0.9 time 8b
	rotate modelcam (random 10° to 20°)->(random -20° to -10°) time 9b
	mul bud modelcam
	mul parent_model modelcam
	move cam 0,0 -> 1,1 seg 0b->1b
	move cam 1,1 seg 1b->8b
	move cam 1,1 -> 0,0 seg 8b->9b
	lifespan 16b
	
verb stage2e:
	mul root
	scale 0->1 time 1b
	mul bud
	mul parent_model
	mul root modelcam
	scale modelcam 1.2->0.9 time 8b
	rotate modelcam (random 10° to 20°)->(random -20° to -10°) time 9b
	mul bud modelcam
	mul parent_model modelcam
	move cam 0,0 -> 1,1 seg 0b->1b
	move cam 1,1 seg 1b->7b
	move cam 1,1 -> 0,0 seg 7b->8b
	lifespan 8b
	
const A -24.54°

const T0 (1b+8t)
const T1 2b
const T2 (2b+8t)
const T3 (3b+8t)
const T4 (4b+8t)
const T5 6b
const T5b (5b+8t)
const T6 7b
const T6b (6b+8t)
const T7 (7b+8t)
const T8 16b #(8b+1b+8t)

const CS (1/-8)

fader f:
	type spring
	k 1000
	mass 0.1
	friction 0.1

fader f2:
	type spring
	k 1000
	mass 0.01
	friction 0.1

fader f3:
	type sine
	cycles 22

verb pulse:
	scale 1.05->1 time (7b+4t) fader f3

verb twist1:
	mul root
	like pulse
	rotate (A*0)->(A*1) seg T0->T1 fader f
	rotate (A*1)->(A*9) seg T1->T2 fader f
	rotate (A*9)->(A*1) seg T2->T3 fader f
	rotate (A*1)->(A*5) seg T3->T4 fader f
	rotate (A*5)->(A*1) seg T4->T5 fader f
	rotate (A*1)->(A*3) seg T5->T6 fader f
	rotate (A*3)->(A*4) seg T6->T7 fader f
	rotate (A*4)->(A*5) seg T7->T8 fader f2
	mul bud
	mul parent_model

verb twist2:
	mul root
	like pulse
	rotate (A*0)->(A*1) seg T0->T1 fader f
	rotate (A*1)->(A*9) seg T1->T2 fader f
	rotate (A*9)->(A*1) seg T2->T3 fader f
	rotate (A*1)->(A*5) seg T3->T4 fader f
	rotate (A*5)->(A*1) seg T4->T5 fader f
	rotate (A*1)->(A*3) seg T5->T6 fader f
	rotate (A*3)->(A*4) seg T6->T7 fader f
	rotate (A*4)->(A*1) seg T7->T8 fader f2
	mul bud
	mul parent_model

verb twist3:
	mul root
	like pulse
	rotate (A*0)->(A*1) seg T0->T1 fader f
	rotate (A*1)->(A*10) seg T1->T2 fader f
	rotate (A*10)->(A*1) seg T2->T3 fader f
	rotate (A*1)->(A*5) seg T3->T4 fader f
	rotate (A*5)->(A*1) seg T4->T5 fader f
	rotate (A*1)->(A*3) seg T5->T6 fader f
	rotate (A*3)->(A*4) seg T6->T7 fader f
	rotate (A*4)->(A*5) seg T7->T8 fader f2
	mul bud
	mul parent_model

verb twist4:
	mul root
	like pulse
	rotate (A*0)->(A*5) seg T0->T1 fader f
	rotate (A*5)->(A*6) seg T1->T2 fader f
	rotate (A*6)->(A*4) seg T2->T3 fader f
	rotate (A*4)->(A*5) seg T3->T4 fader f
	rotate (A*5)->(A*3) seg T4->T5b fader f
	rotate (A*3)->(A*4) seg T5b->T6 fader f
	rotate (A*4)->(A*9) seg T6->T7 fader f
	rotate (A*9)->(A*1) seg T7->T8 fader f2
	mul bud
	mul parent_model

verb tw1:
	like twist1
	lifespan 24b
verb tw2:
	like twist2
	lifespan 24b
verb tw3:
	like twist3
	lifespan 24b
verb tw4:
	like twist4
	lifespan 24b
verb tw5:
	like twist1
	lifespan 24b
verb tw6:
	like twist2
	lifespan 24b
verb tw7:
	like twist3
	lifespan 16b
verb tw8:
	like twist4
	lifespan 8b

verb zoomin:
	move cam 0,0 -> 1,1 seg 7b->8b
	lifespan 8b

alias S better

branch part2:
	stage2 S.panel1 delay p+8b:
		tw5 S.knob5 delay p+0b:
			stage2 S.panel1 delay p+8b:
				tw6 S.knob6 delay p+0b:
					stage2 S.panel1 delay p+8b:
						tw7 S.knob7 delay p+0b:
							stage2e S.panel1 delay p+8b:
								tw8 S.knob8 delay p+0b:
									zoomin none delay p+0b
	
stage S.panel1:
	tw1 S.knob1:
		stage S.panel1 delay p+8b:
			tw2 S.knob2 delay p+0b:
				stage S.panel1 delay p+8b:
					tw3 S.knob3 delay p+0b:
						stage S.panel1 delay p+8b:
							tw4 S.knob4 delay p+0b:
								ref part2
