import sys
from PIL import Image
import numpy as np
on=np.array(Image.open(sys.argv[1]).convert('L')).astype(float)
off=np.array(Image.open(sys.argv[2]).convert('L')).astype(float)
g=np.clip(on-off*(0xc0/255.0),0,None)               # glare contribution only
ys,xs=np.mgrid[0:g.shape[0],0:g.shape[1]]
cx=(g*(xs+0.5)).sum()/g.sum(); cy=(g*(ys+0.5)).sum()/g.sum()
m=g>1.5
# radius containing 90% of the glare energy around its own centroid
r=np.hypot(xs+0.5-cx,ys+0.5-cy); o=np.argsort(r.ravel()); cum=np.cumsum(g.ravel()[o])/g.sum(); r90=r.ravel()[o][np.searchsorted(cum,0.9)]
print(f"glare centroid shift ({cx-400:+.2f},{cy-150:+.2f}) px  energy {g.sum()/255:.0f}  r90 {r90:.1f}px  extent x {xs[m].min()}..{xs[m].max()} y {ys[m].min()}..{ys[m].max()}")
