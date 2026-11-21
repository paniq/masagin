# 08_itsallright

bpm 130
song "08_itsallright.ogg"
bgcolor 0,0,0

fader explog:
	type log
	exp1 0.1
	exp2 1

fader smooth:
	type polynome
	exp1 -0.5
	exp2 -0.5

fader bounce:
	type spring
	k 1000
	mass 1.0
	friction 0.7

verb basegrow:
	move color 0,0,0.05
	mul root
	move -0.5,-0.5
	rotate 50°
	store modelcam
	scale 0% -> 100% time 1b fader smooth
	scale 100% -> 200% time 1b->2b fader bounce
	move 0.5,0.5
	move 2.0,0.0
	mul anchor
	mul parent_model

	scale modelcam 400% -> 200% time 3b
	rotate modelcam -20°
	move modelcam 0.5,0.5
	move modelcam 2.0,0.0	
	mul anchor modelcam
	mul parent_model modelcam

	move cam 0,0,0 -> 1,1,1 seg 0b->1b
	move cam 1,1,1 -> 1,1,1 seg 1b->2b
	move cam 1,1,1 -> 0,0,1 seg 2b->3b fader explog
	
	lifespan 6b

verb grow:
	move color 0,0,0.05
	mul root
	move -0.5,-0.5
	rotate 50°
	store modelcam
	scale 0% -> 100% time 1b fader smooth
	scale 100% -> 200% time 1b->2b fader bounce
	move 0.5,0.5
	move 2.0,0.0
	mul anchor
	mul parent_model

	scale modelcam 400% -> 200% time 3b
	rotate modelcam -20°
	move modelcam 0.5,0.5
	move modelcam 2.0,0.0	
	mul anchor modelcam
	mul parent_model modelcam

	move cam 0,0,1 -> 1,1,1 seg 0b->1b
	move cam 1,1,1 -> 1,1,1 seg 1b->2b
	move cam 1,1,1 -> 0,0,1 seg 2b->3b fader explog
	
	lifespan 6b
	delay p+2b
	
verb lastgrow:
	move color 0,0,0.05
	mul root
	move -0.5,-0.5
	rotate 50°
	store modelcam
	scale 0% -> 100% time 1b fader smooth
	scale 100% -> 200% time 1b->2b fader bounce
	move 0.5,0.5
	move 2.0,0.0
	mul anchor
	mul parent_model

	scale modelcam 400% -> 200% time 3b
	rotate modelcam -20°
	move modelcam 0.5,0.5
	move modelcam 2.0,0.0	
	mul anchor modelcam
	mul parent_model modelcam

	move cam 0,0,1 -> 1,1,1 seg 0b->1b
	move cam 1,1,1 -> 1,1,1 seg 1b->2b
	move cam 1,1,1 -> 0,0,1 seg 2b->4b fader explog
	
	lifespan 4b
	delay p+2b

verb takecam:
	delay p+0b
	move cam 0,0 -> 1,1 seg 2b->4b
	lifespan 4b

basegrow austria:
	grow belgium:
		grow canada:
			grow china:
				grow cuba:
					grow denmark:
						grow finland:
							grow france:
								grow germany:
									grow greece:
										grow hongkong:
											grow hungary:
												grow iran:
													grow israel:
														grow italy:
															grow japan:
																grow netherlands:
																	grow norway:
																		grow palestine:
																			grow poland:
																				grow portugal:
																					grow romania:
																						grow russia:
																							grow southkorea:
																								grow spain:
																									grow sweden:
																										grow switzerland:
																											grow turkey:
																												grow uk:
																													grow ukraine:
																														lastgrow usa:
																															takecam none


