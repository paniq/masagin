
bgcolor 0,0,0

verb stage:
	move cam 1,1
	lifespan 101
	
verb colorfade:
	move color 0,0,0,1 -> 0,0,1,1 time 90->100
	set bgcolor color
	lifespan 101

verb grow:
	scale 1,0 -> 1,1 time 100
	lifespan 101

stage loader.main:
	colorfade none
	grow loader.loadbar
