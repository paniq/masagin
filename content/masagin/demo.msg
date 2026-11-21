

song "masagin.ogg"
bpm 130
bgcolor 1,1,1

import "01_flutepart/01_flutepart.msg" as scene01
import "02_bellydub/02_bellydub.msg" as scene02 delay (64b*1)
import "03_mysticdub/03_mysticdub.msg" as scene03 delay (64b*2)
import "04_stallingangle/04_stallingangle.msg" as scene04 delay (64b*3)
import "05_sidrevival/05_sidrevival.msg" as scene05 delay (64b*4)
import "06_betterland/06_betterland.msg" as scene06 delay (64b*5)
import "07_sugarfairy/07_sugarfairy.msg" as scene07 delay (64b*6)
import "08_itsallright/08_itsallright.msg" as scene08 delay (64b*7)
import "09_eternalhappiness/09_eternalhappiness.msg" as scene09 delay (64b*8)
import "10_turbulences/10_turbulences.msg" as scene10 delay (64b*9)
import "11_aurevoir/11_aurevoir.msg" as scene11 delay (64b*10)
import "12_readysteadygo/12_readysteadygo.msg" as scene12 delay (64b*11)
import "13_evolution/13_evolution.msg" as scene13 delay (64b*12)
import "14_alienvisit/14_alienvisit.msg" as scene14 delay (64b*13)
import "15_showdown/15_showdown.msg" as scene15 delay (64b*14)
import "16_credits/16_credits.msg" as scene16 delay (64b*15)

ref scene01:
	ref[scene01end] scene02:
		ref@2[scene02end] scene03:
			ref scene04:
				ref[scene04end] scene05:
					ref[scene05end] scene06:
						ref scene07:
							ref scene08:
								ref scene09:
									ref scene10:
										ref[scene10end] scene11:
											ref[scene11end] scene12:
												ref[scene12end] scene13:
													ref@2[scene13end] scene14:
														ref[scene14end] scene15:
															ref[scene15end] scene16
