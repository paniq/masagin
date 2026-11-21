# 07_sugarfairy

bpm 130
song "07_sugarfairy.ogg"
bgcolor 1,1,1

alias S astronaut

verb dolly:
	mul root
	mul bud
	mul parent_model
	
	mul root modelcam
	move modelcam -0.5,-0.5
	mul bud modelcam
	mul parent_model modelcam
	
	scale color 1,1,1 -> 1,1,0 time 1b
	set bgcolor color
	
	move cam 1,1 seg 64b
	move cam 1,1 -> 0,0 seg 64b->65b
	lifespan 65b	

verb starzoom:
	mul root
	logscale 100% -> 0% time 8b
	mul bud
	mul parent_model
	lifespan 8b

verb zoom1:
	logscale 100% -> 0% time 32b
	lifespan 32b

verb zoom5:
	logscale 100% -> 4% time 20b
	lifespan 28b
	
verb zoom2:
	logscale 100% -> 0% time 16b
	lifespan 16b

verb zoom2r:
	rotate 0°->360° time 16b
	like zoom2

verb zoom3:
	logscale 100% -> 0% time 32b
	lifespan 32b

verb zoom4:
	logscale 100% -> 2% time 40b
	scale color 1,1,1,1 -> 1,1,1,0 time 20b->28b
	lifespan 28b	

verb starfieldbase:
	mul root
	scale 1,1,(random 0.01 to 0.1) uniform
	move 2.0,0.0
	rotate (random 0° to 360°)
	logscale 100% -> 0% time (random 16b to 32b)
	mul bud
	mul parent_model
	lifespan 8b
	
fader bounce:
	type spring
	k 2000.0
	mass 1.0
	friction 0.6
	
verb flash:
	scale 0% -> 100% fader bounce
	lifespan 32b
	
verb starfieldfirst:
	
verb starfield:
	like starfieldbase
	delay p+s+4t
	
verb starfieldx:
	mul root
	scale 1,1,(random 0.01 to 0.1) uniform
	move 2.0,0.0
	rotate (random 0° to 360°)
	logscale 100% -> 0% time 64b
	mul bud
	mul parent_model
	scale color 1,1,1,1 -> 1,1,1,0 time 6b->8b
	lifespan 8b
	delay p+s+4t

verb zoomstars:
	move 0,-0.5
	logscale 200% -> 100% time 40b
	scale color 1,1,1,1 -> 1,1,1,0 time 32b->40b
	lifespan 40b	

dolly none:
	zoom5 S.nasa delay 44b
	zoomstars starfield
	#zoom4 S.smoke
	starfieldbase S.twinkle3
	starfield*120 S.twinkle3
	starfieldbase S.twinkle2
	starfield*120 S.twinkle2
	starfieldbase S.twinkle1
	starfield*120 S.twinkle1
	starfieldx*20 S.twinkle1
	zoom1 S.planet
	zoom2 S.satelite delay 4b
	zoom3 S.saturn delay 8b
	zoom2r S.rocket delay 16b
	starzoom S.star delay 20b
	zoom1 S.spaceadv delay 22b:
		flash@1 S.spectacular delay p+(9b+8t)
		flash@2 S.miraculous delay p+(9b+8t)
	zoom1 S.galaxy delay 36b
