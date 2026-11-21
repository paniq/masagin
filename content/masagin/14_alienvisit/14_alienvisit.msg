# 14_alienvisit

bpm 130
song "14_alienvisit.ogg"
bgcolor 0.556,1.0,0.19

verb S1:
	mul root
	move 0,-0.5
	scale 0% -> 100% time 1b
	move 0,0.5
	mul bud
	mul parent_model
	move cam 0,0 -> 1,1 seg 1b
	move cam 1,1 -> 0,0 seg 1b->8b

verb S2:

verb S3:
	move cam 0,0 -> 1,1 seg 0b->8b 
	move cam 1,1 -> 0,0 seg 8b->16b 
	
verb S4:
	move cam 0,0 -> 1,1 seg 8b->16b 
	move cam 1,1 -> 0,0 seg 16b->24b 

verb S5:
	move cam 0,0 -> 1,1 seg 16b->24b 
	move cam 1,1 -> 0,0 seg 24b->32b 

verb S6:
	move cam 0,0 -> 1,1 seg 24b->32b 
	move cam 1,1 -> 0,0 seg 32b->48b 

verb S7:
	move cam 0,0 -> 1,1 seg 32b->48b 
	move cam 1,1 -> 0,0 seg 48b->56b 

verb S8:
	move cam 0,0 -> 1,1 seg 48b->56b 
	move cam 1,1 -> 0,0 seg 56b->64b 

verb S9:
	move color 0,0,0 -> 1,0,0 time 4t loop
	move cam 0,0 -> 1,1 seg 56b->64b
	move cam 1,1 -> 0,0 seg 64b->65b 

alias S aliens

S1@2 S.castle lifespan 11b:
	S2 S.teeth lifespan 17b:
		S3 S.doctor lifespan 23b:
			S4 S.station lifespan 30b:
				S5 S.alien lifespan 49b:
					S6 S.dna lifespan 53b:
						S7 S.bob lifespan 65b:
							S8 S.eye lifespan 64b:
								S9 S.hoffi(name=scene14end) lifespan 65b
