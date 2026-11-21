# 01_flutepart

bpm 130
song "01_flutepart.ogg"
bgcolor 1,1,1

# 0b 6b 8b

verb grow1:
	scale color 1,1,1,0 -> 1,1,1,1 time 2b
	move cam 1,1 -> 0,0 seg 6b
	lifespan 32b

verb grow2:
	move cam 0,0 -> 1,1 seg 4b->6b
	move cam 1,1 -> 0,0 seg 6b->8b
	lifespan 32b

verb grow3:
	move cam 0,0 -> 1,1 seg 6b->8b
	move cam 1,1 -> 0,0 seg 8b->16b
	lifespan 32b

verb grow4:
	move cam 0,0 -> 1,1 seg 8b->16b
	move cam 1,1 -> 0,0 seg 16b->24b
	lifespan 32b

verb grow5:
	scale color 1,1,1,1 -> 1,1,1,0 time 28b->32b
	move cam 0,0 -> 1,1 seg 16b->24b
	move cam 1,1 -> 0,0 seg 24b->32b
	lifespan 35b

fader bounce:
	type spring
	k 2000
	mass 1.0
	friction 0.6

verb entry:
	scale 120%,0% -> 100%,100% fader bounce
	
fader bob:
	type sine
	
verb monkin:
	like entry
	#move 0,0 -> 0,0.05 time 19b->20b loop fader bob

verb growtree:
	like entry

verb asked1:
	scale texture 0,0 -> 1,0 time 1b
	scale texture 1,0 -> 0,0 time 10b->12b
	lifespan 12b

verb slide:
	scale texture 0,0 -> 1,0 time 1b
	scale color 1,1,1,1 -> 1,1,1,0 time 12b->16b
	lifespan 16b

verb q1:
	scale texture 0,0 -> 1,0 time 1b
	scale texture 1,0 -> 0,0 time 2b->4b
	lifespan 4b

verb q2:
	scale texture 0,0 -> 1,0 time 1b
	scale texture 1,0 -> 0,0 time 3b->6b
	lifespan 6b
	
verb tsaid:
	scale color 1,1,1,0 -> 1,1,1,1 time 1b

verb rake:
	scale texture 0,0 -> 1,0 seg 32b
	move cam 1,1 -> 1,1 seg 27b
	move cam 1,1 -> 0,0 seg 27b->32b
	lifespan 32b

verb colorblend:
	move color 0.227,1.0,0.3647 seg 31b
	move color 0.227,1.0,0.3647 -> 0.227,0.0,1.0 seg 32b->35b
	move color 0.227,0.0,1.0 seg 35b->64b
	set bgcolor color

verb slidegrowmask:
	options zclear1+zwrite+alpha+texture
	scale texture 0,1 -> 1.5,1 time 4b
	scale texture 1.5,1 -> 0,1 time 6b->8b
	lifespan 8b

verb slidegrow:
	options color+alpha+texture+zread
	scale texture 0,1 -> 1.5,1 time 4b
	scale texture 1.5,1 -> 0,1 time 6b->8b
	lifespan 8b

verb slidegrowmask4:
	options zclear1+zwrite+alpha+texture
	scale texture 0,1 -> 1.5,1 time 2b
	scale texture 1.5,1 -> 0,1 time 2b->4b
	lifespan 4b

verb slidegrow4:
	options color+alpha+texture+zread
	scale texture 0,1 -> 1.5,1 time 2b
	scale texture 1.5,1 -> 0,1 time 2b->4b
	lifespan 4b
	
verb setx:
	options color+zread+alpha
	lifespan 8b

verb setx4:
	options color+zread+alpha
	lifespan 4b
	
verb setmask:
	options zclear1+zwrite
	lifespan 8b

verb setmask4:
	options zclear1+zwrite
	lifespan 4b
	
fader logmove:
	type log
	exp1 1.0
	exp2 0.0

verb slide1:
	move -0.8,0 -> 0.2,0.0 time 16b fader logmove
	lifespan 8b

verb slide2:
	move -0.05,0.03 -> 0,0.0 time 8b
	lifespan 8b

verb slide2b:
	move 0.0,0.0 -> -0.05,0.01 time 8b
	lifespan 8b
	
verb fadeinout2:
	scale color 1,1,1,0 -> 1,1,1,1 time 2b
	scale color 1,1,1,1 -> 1,1,1,0 seg 6b->8b
	lifespan 8b

verb fadeinout1:
	scale color 1,1,1,0 -> 1,1,1,1 time 4b->6b
	scale color 1,1,1,1 -> 1,1,1,0 seg 14b->16b
	lifespan 16b

verb slide3:	
	move -0.5,-0.5
	rotate -10° -> 0° time 8b
	move 0.5,0.5
	mul root
	mul bud
	mul parent_model
	lifespan 8b

verb slide3b:	
	move -0.5,-0.5
	rotate 0° -> 1° time 4b
	move 0,0 -> 0.01,0 time 4b
	move 0.5,0.5
	mul root
	mul bud
	mul parent_model
	lifespan 4b
	
verb slide4:	
	move -0.5,-0.5
	scale 50% -> 100% time 8b
	move 0.5,0.5
	mul root
	mul bud
	mul parent_model
	lifespan 8b

verb stage:
	scale color 0,0,0,1
	mul root
	mul bud
	mul parent_model
	
	mul root modelcam
	move modelcam 0.0,0.0
	mul bud modelcam
	mul parent_model modelcam
	
	move color 1,1,1
	set bgcolor color
	
	move cam 1,1 seg 32b
	lifespan (64b+8b)

alias M masagin
alias S slides

fader expmove:
	type log
	exp1 0
	exp2 1

verb camup:
	mul root
	mul bud
	mul parent_model
	
	mul root modelcam
	scale modelcam 100%
	mul bud modelcam
	mul parent_model modelcam
	
	move cam 0,0 -> 1,1 seg 5b fader expmove

branch story:
	rake M.theater delay 32b:
		monkin@1 M.monk delay 34b lifespan 32b
		asked1@2 M.asked delay 37b:
			q1 M.what delay 43b
			q2 M.buddha delay 46b
		entry@3 M.tozan delay 38b lifespan 32b
		growtree@5 M.tree delay 42b lifespan 32b
		tsaid@4 M.tozansaid delay 53b lifespan 16b:
			slide M.masagin(name=scene01end) delay 59b:
				camup none delay 59b lifespan 8b


stage S.stage:
	fadeinout1 S.presents delay 0b
	fadeinout2 S.aprod delay 16b
	fadeinout2 S.aninvito delay 24b
	slide1 none delay 0b:
		setmask S.nvision08 delay p+0b
		slidegrow S.nvslide delay p+0b
	slide2 none delay 8b:
		setmask S.bp delay p+0b
		slidegrow S.bpslide delay p+0b
	slide2b none delay 8b:
		slidegrowmask S.sceneslide delay p+0b
		setx S.sceneorg delay p+0b
	slide3 none delay 16b:
		slidegrowmask S.frslide delay p+0b
		setx S.farbrausch delay p+0b
	slide3b none delay 20b:
		slidegrowmask4 S.nslide delay p+0b
		setx4 S.neuro delay p+0b
	slide4 none delay 24b:
		slidegrowmask S.nvscslide delay p+0b
		setx S.nvscene delay p+0b
	ref story
