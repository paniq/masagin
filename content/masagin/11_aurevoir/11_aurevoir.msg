# 11_aurevoir

bpm 130
song "11_aurevoir.ogg"
bgcolor 0,0,0

verb grow1:
	mul root
	mul bud
	mul parent_model
	mul root modelcam
	move modelcam -0.5,-0.6
	logscale modelcam 1% -> 100% time 4b
	move modelcam 0.5,0.6
	mul bud modelcam
	mul parent_model modelcam
	move cam 1,1,1 -> 1,1,1 seg 4b
	move cam 1,1,1 -> 0,0,1 seg 4b->16b

verb grow2:
	move cam 0,0,1 -> 1,1,1 seg 4b->16b
	move cam 1,1,1 -> 0,0,1 seg 16b->24b

verb grow3:
	move cam 0,0,1 -> 1,1,1 seg 16b->24b
	move cam 1,1,1 -> 0,0,1 seg 24b->32b

verb grow4:
	move cam 0,0,1 -> 1,1,1 seg 24b->32b
	move cam 1,1,1 -> 0,0,1 seg 32b->40b
	move color 0.33,0,0

verb grow5:
	move cam 0,0,1 -> 1,1,1 seg 32b->40b
	move cam 1,1,1 -> 0,0,1 seg 40b->48b
	move color 0.33,0,0

verb grow6:
	move cam 0,0,1 -> 1,1,1 seg 40b->48b
	move cam 1,1,1 -> 0,0,1 seg 48b->56b
	move color 0.33,0,0

verb grow7:
	move cam 0,0,1 -> 1,1,1 seg 48b->56b
	move cam 1,1,1 -> 0,0,0 seg 56b->64b
	move color 1,0,0 -> 0,0,0 time 16b 

verb grow8:
	move cam 0,0,1 -> 1,1,0 seg 56b->64b
	move cam 1,1,0 -> 0,0,0 seg 64b->72b
	move color 0.66,0,0

grow1 earth lifespan 64b:
	grow2 escher lifespan 64b:
		grow3 strawberries lifespan 64b:
			grow4 earth2 lifespan 64b:
				grow5 escher lifespan 64b:
					grow6 strawberries lifespan 64b:
						grow7 pq lifespan 72b:
							grow8 earth2(name=scene11end) lifespan 72b
