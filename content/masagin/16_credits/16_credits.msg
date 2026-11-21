# 16_credits

bpm 130
song "16_credits.ogg"
bgcolor 1,1,1

verb stage:
	rotate 90°
	move 0.5,0
	move cam 0,0 -> 1,1 seg 1b
	move cam 1,1 seg 1b->64b
	scale color 0,0,0,0 -> 0,0,0,0 time 1b
	set bgcolor color
	lifespan 64b

verb blend:
	scale color 1,1,1,0 -> 1,1,1,1 time 4b
	scale color 1,1,1,1 -> 1,1,1,0 time 12b -> 16b
	lifespan 16b

alias S credits

stage S.backdrop:
	blend S.who
	blend S.thanks delay 16b
	blend S.visit delay 32b
	blend S.url delay 48b
