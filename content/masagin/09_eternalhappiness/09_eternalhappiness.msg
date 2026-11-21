# 09_eternalhappiness

bpm 130
song "09_eternalhappiness.ogg"
bgcolor 0,0,0

const LENGTH 64b

fader spr:
	k 1000.0
	mass 500.0
	friction 0.99

verb stage:


verb roam:
	scale color 0,0,0,0
	scale 0.9
	move 0,0 -> 0,4 time 8b->60b
	move 0,0 -> 0,2 time 60b->64b
	move cam 0,0 -> 1,1 seg 0b -> 8b
	move cam 1,1 seg 8b -> 64b

const ET 1
const G 4
const P 0.01

verb grow1:
	scale color 1,1,1,0 -> 1,1,1,1 time 1b->2b
	move 0,0 -> 0,1 time 8b->60b
	scale texture 0,1,1,G -> ET,1,1,G time LENGTH
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb grow2:
	scale color 1,1,1,0 -> 1,1,1,1 time 1b->2b
	move 0,0 -> 0,2 time 8b->60b
	#like cycle
	scale texture 0,1,1,G -> 0.6048,1,1,G seg 32b
	scale texture 0.6048,1,1,G -> ET,1,1,G seg 32b->LENGTH
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb grow3:
	move 0,0 -> 0,-1 time 8b->60b
	#like cycle
	scale texture 0,1,1,G -> ET,1,1,G time 8b->LENGTH
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb grow4:
	move 0,0 -> 0,-2 time 8b->60b
	#like cycle
	scale texture 0,1,1,G -> ET,1,1,G time 16b->LENGTH
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb grow5:
	#like cycle
	scale texture 0,1,1,G -> ET,1,1,G time 32b->LENGTH
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb grow6:
	#like cycle
	scale texture 0,1,1,G -> ET,1,1,G time 48b->LENGTH
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb rise1:
	scale color 1,1,1,0 -> 1,1,1,1 time 1b->2b
	#like cycle
	mul root
	move 0,-0.5
	turn -40°
	route@1 turn P,P,0 -> P,P,1 time LENGTH
	move 0,0.5
	mul bud
	route@1 move 0 -> 1 time LENGTH
	mul parent
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb rise2:
	scale color 1,1,1,0 -> 1,1,1,1 time 1b->2b
	#like cycle
	mul root
	move 0,-0.5
	turn -135°
	route@1 turn P,P,0 -> P,P,0.6 seg 32b
	route@1 turn P,P,0.6 -> P,P,1 seg 32b->LENGTH
	move 0,0.5
	mul bud
	route@1 move 0 -> 0.6 seg 32b
	route@1 move 0.6 -> 1 seg 32b->LENGTH
	mul parent
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb rise3:
	#like cycle
	scale color 1,1,1,0 seg 8b
	mul root
	move 0,-0.5
	turn -170°
	route@1 turn P,P,0 -> P,P,1 time 8b->LENGTH
	move 0,0.5
	mul bud
	route@1 move 0 -> 1 time 8b->LENGTH
	mul parent
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb rise4:
	#like cycle
	scale color 1,1,1,0 seg 16b
	mul root
	move 0,-0.5
	turn -15°
	route@1 turn P,P,0 -> P,P,1 time 16b->LENGTH
	move 0,0.5
	mul bud
	route@1 move 0 -> 1 time 16b->LENGTH
	mul parent
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb rise5:
	#like cycle
	scale color 1,1,1,0 seg 32b
	mul root
	move 0,-0.5
	turn -180°
	route@1 turn P,P,0 -> P,P,1 time 32b->LENGTH
	move 0,0.5
	mul bud
	route@1 move 0 -> 1 time 32b->LENGTH
	mul parent
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

verb rise6:
	#like cycle
	scale color 1,1,1,0 seg 48b
	mul root
	move 0,-0.5
	turn -15°
	route@1 turn P,P,0 -> P,P,1 time 48b->LENGTH
	move 0,0.5
	mul bud
	route@1 move 0 -> 1 time 48b->LENGTH
	mul parent
	move color 0,0,0,0 -> 1,0,0,0 time LENGTH

alias GROW grow1

verb passby:
	move 1,0 -> -1,0 time 64b
	move 0,0 -> 0,4.5 time 8b->60b
verb passby2:
	move 0.5,0 -> -0.5,0 time 64b
	move 0,0 -> 0,6 time 8b->60b
verb passby3:
	scale color 1,1,1,0 -> 1,1,1,1 time 1b

verb passby4:
	scale 0 -> 1 time 8b fader spr
	rotate 20°->-60° time 64b
	move color 0,0,0,0 -> 2,0,0,0 time LENGTH
	move 0,2.82 -> 0,6.1 time 28b
	
verb shine:
	scale 2
	move color 0,0,0,0 -> 2,0,0,0 time LENGTH
	rotate 0°->-360° time 24b loop

verb shine2:
	move color 2,0,0,0 -> 0,0,0,0 time LENGTH
	rotate 0°->360° time 26b loop

verb colorfade:
	move color 0.56,1,1 -> 0.56,1,0.8117 time 2b
	set bgcolor color
	lifespan 8b
	
verb initfade:
	move cam 1,1 -> 0,0 seg 0b -> 8b
	lifespan 8b
	
fader hop:
	type sine
	cycles 8
	phase -90°
	
verb bunnyhop:
	scale 180%
	delay p+16b
	rotate -1°->1° time 3b loop fader hop
	move 0,-0.4 -> 0,2 time 16b fader hop
	lifespan 16b

fader bloomfader:
	type spring
	k 1000.0
	mass 1.0
	friction 0.8

fader schunkel:
	type sine

verb bloom:
	scale 1,1,(random 120% to 180%) uniform
	scale 0%,110% -> 1,1 fader bloomfader
	skew -5°,0 -> 5°,0 time 4b loop fader schunkel
	lifespan 16b

verb bloom2:
	scale 1,1,(random 120% to 180%) uniform
	scale 0%,110% -> 1,1 fader bloomfader
	skew -5°,0 -> 5°,0 time 4b loop fader schunkel
	scale -1,1
	lifespan 16b
	
verb rainbase:
	move -0.5,-0.5
	rotate (random 0° to 360°)
	rotate 0° -> 270° time 12b
	scale 1,1,(random 1% to 2%) uniform
	move (random 0.8 to 1.1),0.5
	move 0,0 -> 0,-0.4 time (random 8b to 12b)
	mul root
	mul bud
	mul parent_model
	lifespan 12b
	
verb rain:
	move -0.5,-0.5
	rotate (random 0° to 360°)
	rotate 0° -> 270° time 12b
	scale 1,1,(random 1% to 2%) uniform
	move (random 0.8 to 1.1),0.5
	move 0,0 -> 0,-0.4 time (random 8b to 12b)
	mul root
	mul bud
	mul parent_model
	lifespan 12b
	delay p+s+4t
	
stage rainbow.scene lifespan 64b:
	colorfade none
	passby4@9 rainbow.sonne delay 32b lifespan 32b:
		shine! rainbow.strahlen delay p+0b lifespan 32b
		shine2! rainbow.strahlen delay p+0b lifespan 32b
	passby2@8 rainbow.clouds2 lifespan 64b
	passby@8 rainbow.clouds1 lifespan 64b
	GROW@2 rainbow.trail1 lifespan 64b:
		rise1 rainbow.star1 lifespan 64b
	grow2@3 rainbow.trail2 lifespan 64b:
		rise2 rainbow.star2 lifespan 64b
	grow3@4 rainbow.trail3 lifespan 64b:
		rise3 rainbow.star3 lifespan 64b
	grow4@5 rainbow.trail4 lifespan 64b:
		rise4 rainbow.star4 lifespan 64b
	grow5@6 rainbow.trail5 lifespan 64b:
		rise5 rainbow.star5 lifespan 64b
	grow6@7 rainbow.trail6 lifespan 64b:
		rise6 rainbow.star6 lifespan 64b
	passby3@8 rainbow.clouds lifespan 64b:
		initfade@1 none
		bunnyhop!@2 bunny
		bloom2!@3 sunflower3 delay p+29b
		bloom!@10 sunflower3 delay p+31b
		bloom!@4 rose delay p+41b
		bloom2!@5 rose delay p+42b
		bloom2!@6 rose delay p+43b
	roam@1 rainbow.dolly lifespan 64b:
		rainbase@1 heart delay p+s+46b
		rain@1*16 heart delay p+s+8t
