# svgmesh loads a regular svg file and instructs a user-implemented callback to add vertices, mesh, textures, and so on.

import re
from xml.dom.minidom import parse as xmlparse
import traceback
from numpy import array
from numpy import dot as matrixmultiply
import os, math

from OpenGL.GLU import *
from OpenGL.GL import *

BEZIER_NCOUNT = 10
SIMPLIFY = False
S_ANGLESCALE = 4.5 * math.pi / 180.0

INKSCAPE_NS = "http://www.inkscape.org/namespaces/inkscape"

def log(*args):
  try:
    stack = traceback.extract_stack()
    if len(stack) >= 3:
      entry = traceback.extract_stack()[-2]
    else:
      entry = ('???','','<???>')
    name = os.path.splitext(os.path.basename(entry[0]))[0]
    line = entry[1]
    funcname = entry[2]
    print('  File "%s", line %s, in %s\n>>> %s' % (name, line, funcname, ' '.join([str(arg) for arg in args])))
  except:
    print(' '.join([str(arg) for arg in args]))

def dot (v,w):
    """Return the dot product of vectors v and w"""
    return sum( [ x*y for x,y in zip(v,w) ] )

def cross (v,w):
  """Return the cross product of two vectors."""
  return [ v[1]*w[2]-v[2]*w[1],  v[2]*w[0]-v[0]*w[2], v[0]*w[1]-v[1]*w[0] ]
  
def vcosAngle (v,w):
    """Return the cosine of the angle between two vectors"""
    return dot(v,w)/vlength(v)/vlength(w)

def vangle (v,w):
  """Return the angle between two vectors"""
  s = v[0]*w[1] - v[1]*w[0]
  if s == 0:
    s = 1
  s = s / abs(s)
  a = dot(v,w) / (vlength(v) * vlength(w))
  if a < -1.0:
    a = -1.0
  if a > 1.0:
    a = 1.0
  return math.acos(a) * s

def vsquare (v):
    """Return the squared components [x^2, y^2, z^2] of v[x,y,z]"""
    return [ x*x for x in v ]

def vnorm (v):
    """Return the square length [x^2 + y^2 + z^2 of a vector v"""
    return sum(vsquare(v))

def vlength (v):
    """Return the length of the vector v"""
    return math.sqrt(vnorm(v))

def vequals(v,w):
  return sum([a == b for a,b in zip(v,w)]) == len(v)

def vextents(vl):
  return [min([v[n] for v in vl]) for n in range(len(vl[0]))], [max([v[n] for v in vl]) for n in range(len(vl[0]))]

def vipol(v,w,p=0.5):
  return [(a+b)*0.5 for a,b in zip(v,w)]

def vsum (vl):
  """Returns the sum of all vectors in vl"""
  return [sum([v[n] for v in vl]) for n in range(len(vl[0]))]

def vscale (v,a):
    """Return the a * [x, y, z], where a is a scalar"""
    return [ a*x for x in v ]

def vdiff (v,w):
    """Return the difference vector v-w"""
    return [ x-y for x,y in zip(v,w) ]

def vnormalize(v):
  """Returns a normalized vector"""
  return vscale(v,1.0/vlength(v))

def vadd (v,w):
    """Return the sum of the vectors v and w"""
    return [ x+y for x,y in zip(v,w) ]

def determinant2d(u,v):
  return (u[1] * v[0]) - (u[0] * v[1])

def intersection(a,u,b,v):
  """Returns the intersection point of two 2d vectors"""
  d = (u[1] * v[0]) - (u[0] * v[1])
  assert d, 'cant intersect: a=%s, u=%s, b=%s, v=%s' % (repr(a),repr(u),repr(b),repr(v))
  s = ((b[1] - a[1]) * v[0] - (b[0] - a[0]) * v[1]) / d
  return vadd(a, vscale(u, s))

def transform_points_2d(points, matrix):
  return [transform_point_2d(p, matrix) for p in points]

def transform_point_2d(xxx_todo_changeme4, matrix):
  (x,y) = xxx_todo_changeme4
  ((x,),(y,),(z,)) = matrixmultiply(matrix,[[x],[y],[1.0]])
  return x,y

def transform_point_3d(xxx_todo_changeme5, matrix):
  (x,y,z) = xxx_todo_changeme5
  ((x,),(y,),(z,),(w,)) = matrixmultiply(matrix,[[x],[y],[z],[1.0]])
  return x,y,z

def transform_point_4d(xxx_todo_changeme6, matrix):
  (x,y,z,w) = xxx_todo_changeme6
  ((x,),(y,),(z,),(w,)) = matrixmultiply(matrix,[[x],[y],[z],[w]])
  return x,y,z,w
  
def projection(v,w):
  return dot(v,w)/vlength(w)

class CBezier:
  def __init__(self,pos1,pos2,pos3):
    self.pos1 = pos1
    self.pos2 = pos2
    self.pos3 = pos3
    
  def get_point(self,n):
    c1 = vscale(self.pos1,(1.0-n)**2.0)
    c2 = vscale(self.pos2,2*n*(1.0-n))
    c3 = vscale(self.pos3,n**2.0)
    return vsum([c1,c2,c3])

class QBezier:
  def __init__(self,pos1,pos2,pos3,pos4):
    self.pos1 = pos1
    self.pos2 = pos2
    self.pos3 = pos3
    self.pos4 = pos4
    
  def get_point(self,n):
    c1 = vscale(self.pos1,(1.0-n)**3.0)
    c2 = vscale(self.pos2,3*n*((1.0-n)**2.0))
    c3 = vscale(self.pos3,3*(n**2.0)*(1.0-n))
    c4 = vscale(self.pos4,n**3.0)
    return vsum([c1,c2,c3,c4])

class Texture:
  def __init__(self,width=512,height=512,path=None,internalformat=GL_RGB8,format=GL_RGB,pdtype=GL_INT,levels=None,unit=0, clamp=None,image=0,invert=False,transparent=False,noipol=False):
    assert unit < 16, 'illegal value for unit'
    self._unit = unit
    self._t = glGenTextures(1)
    self.bind()
    w,h = width,height
    if path:
      _log('loading texture "%s"' % path)			
      image_file = Image.open(path)
      w,h = image_file.size
      if transparent:
        image = image_file.tostring("raw", "RGBA", 0, -1)
        internalformat = GL_RGBA8
        format = GL_RGBA
      else:
        image = image_file.tostring("raw", "RGB", 0, -1)
        internalformat = GL_RGB8
        format = GL_RGB
      pdtype = GL_UNSIGNED_BYTE			
      if clamp == None:
        clamp = GL_REPEAT
    if clamp == None:
      clamp = GL_CLAMP
    if levels == None:
      if image:
        levels = int((log(float(max(w,h))) / log(2.0))-1.0)
      else:
        levels = 1
    if levels == 1:
      glTexImage2DEXT(GL_TEXTURE_2D, 0, internalformat, w, h, 0, format, pdtype, image)
    else:
      gluBuild2DMipmaps(GL_TEXTURE_2D, internalformat, w, h, format, pdtype, image)
    if internalformat in (GL_RGB8, GL_RGBA8):
      if levels == 1:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
      elif noipol:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
      else:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)			
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp)
    elif internalformat in (GL_DEPTH_COMPONENT16_ARB,GL_DEPTH_COMPONENT24_ARB, GL_DEPTH_COMPONENT32_ARB):
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB)
      glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_LUMINANCE)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
      self.ATTACHMENT = GL_DEPTH_ATTACHMENT_EXT
      
  def set_max_lod(self, lod):
    from gl import GL_TEXTURE_MAX_LOD
    self.bind()
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, lod)
      
  def get_unit(self):
    return self._unit
      
  def select_unit(self):
    select_unit(self._unit)
    
  def bind(self):
    self.select_unit()
    glBindTexture(GL_TEXTURE_2D, self._t)
        
  def __del__(self):
    glDeleteTextures([self._t])

class Bitmap:
  _default_matrix_ = array([[1.0,0.0,0.0],[0.0,1.0,0.0],[0.0,0.0,1.0]])
  
  def __init__(self):
    self.reset_matrix()
    
  def reset_matrix(self):
    self.matrix = self._default_matrix_

  def transform_point(self, xxx_todo_changeme):
    (x,y) = xxx_todo_changeme
    return transform_point_2d((x,y),self.matrix)
    
  def inherit(self, bitmap):
    self.matrix = bitmap.matrix

  def get_image(self, size=(64,64), ofs=(0,0)):
    import Image
    img = Image.fromstring('RGBA',size,self.get_image_raw(size,ofs))
    return img
    
  def get_texture(self, size=(64,64), ofs=(0,0)):
    return Texture(width=size[0],height=size[1],internalformat=GL_RGBA8,format=GL_RGBA,pdtype=GL_UNSIGNED_BYTE,image=self.get_image_raw(size,ofs),clamp=GL_CLAMP)

class Pattern(Bitmap):
  _default_matrix_ = array([[1.0,0.0,0.0],[0.0,-1.0,0.0],[0.0,0.0,1.0]])
  
  def __init__(self):
    Bitmap.__init__(self)
    self.image = ''

  def inherit(self, pattern):
    Bitmap.inherit(self,pattern)
    self.image = pattern.image

  def get_image_raw(self, size=(64,64), ofs=(0,0)):
    import res
    image_file = Image.open(res.find(self.image))
    source = image_file.resize(size,1).tostring("raw", "RGBA", 0, -1)
    width,height = size
    data = [None]*(width*height)
    for p in range(width*height):
      x = p % width
      y = p / width
      x,y = self.transform_point((x,y))
      x = int(x) % width
      y = int(y) % height
      ps = (x + (y*width))*4
      data[p] = source[ps:ps+4]
    return ''.join(data)

class Gradient(Bitmap):
  def __init__(self):
    Bitmap.__init__(self)
    self.stops = {}
    self.radial = False
    self.pos1 = (0.0,0.0)
    self.pos2 = (1.0,1.0)
    self.scale = 1.0
    
  def inherit(self, gradient):
    Bitmap.inherit(self,gradient)
    self.stops = dict(gradient.stops)
    self.pos1 = gradient.pos1
    self.pos2 = gradient.pos2
    self.radial = gradient.radial
    
  def set_position(self, pos1, pos2):
    self.pos1 = pos1
    self.pos2 = pos2
    
  def get_position(self, transformed = False):
    if transformed:
      return self.transform_point(self.pos1), self.transform_point(self.pos2)
    else:
      return self.pos1, self.pos2
    
  def add_stop(self, offset, color):
    self.stops[offset] = color
    
  def get_sample(self, x,y):
    v = LIVector()
    for offset,color in self.stops.items():
      v[offset] = color
    if not self.radial:
      pos1 = self.transform_point(self.pos1)
      pos2 = self.transform_point(self.pos2)
      w = vdiff(pos2,pos1)
      wlen = vlength(w)
      offset = min(max(0,projection(vdiff((x,y),self.pos1),w)),wlen)
      return v[offset/wlen]
    else:
      rx, ry = vdiff(self.pos2, self.pos1)
      if not rx:
        rx = 0.0001
      if not ry:
        ry = 0.0001
      a = abs(vangle(self.pos2, self.pos1))
      dx, dy = vdiff(self.pos1, (x,y))
      d = vlength((dx/rx, dy/ry))
      n = d
      offset = int(min(max(0,d),1.0))
      return v[n]
    data = ''.join(data)
    return data

class Material:
  def __init__(self):
    self.ambient_texture = None
    self.diffuse_texture = None
    self.specular_texture = None
    self.ambient_color = (0.0, 0.0, 0.0)
    self.diffuse_color = (0.5, 0.5, 0.5)
    self.specular_color = (1.0, 1.0, 1.0)
    self.add = False
    self.shader_program = None
    self.shaderassembly = ''		
    self.alpha = 1.0
    self.illumination = 0.0
    self.blend = True
    self.texture_matrix = array([[1.0,0.0,0.0,0.0],[0.0,1.0,0.0,0.0],[0.0,0.0,1.0,0.0],[0.0,0.0,0.0,1.0]])
    self.funcs = []
    
  def animate_uniform(self, name):
    pass

  def wipe_remote(self):
    if self.shader_program:
      del self.shader_program
    self.funcs = []

  def get_program(self):
    return self.shader_program
    
  def set_pipeline(self,shaderassembly):
    self.shaderassembly = shaderassembly

  def render_shader(self):
    self.render_material()
    self.shader_program.use()
    
  def render_material(self):
    if self.blend:
      glEnable(GL_BLEND)
      if self.add:
        glBlendFunc(GL_SRC_ALPHA,GL_ONE)
      else:
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA)
    else:
      glDisable(GL_BLEND)
    glColorMaterial(GL_FRONT, GL_DIFFUSE)
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, (0.0, 0.0, 0.0, 1.0))
    #~ glMaterialfv(GL_FRONT, GL_DIFFUSE, self.diffuse_color)
    #~ glMaterialfv(GL_FRONT, GL_SPECULAR, self.specular_color)
    
    glEnable(GL_COLOR_MATERIAL)
    if self.illumination:
      illum_cof = 1.0 - self.illumination
      glColor4f(*(tuple(scale(self.diffuse_color,illum_cof)) + (self.alpha,)))
      glMaterialfv(GL_FRONT, GL_EMISSION, scale(self.diffuse_color,self.illumination) + [1.0])
    else:
      glMaterialfv(GL_FRONT, GL_EMISSION, (0.0,0.0,0.0,1.0))
      glColor4f(*(tuple(self.diffuse_color) + (self.alpha,)))
    
  def render_no_shader(self):
    GLSL.use_fixed()
    self.render_material()
    
  def render_no_textures(self):
    glDisable(GL_TEXTURE_2D)
    
  def render_diffuse_texture(self):
    glEnable(GL_TEXTURE_2D)
    #~ glMatrixMode(GL_TEXTURE)
    #~ glLoadMatrixf(self.texture_matrix)
    self.diffuse_texture.bind()
    
  def realize(self):
    log('realizing material')
    if self.shaderassembly:
      self.shader_program = GLSL.create_program(self.shaderassembly)
    if self.shader_program:
      self.funcs.append(self.render_shader)
    else:
      self.funcs.append(self.render_no_shader)
    if self.diffuse_texture:
      if type(self.diffuse_texture) in (str,str):
        log('loading diffuse texture "%s"' % self.diffuse_texture)
        self.diffuse_texture = texture.Texture(path=self.diffuse_texture)
      self.funcs.append(self.render_diffuse_texture)
    else:
      self.funcs.append(self.render_no_textures)

  #~ def apply_to_mesh(self, mesh):
    #~ mesh.set_texture(texture=self.diffuse_texture)

  def render(self,time=0.0):
    for func in self.funcs:
      func()

class Mesh:
  vertex_type = 'f'
  vertex_gltype = GL_FLOAT
  #~ vertex_type = 'd'
  #~ vertex_gltype = GL_DOUBLE
  vertex_stride = 4
  texcoord_stride = 2
  normal_stride = 3
  
  def __init__(self):
    self.vertices_id = None
    self.texcoords_id = None
    self.normal_id = None
    self.materials = []
    self.funcs = []
    self.wipe_local()
    
  def wipe_local(self):
    self.vertices = []
    self.faces = []
    self.texcoords = []
    self.texcoord_faces = []
    self.normals = []
    self.normal_faces = []
    self.smooth_groups = []
    
  def __del__(self):
    assert not self.vertices_id

  def add_texcoord(self,u,v):
    self.texcoords.append((u,v))
    
  def add_texcoord_face(self,t1,t2,t3):
    self.texcoord_faces.append((t1,t2,t3))
  
  def add_vertex(self,x,y,z,w=1.0):
    self.vertices.append((x,y,z,w))
    
  def add_face(self,v1,v2,v3,v4=None):
    if v4:
      self.faces.append((v1,v2,v3))
      self.faces.append((v1,v3,v4))
    else:
      self.faces.append((v1,v2,v3))

  def add_face4(self,v1,v2,v3,v4):
    self.faces.append((v1,v2,v3))
    self.faces.append((v1,v3,v4))
    
  def add_smooth_group(self, s):
    self.smooth_groups.append(s)
        
  def generate_normals(self):
    log('generating normals')
    def replace_vertice(self, i, inew):
      del self.vertices[i]
      for vfi in range(len(self.faces)):
        newface = []
        for f in self.faces[vfi]:
          if f == i:
            newface.append(inew)
          elif f > i:
            newface.append(f-1)
          else:
            newface.append(f)
        if newface != self.faces[vfi]:
          self.faces[vfi] = newface

    # remove duplicate vertices
    dupemap = {}
    for vi1 in range(len(self.vertices)):
      for vi2 in reversed(range(len(self.vertices))):
        if vi1 != vi2:
          v1 = self.vertices[vi1]
          v2 = self.vertices[vi2]
          if v1 == v2:
            ni = min(vi1,vi2)
            oi = max(vi1,vi2)
            if not oi in dupemap:
              dupemap[oi] = ni						
    for oi in reversed(sorted(dupemap.keys())):
      ni = dupemap[oi]
      replace_vertice(self,oi,ni)

    def create_vi2fi(self):
      # create a dict of vertice indices to face indices
      v2f = {}
      for fi in range(len(self.faces)):
        f = self.faces[fi]
        for fvi in range(len(f)):
          vi = f[fvi]
          fis = v2f.get(vi,[])
          if not fi in fis:
            # append position of vertice and face index
            fis.append((fvi,fi))
            v2f[vi] = fis
      return v2f
      
    # duplicate vertices which share different smoothing groups
    def duplicate_vertice(self, i):
      newindex = len(self.vertices)
      self.vertices.append(self.vertices[i])
      return newindex

    if self.smooth_groups:
      vi2fi = create_vi2fi(self)
      vi2sgs = {}
      # collect smoothing groups
      for vi in range(len(self.vertices)):
        sgs = {}
        for n,vfi in vi2fi[vi]:
          sg = self.smooth_groups[vfi]
          sgn = sgs.get(sg,[])
          sgn.append((n,vfi))
          sgs[sg] = sgn
        vi2sgs[vi] = sgs
      
      # duplicate vertices and update faces
      for vi,sgn in vi2sgs.items():
        for sg,nvfi in list(sgn.items())[1:]:
          nvi = duplicate_vertice(self,vi)
          for n,vfi in nvfi:
            f = list(self.faces[vfi])
            f[n] = nvi
            self.faces[vfi] = f

    # calculate flat normals
    vncount = 0
    for vf in self.faces:
      v1 = self.vertices[vf[0]]
      v2 = self.vertices[vf[1]]
      v3 = self.vertices[vf[2]]
      u = vector.diff(v3,v1)
      v = vector.diff(v1,v2)
      w = vector.cross(u,v)			
      length = vector.length(w)
      if length:
        w = vector.scale(w,1.0/length)
      self.add_normal(*w)
      self.add_normal_face(vncount,vncount,vncount)
      vncount += 1
      
    # smooth normals
    vi2fi = create_vi2fi(self)
    nbase = len(self.normals)		
    oldnormals = self.normals
    self.normals = []
    for vi in range(len(self.vertices)):
      vnf = []
      for n,vfi in vi2fi[vi]:
        vf = self.faces[vfi]				
        if vi in vf:
          vnf += [oldnormals[vfi]]
      w = vector.vsum(vnf)
      length = vector.length(w)
      if length:
        w = vector.scale(w,1.0/length)
      self.add_normal(*w)
      ### debug extrude
      #~ we = vector.scale(w + [0.0], 4.0)
      #~ self.vertices[vi] = vector.add(self.vertices[vi],we)
      ### debug
    self.normal_faces = self.faces

  def add_normal(self,x,y,z):
    self.normals.append((x,y,z))
    
  def add_normal_face(self,n1,n2,n3):
    self.normal_faces.append((n1,n2,n3))
    
  def get_size(self):
    minv,maxv = self.get_extents()
    minx,miny,minz = minv
    maxx,maxy,maxz = maxv
    w = maxx - minx
    h = maxy - miny
    d = maxz - minz
    return w,h,d
    
  def make_bounding_box_mesh(self):
    minv,maxv = self.get_extents()
    minx,miny,minz = minv
    maxx,maxy,maxz = maxv
    w = maxx - minx
    h = maxy - miny
    d = maxz - minz
    box = Cube(w,h,d,use_texcoords=False,use_normals=False)
    box.translate(minx+w/2,miny+h/2,minz+d/2)
    return box
    
  def get_extents(self):
    minx,maxx = 999999.0,-999999.0
    miny,maxy = 999999.0,-999999.0
    minz,maxz = 999999.0,-999999.0
    for x,y,z,w in self.vertices:
      minx = min(minx,x)
      maxx = max(maxx,x)
      miny = min(miny,y)
      maxy = max(maxy,y)
      minz = min(minz,z)
      maxz = max(maxz,z)
    return ((minx,miny,minz),(maxx,maxy,maxz))
    
  def get_vertex(self,index):
    return self.vertices[index]
    
  def translate(self,x,y,z):
    for n in range(len(self.vertices)):
      vx,vy,vz,vw = self.vertices[n]
      self.vertices[n] = (vx+x,vy+y,vz+z,vw)
      
  def apply_matrix(self, m):
    for n in range(len(self.vertices)):
      self.vertices[n] = vector.transform_point_4d(self.vertices[n],m)
      
  def apply_texcoords(self, func, *args):
    for n in range(len(self.texcoords)):
      self.texcoords[n] = func(self.texcoords[n], *args)
    
  def merge(self,mesh):
    ofsv = len(self.vertices)
    ofsvt = len(self.texcoords)
    ofsvn = len(self.normals)
    self.vertices += mesh.vertices
    for face in mesh.faces:
      f1,f2,f3 = face
      self.faces.append((f1+ofsv,f2+ofsv,f3+ofsv))
    self.texcoords += mesh.texcoords
    for texcoord_face in mesh.texcoord_faces:
      f1,f2,f3 = texcoord_face
      self.texcoord_faces.append((f1+ofsvt,f2+ofsvt,f3+ofsvt))
    self.normals += mesh.normals
    for normal_face in mesh.normal_faces:
      f1,f2,f3 = normal_face
      self.normal_faces.append((f1+ofsvn,f2+ofsvn,f3+ofsvn))
    self.materials += mesh.materials
    
  def upload(self):
    log('uploading')
    self.wipe_remote()
    
    vertex_stride = self.vertex_stride
    texcoord_stride = self.texcoord_stride
    normal_stride = self.normal_stride
    
    if [mat for mat in self.materials if mat]:
      for mat in self.materials:
        mat.realize()
      self.funcs.append(self.render_material)
    else:
      self.funcs.append(self.render_no_material)
    
    if self.faces:
      if self.vertices:
        assert len(self.vertices[0]) == vertex_stride
        linearvertices = []
        for f1,f2,f3 in self.faces:					
          linearvertices += self.vertices[f1]
          linearvertices += self.vertices[f2]
          linearvertices += self.vertices[f3]
        if linearvertices:
          self.vertices_id = upload(array(self.vertex_type,linearvertices).tobytes())
          self.vertices_count = len(linearvertices)//vertex_stride
          self.funcs.append(self.render_vertices)
      else:
        self.funcs.append(self.render_no_vertices)
        
      if self.normals:
        assert len(self.normals[0]) == normal_stride
        linearnormals = []
        assert len(self.normal_faces) == len(self.faces), "%s != %s" % (len(self.normal_faces), len(self.faces))
        for f1,f2,f3 in self.normal_faces:					
          linearnormals += self.normals[f1]
          linearnormals += self.normals[f2]
          linearnormals += self.normals[f3]
        if linearnormals:
          self.normals_id = upload(array('f',linearnormals).tobytes())
          self.funcs.append(self.render_normals)
      else:
        self.funcs.append(self.render_no_normals)
        
      if self.texcoords:
        assert len(self.texcoords[0]) == texcoord_stride
        lineartexcoords = []
        assert len(self.texcoord_faces) == len(self.faces), "%s != %s" % (len(self.texcoord_faces), len(self.faces))
        for f1,f2,f3 in self.texcoord_faces:					
          lineartexcoords += self.texcoords[f1]
          lineartexcoords += self.texcoords[f2]
          lineartexcoords += self.texcoords[f3]
        if lineartexcoords:
          self.texcoords_id = upload(array('f',lineartexcoords).tobytes())
          self.funcs.append(self.render_texcoords)
      else:
        self.funcs.append(self.render_no_texcoords)
      
  def set_material(self,slot,material):
    while len(self.materials) < (slot+1):
      self.materials.append(None)
    self.materials[slot] = material
      
  def get_material(self,slot=0):
    return self.materials[slot]
      
  def wipe_remote(self):
    if self.vertices_id:
      glDeleteBuffersARB([self.vertices_id])
    if self.texcoords_id:
      glDeleteBuffersARB([self.texcoords_id])
    if self.normal_id:
      glDeleteBuffersARB([self.normal_id])
    self.funcs = []
    
  def render_no_material(self):
    pass
    
  def render_material(self,slot=0):
    self.materials[slot].render()
    
  def render_no_vertices(self):
    glDisableClientState(GL_VERTEX_ARRAY)
    
  def render_vertices(self):
    glEnableClientState(GL_VERTEX_ARRAY)
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, self.vertices_id)
    glVertexPointerARB(self.vertex_stride,self.vertex_gltype,0,0)

  def render_no_texcoords(self):
    glDisableClientState(GL_TEXTURE_COORD_ARRAY)
    
  def render_texcoords(self):
    glEnableClientState(GL_TEXTURE_COORD_ARRAY)
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, self.texcoords_id)
    glTexCoordPointerARB(2,GL_FLOAT,0,0)

  def render_no_normals(self):
    glDisableClientState(GL_NORMAL_ARRAY)
    
  def render_normals(self):
    glEnableClientState(GL_NORMAL_ARRAY)
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, self.normals_id)
    glNormalPointerARB(GL_FLOAT, 0, 0)		

  def render(self,time=0.0):
    if self.funcs:
      for func in self.funcs:
        func()
      glDrawArrays(GL_TRIANGLES,0,self.vertices_count)

class Keyframe:
  def __init__(self, value = None):
    self.value = value

def interpolate_boolean(v1, v2, x):
  return v1.value

def interpolate_object(v1, v2, x):
  return v1.value

def interpolate_linear(v1, v2, x):
  return v1.value + (v2.value - v1.value)*x

def interpolate_bezier(v1, v2, x):
  return QBezier(v1.value,v1.value,v2.value,v2.value).get_point(x)

def interpolate_logarithmic(v1, v2, x):
  if (v1.value < 0.0) and (v2.value < 0.0):
    v1, v2 = math.log(-v1.value), math.log(-v2.value)
    return -math.exp(v1 + (v2 - v1)*x)
  elif (v1.value > 0.0) and (v2.value > 0.0):
    v1, v2 = math.log(v1.value), math.log(v2.value)
    return math.exp(v1 + (v2 - v1)*x)
  else:
    assert 0, 'both values must have the same sign and not be zero'

def interpolate_vector_linear(v1, v2, x):
  return tuple(vadd(v1.value,vscale(vdiff(v2.value,v1.value), x)))

def interpolate_vector_bezier(v1, v2, x):
  return tuple(QBezier(v1.value,v1.value,v2.value,v2.value).get_point(x))
  #return tuple(vadd(v1.value,vscale(vdiff(v2.value,v1.value), x)))

def interpolate_vector_logarithmic(v1, v2, x):
  return tuple([interpolate_logarithmic(Keyframe(i1),Keyframe(i2),x) for i1,i2 in zip(v1.value,v2.value)])

class LIValue:
  STYLE_LINEAR = 1
  STYLE_BEZIER = 2
  STYLE_LOG = 3 # logarithmic
  
  def __init__(self):
    self._events = {}
    self.wipe_keyframes()
    self.default_style = self.STYLE_LINEAR
    
  def __setitem__(self, time, value, style = None, **kargs):
    if not style:
      style = self.default_style
    kf = Keyframe()
    kf.value = value
    kf.style = style
    for key,value in kargs.items():
      setattr(kf, key, value)
    kf.interpolate = self.get_interpolation_method(style)
    self._events[time] = kf
    self._mintime = min(self._mintime,time)
    self._maxtime = max(self._maxtime,time)
  
  def get_keys(self, mintime=None, maxtime=None):
    values = []
    for t in self._events.keys():
      if ((mintime == None) or (t >= mintime)) and ((maxtime == None) or (t < maxtime)):
        values.append(t)
    return values
    
  def wipe_keyframes(self):
    if 0.0 in self._events:
      zeroevent = self[0.0]
    else:
      zeroevent = None
    self._events = {}
    self._mintime = 999999.0
    self._maxtime = -999999.0
    if zeroevent:
      self[0.0] = zeroevent

  def get_interpolation_method(self, style):
    return {
      self.STYLE_LINEAR : interpolate_linear,
      self.STYLE_BEZIER : interpolate_bezier,
      self.STYLE_LOG : interpolate_logarithmic,
    }[style]
    
  def __getitem__(self, time):
    if time < self._mintime:
      return self._events[self._mintime].value
    elif time > self._maxtime:
      return self._events[self._maxtime].value
    elif time in self._events:
      return self._events[time].value
    mintime = self._mintime
    maxtime = self._maxtime
    for t in self._events.keys():
      if (t < time) and (t > mintime):
        mintime = t
      if (t > time) and (t < maxtime):
        maxtime = t
    e1 = self._events[mintime]
    e2 = self._events[maxtime]
    pos = (time-mintime) / (maxtime - mintime)
    return e1.interpolate(e1, e2, pos)

class LIBoolean(LIValue):
  def get_interpolation_method(self, style):
    return interpolate_boolean

class LIObject(LIValue):
  def get_interpolation_method(self, style):
    return interpolate_object

class LIVector(LIValue):
  def __init__(self):
    LIValue.__init__(self)
    self.default_style = self.STYLE_BEZIER
    
  def get_interpolation_method(self, style):
    return {
      self.STYLE_LINEAR : interpolate_vector_linear,
      self.STYLE_BEZIER : interpolate_vector_bezier,
      self.STYLE_LOG : interpolate_vector_logarithmic,
    }[style]

class Actor(object):
  def __init__(self):
    self.init()
    
  def init(self):
    self.pivot = (0.0, 0.0, 0.0)
    self._visible = LIBoolean()
    self._visible[0.0] = True
    self._scale = LIVector()
    self._scale.default_style = LIVector.STYLE_LOG
    self._scale[0.0] = (1.0, 1.0, 1.0)
    self._position = LIVector()
    self._position[0.0] = (0.0, 0.0, 0.0)
    self._rotation = LIVector()
    self._rotation[0.0] = (0.0, 0.0, 0.0, 1.0)
    self.matrix = [
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0,
    ]
    
  def wipe_keyframes(self):
    self._visible.wipe_keyframes()
    self._scale.wipe_keyframes()
    self._position.wipe_keyframes()
    self._rotation.wipe_keyframes()
    
  def set_visible(self,visible,time=0.0,*args):
    self._visible.__setitem__(time,visible,*args[1:])
    
  def get_visible(self, time=0.0):
    return self._visible[time]
    
  def rescale_position_keyframes(self, xxx_todo_changeme1, mintime=None, maxtime=None):
    (x,y,z) = xxx_todo_changeme1
    for t in self._position.get_keys(mintime,maxtime):
      px,py,pz = self._position[t]
      self._position[t] = px*x, py*y, pz*z
    
  def rescale_scale_keyframes(self, xxx_todo_changeme2, mintime=None, maxtime=None):
    (x,y,z) = xxx_todo_changeme2
    for t in self._scale.get_keys(mintime,maxtime):
      sx,sy,sz = self._scale[t]
      self._scale[t] = sx*x, sy*y, sz*z

  def set_position(self,position,time=0.0,*args):
    self._position.__setitem__(time,position,*args[1:])

  def get_position(self,time=0.0):
    return self._position[time]

  def set_rotation(self,rotation,time=0.0,*args):
    self._rotation.__setitem__(time,rotation,*args[1:])

  def get_rotation(self,time=0.0):
    return self._rotation[time]

  def set_scale(self,scale,time=0.0,*args):
    self._scale.__setitem__(time,scale,*args[1:])

  def get_scale(self,time=0.0):
    return self._scale[time]
    
  def copy_keyframe(self, srckey, dstkey):
    self.set_position(self.get_position(srckey), dstkey)
    self.set_rotation(self.get_rotation(srckey), dstkey)
    self.set_scale(self.get_scale(srckey), dstkey)
    
  def extend_keyframe(self, t, time):
    nt = t + time
    self.copy_keyframe(t, nt)
    return nt

  def render(self, time = 0.0):
    pass

  visible = property(get_visible, set_visible)
  position = property(get_position, set_position)
  rotation = property(get_rotation, set_rotation)
  scale = property(get_scale, set_scale)

class MeshActor(Actor):
  def __init__(self, mesh = None):
    Actor.__init__(self)
    self.mesh = mesh
    self.transform = self.transform_default
    self.matrix_anim = False
    self.ode_body = None
    
  def set_ode_body(self, body):
    self.ode_body = body
    self.transform = self.transform_ode_body
    
  def reset_ode_body(self):
    self.ode_body.setPosition(self.position)
    self.ode_body.setAngularVel([0,0,0])
    self.ode_body.setLinearVel([0,0,0])
    a,x,y,z = self.rotation
    self.ode_body.setQuaternion(vector.angleaxis_to_quat([(pi/180.0)*a,x,y,z]))
    
  def make_ode_body(self, world, typename = 'box', space = None):
    assert not self.ode_body
    import ode
    lx,ly,lz = self.mesh.get_size()
    sx, sy, sz = self.scale 
    lx *= sx
    ly *= sy
    lz *= sz

    body = ode.Body(world)
    M = ode.Mass()

    geom = None
    if typename == 'box':
      M.setBox(250.0, lx, ly, lz)
      if space:
        geom = ode.GeomBox(space, lengths=(lx,ly,lz))
    else:
      # sphere
      radius = max(lx,ly,lz)*0.5			
      M.setSphere(250.0, radius)
      M.mass = 1.0
      if space:
        # Create a box geom for collision detection
        geom = ode.GeomSphere(space, radius)

    body.setMass(M)
    self.set_ode_body(body)
    self.reset_ode_body()
    
    if geom:
      geom.setBody(body)


  def transform_ode_body(self,time):
    x,y,z = self.ode_body.getPosition()
    R = self.ode_body.getRotation()
    matrix = [R[0], R[3], R[6], 0.,
          R[1], R[4], R[7], 0.,
          R[2], R[5], R[8], 0.,
          x, y, z, 1.0]
    px, py, pz = self.pivot
    glMultMatrixf(matrix)
    glScalef(*self.scale)
    glTranslatef(-px, -py, -pz)
    glMultMatrixf(self.matrix)
    
  def get_matrix(self,time=0.0):
    glMatrixMode(GL_MODELVIEW)
    glPushMatrix()
    glLoadIdentity()
    self.transform(time)
    m = glGetDoublev(GL_MODELVIEW_MATRIX)
    m.swapaxes(0,1)
    glMatrixMode(GL_MODELVIEW)
    glPopMatrix()
    return m
    
  def transform_default(self,time):
    px, py, pz = self.pivot
    glMultMatrixf(self.matrix)
    glTranslatef(*self._position[time])
    glRotatef(*self._rotation[time])
    glScalef(*self._scale[time])
    glTranslatef(-px, -py, -pz)

  def render(self,time=0.0):
    glMatrixMode(GL_MODELVIEW)
    glPushMatrix()
    self.transform(time)
    if self.mesh and self._visible[time]:
      self.mesh.render()
    glMatrixMode(GL_MODELVIEW)
    glPopMatrix()

class CssURL:
  href = ''
  is_local = False

re_digit = re.compile(r'^([0-9.,]+)(px|cm|in|mm|pt|pc)?$')
re_rect = re.compile(r'^([0-9.,]+) ([0-9.,]+) ([0-9.,]+) ([0-9.,]+)$')
re_url = re.compile(r'^url\(([^\)]+)\)$')
re_css = re.compile(r'\s*.?([A-Za-z0-9_]+)\s*\{\s*([^\}]*)\s*\}\s*(.*)')

UNIT_IN = 96.0
UNIT_CM = UNIT_IN / 2.54
UNIT_MM = UNIT_CM / 10.0
UNIT_PT = UNIT_IN / 72.0
UNIT_PC = UNIT_PT * 12.0

def convert_to_pixel(v,unit):
  if unit == 'px':
    return v
  if unit == 'in':
    return v * UNIT_IN
  if unit == 'cm':
    return v * UNIT_CM
  if unit == 'mm':
    return v * UNIT_MM
  if unit == 'pt':
    return v * UNIT_PT
  if unit == 'pc':
    return v * UNIT_PC
  return v

def parse_css_value(value):
  if re_rect.match(value):
    m = re_rect.match(value)
    value = float(m.group(1)),float(m.group(2)),float(m.group(3)),float(m.group(4))
  elif re_digit.match(value):
    m = re_digit.match(value)
    unit = m.group(2)
    value = convert_to_pixel(float(m.group(1)), unit)
  elif value.startswith('#'):
    number = int(value[1:],16)
    if len(value) == 4:
      b = (number & 0xf) / 15.0
      g = ((number >> 4) & 0xf) / 15.0
      r = ((number >> 8) & 0xf) / 15.0
    else:
      b = (number & 0xff) / 255.0
      g = ((number >> 8) & 0xff) / 255.0
      r = ((number >> 16) & 0xff) / 255.0
    value = (r,g,b)
  elif re_url.match(value):				
    m = re_url.match(value)
    value = CssURL()
    value.href = m.group(1)
    if value.href.startswith('#'):
      value.href = value.href[1:]
      value.is_local = True
  elif value == 'none':
    value = None
  return value

class CssStyle:
  re_statement = re.compile(r'^;?([^;]*)(;?.*)')
  re_attrib = re.compile(r'^\s*([a-z0-9\-]+)\s*:\s*([#a-z%A-Z_,.\(\'"\)\s0-9\-]+)\s*$')
  
  def __init__(self):
    self.attributes = {}
    
  def __getitem__(self, name):
    return self.attributes.get(name, None)

  def parse(self, text):		
    def parse_statement(text):
      m = self.re_statement.match(text)
      return m.group(1), m.group(2)		
    def parse_attrib(text):
      m = self.re_attrib.match(text)
      assert m, repr(text)
      key, value = m.group(1), m.group(2)
      value = parse_css_value(value)
      return key, value
    while text:
      statement,text = parse_statement(text)
      if statement:
        name,value = parse_attrib(statement)
        self.attributes[name] = value

def get_shape_length_generator(shape):
  length = 0
  lpt = shape[0]
  for pt in shape:
    length += vlength(vdiff(pt,lpt))
    lpt = pt
    yield length

def get_shape_length(shape):
  return list(get_shape_length_generator(shape))[-1]

def is_shape_closed(shape):
  return vequals(shape[0],shape[-1])

def fix_hard_shape_edges(shape):
  points = []	
  try:
    closepath = is_shape_closed(shape)	
    pcount = len(shape)
    for i in range(pcount):
      if i == 0:
        if closepath:
          v = shape[pcount-2], shape[0], shape[1]
        else:
          v = shape[0], shape[0], shape[1]
      elif i == (pcount - 1):
        if closepath:
          v = shape[i-1], shape[i], shape[1]
        else:
          v = shape[i-1], shape[i], shape[i]
      else:
        v = shape[i-1], shape[i], shape[i+1]
      pa = vdiff(v[1],v[0]) # part 1 of tri
      pb = vdiff(v[2],v[1]) # part 2 of tri
      vla = vlength(pa)
      vlb = vlength(pb)
      if vla and vlb:
        cosang = min(vcosAngle(pa,pb),1.0)
        arcang = math.acos(cosang)
        diffangle = 180.0 - ((180.0/math.pi) * arcang)
        pa = vnormalize(pa)
        pb = vnormalize(pb)
        d = determinant2d(pa,pb)
        if d and abs(diffangle < 30.0):
          pa2 = vscale((pa[1], -pa[0]),0.001)
          pb2 = vscale((pb[1], -pb[0]),0.001)
          points.append(vadd(v[1],pa2))
          points.append(vadd(v[1],pb2))
        else:
          points.append(v[1])
      else:
        points.append(v[1])
  except:
    import traceback
    traceback.print_exc()
  return points

def get_stroke_outline(shape, width):
  closepath = is_shape_closed(shape)
  points = []
  pcount = len(shape)
  for i in range(pcount):
    if i == 0:
      if closepath:
        v = shape[pcount-2], shape[0], shape[1]
      else:
        v = shape[0], shape[0], shape[1]
    elif i == (pcount - 1):
      if closepath:
        v = shape[i-1], shape[i], shape[1]
      else:
        v = shape[i-1], shape[i], shape[i]
    else:
      v = shape[i-1], shape[i], shape[i+1]
    pa = vdiff(v[1],v[0]) # part 1 of tri
    pb = vdiff(v[2],v[1]) # part 2 of tri
    vla = vlength(pa)
    vlb = vlength(pb)
    if vla and vlb:
      pa = vnormalize(pa)
      pb = vnormalize(pb)
      d = determinant2d(pa,pb)
      if d:
        pa2 = vscale((pa[1], -pa[0]),width*0.5)
        pb2 = vscale((pb[1], -pb[0]),width*0.5)
        points.append(intersection(vadd(v[1], pa2),pa,vadd(v[1], pb2),pb))
      else:
        pa2 = vscale((pa[1], -pa[0]),width*0.5)
        points.append(vadd(v[1],pa2))
    elif vla:
      pa = vnormalize(pa)
      pa2 = vscale((pa[1], -pa[0]),width*0.5)
      points.append(vadd(v[1],pa2))
    else:
      pb = vnormalize(pb)
      pb2 = vscale((pb[1], -pb[0]),width*0.5)
      points.append(vadd(v[1],pb2))
  return points

def get_stroke_outlines(shape, width):
  return get_stroke_outline(shape, -width), get_stroke_outline(shape, width)

import OpenGL.GLU.tess

def combineWrapper( self, function ):
  """Wrap a Python function with ctypes-compatible wrapper for combine callback
  
  For a Python combine callback, the signature looks like this:
    def combine(
      GLdouble coords[3], 
      void *vertex_data[4], 
      GLfloat weight[4]
    ):
      return data
  While the C signature looks like this:
    void combine( 
      GLdouble coords[3], 
      void *vertex_data[4], 
      GLfloat weight[4], 
      void **outData 
    )
  """
  if (function is not None) and (not callable( function )):
    raise TypeError( """Require a callable callback, got:  %s"""%(function,))
  def wrap( coords, vertex_data, weight, outData, *args ):
    """The run-time wrapper around the function"""
    coords = self.ptrAsArray( coords, 3, arrays.GLdoubleArray )
    weight = self.ptrAsArray( weight, 4, arrays.GLfloatArray )
    # find the original python objects for vertex data
    vertex_data = [ self.originalObject( vertex_data[i] ) for i in range(4) ]
    args = tuple( [ self.originalObject( x ) for x in args ] )
    try:
      result = function( coords, vertex_data, weight, *args )
    except Exception as err:
      raise err.__class__(
        """Failure during combine callback %r with args( %s,%s,%s,*%s):\n%s"""%(
          function, coords, vertex_data, weight, args, str(err),
        )
      )
    outData.contents.value = self.noteObject(result)
    return None
  return wrap
  

OpenGL.GLU.tess.GLUtesselator.combineWrapper = combineWrapper

def rgb(r,g,b):
  return r/255.0,g/255.0,b/255.0
# use http://www.w3.org/TR/SVG/types.html#ColorKeywords for reference
COLOR_KEYWORDS = {
'aliceblue': rgb(240, 248, 255),
'antiquewhite': rgb(250, 235, 215),
'aqua': rgb( 0, 255, 255),
'aquamarine': rgb(127, 255, 212),
'azure': rgb(240, 255, 255),
'beige': rgb(245, 245, 220),
'bisque': rgb(255, 228, 196),
'black': rgb( 0, 0, 0),
'blanchedalmond': rgb(255, 235, 205),
'blue': rgb( 0, 0, 255),
'blueviolet': rgb(138, 43, 226),
'brown': rgb(165, 42, 42),
'burlywood': rgb(222, 184, 135),
'cadetblue': rgb( 95, 158, 160),
'chartreuse': rgb(127, 255, 0),
'chocolate': rgb(210, 105, 30),
'coral': rgb(255, 127, 80),
'cornflowerblue': rgb(100, 149, 237),
'cornsilk': rgb(255, 248, 220),
'crimson': rgb(220, 20, 60),
'cyan': rgb( 0, 255, 255),
'darkblue': rgb( 0, 0, 139),
'darkcyan': rgb( 0, 139, 139),
'darkgoldenrod': rgb(184, 134, 11),
'darkgray': rgb(169, 169, 169),
'darkgreen': rgb( 0, 100, 0),
'darkgrey': rgb(169, 169, 169),
'darkkhaki': rgb(189, 183, 107),
'darkmagenta': rgb(139, 0, 139),
'darkolivegreen': rgb( 85, 107, 47),
'darkorange': rgb(255, 140, 0),
'darkorchid': rgb(153, 50, 204),
'darkred': rgb(139, 0, 0),
'darksalmon': rgb(233, 150, 122),
'darkseagreen': rgb(143, 188, 143),
'darkslateblue': rgb( 72, 61, 139),
'darkslategray': rgb( 47, 79, 79),
'darkslategrey': rgb( 47, 79, 79),
'darkturquoise': rgb( 0, 206, 209),
'darkviolet': rgb(148, 0, 211),
'deeppink': rgb(255, 20, 147),
'deepskyblue': rgb( 0, 191, 255),
'dimgray': rgb(105, 105, 105),
'dimgrey': rgb(105, 105, 105),
'dodgerblue': rgb( 30, 144, 255),
'firebrick': rgb(178, 34, 34),
'floralwhite': rgb(255, 250, 240),
'forestgreen': rgb( 34, 139, 34),
'fuchsia': rgb(255, 0, 255),
'gainsboro': rgb(220, 220, 220),
'ghostwhite': rgb(248, 248, 255),
'gold': rgb(255, 215, 0),
'goldenrod': rgb(218, 165, 32),
'gray': rgb(128, 128, 128),
'grey': rgb(128, 128, 128),
'green': rgb( 0, 128, 0),
'greenyellow': rgb(173, 255, 47),
'honeydew': rgb(240, 255, 240),
'hotpink': rgb(255, 105, 180),
'indianred': rgb(205, 92, 92),
'indigo': rgb( 75, 0, 130),
'ivory': rgb(255, 255, 240),
'khaki': rgb(240, 230, 140),
'lavender': rgb(230, 230, 250),
'lavenderblush': rgb(255, 240, 245),
'lawngreen': rgb(124, 252, 0),
'lemonchiffon': rgb(255, 250, 205),
'lightblue': rgb(173, 216, 230),
'lightcoral': rgb(240, 128, 128),
'lightcyan': rgb(224, 255, 255),
'lightgoldenrodyellow': rgb(250, 250, 210),
'lightgray': rgb(211, 211, 211),
'lightgreen': rgb(144, 238, 144),
'lightgrey': rgb(211, 211, 211),
'lightpink': rgb(255, 182, 193),
'lightsalmon': rgb(255, 160, 122),
'lightseagreen': rgb( 32, 178, 170),
'lightskyblue': rgb(135, 206, 250),
'lightslategray': rgb(119, 136, 153),
'lightslategrey': rgb(119, 136, 153),
'lightsteelblue': rgb(176, 196, 222),
'lightyellow': rgb(255, 255, 224),
'lime': rgb( 0, 255, 0),
'limegreen': rgb( 50, 205, 50),
'linen': rgb(250, 240, 230),
'magenta': rgb(255, 0, 255),
'maroon': rgb(128, 0, 0),
'mediumaquamarine': rgb(102, 205, 170),
'mediumblue': rgb( 0, 0, 205),
'mediumorchid': rgb(186, 85, 211),
'mediumpurple': rgb(147, 112, 219),
'mediumseagreen': rgb( 60, 179, 113),
'mediumslateblue': rgb(123, 104, 238),
'mediumspringgreen': rgb( 0, 250, 154),
'mediumturquoise': rgb( 72, 209, 204),
'mediumvioletred': rgb(199, 21, 133),
'midnightblue': rgb( 25, 25, 112),
'mintcream': rgb(245, 255, 250),
'mistyrose': rgb(255, 228, 225),
'moccasin': rgb(255, 228, 181),
'navajowhite': rgb(255, 222, 173),
'navy': rgb( 0, 0, 128),
'oldlace': rgb(253, 245, 230),
'olive': rgb(128, 128, 0),
'olivedrab': rgb(107, 142, 35),
'orange': rgb(255, 165, 0),
'orangered': rgb(255, 69, 0),
'orchid': rgb(218, 112, 214),
'palegoldenrod': rgb(238, 232, 170),
'palegreen': rgb(152, 251, 152),
'paleturquoise': rgb(175, 238, 238),
'palevioletred': rgb(219, 112, 147),
'papayawhip': rgb(255, 239, 213),
'peachpuff': rgb(255, 218, 185),
'peru': rgb(205, 133, 63),
'pink': rgb(255, 192, 203),
'plum': rgb(221, 160, 221),
'powderblue': rgb(176, 224, 230),
'purple': rgb(128, 0, 128),
'red': rgb(255, 0, 0),
'rosybrown': rgb(188, 143, 143),
'royalblue': rgb( 65, 105, 225),
'saddlebrown': rgb(139, 69, 19),
'salmon': rgb(250, 128, 114),
'sandybrown': rgb(244, 164, 96),
'seagreen': rgb( 46, 139, 87),
'seashell': rgb(255, 245, 238),
'sienna': rgb(160, 82, 45),
'silver': rgb(192, 192, 192),
'skyblue': rgb(135, 206, 235),
'slateblue': rgb(106, 90, 205),
'slategray': rgb(112, 128, 144),
'slategrey': rgb(112, 128, 144),
'snow': rgb(255, 250, 250),
'springgreen': rgb( 0, 255, 127),
'steelblue': rgb( 70, 130, 180),
'tan': rgb(210, 180, 140),
'teal': rgb( 0, 128, 128),
'thistle': rgb(216, 191, 216),
'tomato': rgb(255, 99, 71),
'turquoise': rgb( 64, 224, 208),
'violet': rgb(238, 130, 238),
'wheat': rgb(245, 222, 179),
'white': rgb(255, 255, 255),
'whitesmoke': rgb(245, 245, 245),
'yellow': rgb(255, 255, 0),
'yellowgreen': rgb(154, 205, 50),
}

class Parser:
  re_matrix = re.compile(r'^matrix\(([-\d.e]+)[\s,]*([-\d.e]+)[\s,]*([-\d.e]+)[\s,]*([-\d.e]+)[\s,]*([-\d.e]+)[\s,]*([-\d.e]+)[\s,]*\)')
  re_translate = re.compile(r'^translate\(([-\d.e]+)[\s,]*([-\d.e]+)[\s,]*\)')
  re_scale = re.compile(r'^scale\(([-\d.e]+)[\s,]*([-\d.e]+)[\s,]*\)')
  re_rotate = re.compile(r'^rotate\(([-\d.e]+)[\s,]*\)')
  re_skewx = re.compile(r'^skewX\(([-\d.e]+)[\s,]*\)')
  re_skewy = re.compile(r'^skewY\(([-\d.e]+)[\s,]*\)')
  
  class SVGPath:
    re_d_tag = re.compile(r'^([MmZzLlHhVvCcSsQqTtAa])\s*(.*)')
    re_d_num = re.compile(r'^(-?[\d]*[.]?[\d]+)(e-?[\d]+)?[\s,]*(.*)')

    def __init__(self,stager,*args,**kargs):
      self.pos = None
      self.shapes = []
      self.shape = []
      self.detail = BEZIER_NCOUNT
      self.strokewidth = 0.0
      self.fillrule = 'nonzero'
      self.strokeuvmap = False
      self.invertuvmap = False
      self.stager = stager
      self.id = ""
      self.msgclass = None
      self.material = None
      
    def get_extents(self):
      return vextents([vextents(shape)[0] for shape in self.shapes] + [vextents(shape)[1] for shape in self.shapes])
      
    def set_fill_rule(self, fillrule):
      self.fillrule = fillrule
      
    def set_stroke_width(self, strokewidth):
      self.strokewidth = strokewidth
      
    def enable_stroke_uvmap(self, enable, invert=False):
      self.strokeuvmap = enable
      self.invertuvmap = invert
      
    def get_mesh_shape(self):
      vmin, vmax = self.get_extents()
      vcenter = vscale(vadd(vmin,vmax),0.5)		
      scale = max(vdiff(vmax,vmin))		
      if not scale:
        return None
      vxscale = 1.0 / scale
      self.stager.builder.begin_shape(self.id,self.msgclass)
      tlatemtx = self.stager.svg_matrix([1.0,0.0,0.0,1.0,vcenter[0],vcenter[1]])
      scalemtx = self.stager.svg_matrix([scale,0.0,0.0,scale,0.0,0.0])
      mtx = self.stager.get_current_matrix()
      self.stager.push_matrix()
      self.stager.identity()
      self.stager.multiply_matrix(mtx)
      self.stager.multiply_matrix(tlatemtx)
      self.stager.multiply_matrix(scalemtx)
      self.stager.builder.shape_matrix3x3(self.stager.get_current_matrix())
      self.stager.builder.shape_matrix4x4(self.stager.matrix3x3_to_4x4(self.stager.get_current_matrix()))
      for shape in self.shapes:
        for index,pos in enumerate(shape):
          vc = vscale(vdiff(pos,vcenter),vxscale)
          self.stager.builder.shape_point(vc[0], vc[1])
      self.stager.pop_matrix()
      self.stager.builder.end_shape()
      
    def simplify_shape(self, s, scale):
      peaks = s[:]
      i = 0
      while i < len(peaks):
        v = peaks[i]
        keep = False
        if (i != 0) and (i != len(peaks)-1):					
          v0 = peaks[i-1]
          v1 = peaks[i+1]
          sa = vdiff(v,v0)
          sb = vdiff(v1,v)
          lena = vlength(sa)
          lenb = vlength(sb)
          if lena*lenb:
            a = abs(vangle(sa,sb))
            if a >= S_ANGLESCALE:
              keep = True
        else:
          keep = True
        if not keep:
          del peaks[i]
          i -= 1
        else:
          i += 1
      return peaks
      
    def get_mesh(self,offset):
      vmin, vmax = self.get_extents()
      vcenter = vscale(vadd(vmin,vmax),0.5)		
      scale = max(vdiff(vmax,vmin))		
      if not scale:
        return None
      if SIMPLIFY:
        for i,shape in enumerate(self.shapes):
          self.shapes[i] = self.simplify_shape(shape, scale)
      vxscale = 1.0 / scale
      mobj = Mesh()
      pathlen = 0.0
      totalpathlen = 0.0
      def begin(typename):
        self.stager.builder.begin_mesh()
        tlatemtx = self.stager.svg_matrix([1.0,0.0,0.0,1.0,vcenter[0],vcenter[1]])
        scalemtx = self.stager.svg_matrix([scale,0.0,0.0,scale,0.0,0.0])
        mtx = self.stager.get_current_matrix()
        self.stager.push_matrix()
        self.stager.identity()
        self.stager.multiply_matrix(mtx)
        self.stager.multiply_matrix(tlatemtx)
        self.stager.multiply_matrix(scalemtx)
        mtx4 = self.stager.matrix3x3_to_4x4(self.stager.get_current_matrix())
        #~ gluTessNormal(tobj, *v)
        self.stager.builder.mesh_matrix3x3(self.stager.get_current_matrix())
        self.stager.builder.mesh_matrix4x4(mtx4)
        assert typename == GL_TRIANGLES, 'typename is %s, not %s' % (typename,GL_TRIANGLES)
      def edge_flag(flag):
        # if true, edge is an outline
        self.stager.builder.edge_flag(flag)
      def vertex(v):
        self.combine_result = None
        assert v
        vc = vscale(vdiff(v[:2],vcenter),vxscale)
        #vc = vscale(v[:2],vxscale)
        if self.strokeuvmap:
          tc = [v[2], v[3]]
        else:
          tc = vscale(vdiff(v[:2],vmin),vxscale)
        self.stager.builder.vertex(vc[0], vc[1], 0.0)
        mobj.vertices.append(vc + [0.0,1.0])
        self.stager.builder.texcoord(tc[0], tc[1])
        if isinstance(self.material.diffuse_texture, Gradient):					
          self.stager.builder.color(*self.material.diffuse_texture.get_sample(tc[0], tc[1]))
        mobj.texcoords.append(tc)
        vcount = len(mobj.vertices)
        if not (vcount % 3):
          idx = [vcount-3, vcount-2, vcount-1]
          mobj.faces.append(idx)
          self.stager.builder.face(*idx)
          mobj.texcoord_faces.append(idx)
      self.combine_result = None
      self.results = []
      def error(enum):
        log("ERROR:",gluErrorString(enum))
        print(self.current_shape)
        print(self.current_pos)
      def combine(coords, vertex_data, weight):
        # make sure you match PyOpenGL's tess.py so that the combine wrapper
        # returns the value like this:
        # outData.contents.value = self.noteObject(result)
        # or we just change the wrapper to our own method below
        return list(coords)
      def end():
        assert (len(mobj.vertices) % 3) == 0, 'ending before finalizing last polygon?!'
        #~ self.stager.pop_matrix()
        #~ self.stager.pop_matrix()				
        #~ self.stager.pop_matrix()
        self.stager.pop_matrix()
        self.stager.builder.end_mesh()
      tobj = gluNewTess()
      gluTessNormal(tobj, 0,0,1)
      gluTessCallback(tobj,GLU_TESS_BEGIN,begin)
      gluTessCallback(tobj,GLU_TESS_EDGE_FLAG,edge_flag)
      gluTessCallback(tobj,GLU_TESS_VERTEX,vertex)
      gluTessCallback(tobj,GLU_TESS_END,end)
      gluTessCallback(tobj,GLU_TESS_COMBINE,combine)
      gluTessCallback(tobj,GLU_TESS_ERROR,error)
      gluTessProperty(tobj,GLU_TESS_BOUNDARY_ONLY,False)
      gluTessProperty(tobj,GLU_TESS_TOLERANCE,1.0)
      if self.fillrule == 'nonzero':
        gluTessProperty(tobj,GLU_TESS_WINDING_RULE,GLU_TESS_WINDING_NONZERO)
      elif self.fillrule == 'evenodd':
        gluTessProperty(tobj,GLU_TESS_WINDING_RULE,GLU_TESS_WINDING_ODD)
      else:
        gluTessProperty(tobj,GLU_TESS_WINDING_RULE,GLU_TESS_WINDING_ODD)
      error_points = []
      gluTessBeginPolygon(tobj, None)
      if self.strokewidth:
        self.strokeuvmap = True
      if self.strokeuvmap:
        totalpathlen = 0.0
        for shape in self.shapes:				
          totalpathlen += get_shape_length(shape)
      vposdelta = 0.0
      if self.strokeuvmap:
        begin(GL_TRIANGLES)
      for shape in self.shapes:
        self.current_shape = shape
        if self.strokewidth:
          shape = fix_hard_shape_edges(shape)
          self.current_shape = shape
          outlines = get_stroke_outlines(shape, self.strokewidth)
          if self.strokeuvmap:						
            vpos = [v/totalpathlen for v in list(get_shape_length_generator(shape))]
            pointcount = len(shape)
            for ipos in range(pointcount-1):
              tv = vposdelta + vpos[ipos]
              tv2 = vposdelta + vpos[ipos+1]
              p1 = outlines[0][ipos] + [tv, 0.0]    # 1:0    2:1
              p2 = vipol(outlines[0][ipos],outlines[1][ipos]) + [tv, 1.0]    #       
              p3 = outlines[1][ipos] + [tv, 0.0]    #       
              p4 = outlines[0][ipos+1] + [tv2, 0.0] # 3:0+1  4:1+1
              p5 = vipol(outlines[0][ipos+1],outlines[1][ipos+1]) + [tv2, 1.0]    #       
              p6 = outlines[1][ipos+1] + [tv2, 0.0] #       
              edge_flag(1)
              for p in [p1,p2,p4,p2,p5,p4,p2,p3,p5,p3,p6,p5]:
                vertex(p)
            vposdelta += max(vpos)
          else:
            gluTessBeginContour(tobj)
            for ipos in range(len(shape)):
              gluTessVertex(tobj,outlines[0][ipos] + [0.0],outlines[0][ipos] + [0.0])
            for ipos in reversed(range(len(shape))):
              gluTessVertex(tobj,outlines[1][ipos] + [0.0],outlines[1][ipos] + [0.0])
            gluTessEndContour(tobj)
        else:
          gluTessBeginContour(tobj)
          for index,pos in enumerate(shape):						
            self.current_pos = index
            error_points.append(pos)
            gluTessVertex(tobj,list(pos) + [0.0],list(pos) + [0.0])
          gluTessEndContour(tobj)
      if self.strokeuvmap:
        end()
      try:
        print(dir(tobj))
        gluTessEndPolygon(tobj)
      except GLUError:
        import traceback
        traceback.print_exc()
      if not mobj.vertices:
        log("error tesselating.")
        for points in error_points:
          self.stager.builder.error_point(*points)
        return None
      return mobj, vcenter, scale

    def add_pos(self):
      if self.pos != None:
        if self.shape:
          v = vdiff(self.shape[-1], self.pos)
          if (v[0] == 0) and (v[1] == 0):
            return
          d = vlength(vdiff(self.shape[-1], self.pos))
          if d < 0.00001: # next point is too close to the last one
            #log('discarding point (%s ~ %s)' % (self.shape[-1],self.pos))
            return
        self.shape.append(self.pos)
      
    def moveto(self,pos,absolute=True):
      if self.shape:
        self.closepath()
      if not absolute:
        self.pos = vadd(self.pos,pos)			
      else:
        self.pos = pos		
      self.add_pos()
        
    def closepath(self, visualclose=False):
      if self.shape:
        if visualclose and (list(self.shape[0]) != list(self.shape[-1])):
          self.pos = self.shape[0]
          self.add_pos()
        self.shapes.append(self.shape)
        self.shape = []
        self.pos = None
        
    def lineto(self,pos,absolute=True):
      if absolute:
        pos2 = pos
      else:
        pos2 = vadd(self.pos,pos)
      self.pos = pos2
      self.add_pos()
      
    def curveto(self,pos1,pos2,pos,absolute=True): # lcp = last control point
      if absolute:
        b = QBezier(self.pos,pos1,pos2,pos)
        length = vdiff(pos,self.pos)
        endpos = pos
        self.lcp = pos2
      else:
        b = QBezier(self.pos,vadd(self.pos,pos1),vadd(self.pos,pos2),vadd(self.pos,pos))
        length = vdiff(vadd(self.pos,pos),self.pos)
        endpos = vadd(self.pos,pos)
        self.lcp = vadd(self.pos,pos2)
      for i in range(1,self.detail):
        self.pos = b.get_point(i/float(self.detail))
        self.add_pos()
      self.pos = endpos
      self.add_pos()
      
    def arcto(self,r,xaxis,arcflag,sweepflag,pos,absolute=True):
      xaxis = (math.pi * (xaxis % 360.0)) / 180.0
      if (pos[0] == self.pos[0]) and (pos[1] == self.pos[1]):
        return
      if (r[0] == 0) or (r[1] == 0):
        self.lineto(pos,absolute)
        return
      r = [abs(r[0]),abs(r[1])]
      if arcflag:
        arcflag = 1
      else:
        arcflag = 0
      if sweepflag:
        sweepflag = 1
      else:
        sweepflag = 0
      pos1 = self.pos
      if absolute:
        pos2 = pos
      else:
        pos2 = vadd(pos1,pos)
      ca = math.cos(xaxis)
      sa = math.sin(xaxis)
      m = array([
        [ca, sa],
        [-sa, ca]
        ])
      pos3 = list(matrixmultiply(m,[(pos1[0] - pos2[0])*0.5, (pos1[1] - pos2[1])*0.5]))
      pos32 = [pos3[0]*pos3[0], pos3[1]*pos3[1]]
      n = (pos32[0]/(r[0]*r[0])) + (pos32[1]/(r[1]*r[1]))
      if n > 1:
        r = [math.sqrt(n)*r[0], math.sqrt(n)*r[1]]
      r2 = [r[0]*r[0], r[1]*r[1]]
      nom = (r2[0]*r2[1]) - (r2[0]*pos32[1]) - (r2[1]*pos32[0])
      denom = (r2[0]*pos32[1] + r2[1]*pos32[0])
      if n > 1:
        nom = 0
      fc = math.sqrt(nom / denom)
      if (arcflag == sweepflag):
        fc *= -1.0
      c2 = [fc * ((r[0]*pos3[1])/r[1]), fc * -((r[1]*pos3[0])/r[0])]
      m2 = array([
        [ca, -sa],
        [sa, ca]
        ])
      c3 = list(matrixmultiply(m2,c2))
      c = vadd(c3, [(pos1[0]+pos2[0])*0.5, (pos1[1]+pos2[1])*0.5])
      p1 = [(pos3[0]-c2[0]) / r[0], (pos3[1]-c2[1]) / r[1]]
      p2 = [(-pos3[0]-c2[0]) / r[0], (-pos3[1]-c2[1]) / r[1]]
      astart = vangle([1,0],p1)
      asweep = vangle(p1, p2) % (math.pi*2)
      if sweepflag and (asweep < 0):
        asweep += math.pi*2
      elif not sweepflag and (asweep > 0):
        asweep -= math.pi*2
      detail = self.detail * 2
      for i in range(1,detail):
        theta = i/float(detail)
        theta = astart + theta * asweep
        w = matrixmultiply(m2,[r[0]*math.cos(theta),r[1]*math.sin(theta)])
        v = vadd(c,w)
        self.pos = v
        self.add_pos()
      self.pos = pos2
      self.add_pos()
    
    def handle_z(self,t,c):
      self.closepath(True)
      
    def handle_a(self,t,c):
      try:
        while True:
          self.arcto(self.stager.svg_vec2([next(c),next(c)]),next(c),next(c),next(c),self.stager.svg_vec2([next(c),next(c)]),{'A':True,'a':False}[t])
      except StopIteration:
        pass
      
    def handle_c(self,t,c):
      try:
        while True:
          self.curveto(self.stager.svg_vec2([next(c),next(c)]),self.stager.svg_vec2([next(c),next(c)]),self.stager.svg_vec2([next(c),next(c)]),{'C':True,'c':False}[t])
      except StopIteration:
        pass

    def handle_s(self,t,c):
      try:
        while True:
          v = vadd(self.pos,vdiff(self.pos,self.lcp))
          if {'S':True,'s':False}[t]:
            self.curveto(v,self.stager.svg_vec2([next(c),next(c)]),self.stager.svg_vec2([next(c),next(c)]))
          else:
            self.curveto(v,vadd(self.pos,self.stager.svg_vec2([next(c),next(c)])),vadd(self.pos,self.stager.svg_vec2([next(c),next(c)])))
      except StopIteration:
        pass

    def handle_l(self,t,c):
      try:
        while True:
          self.lineto(self.stager.svg_vec2([next(c),next(c)]),{'L':True,'l':False}[t])
      except StopIteration:
        pass

    def handle_h(self,t,c):
      x,y = self.pos
      try:
        while True:
          if {'H':True,'h':False}[t]:
            self.lineto(self.stager.svg_vec2([next(c),y]), True)
          else:
            self.lineto(self.stager.svg_vec2([x+next(c),y]), True)
      except StopIteration:
        pass

    def handle_v(self,t,c):
      x,y = self.pos
      try:
        while True:					
          if {'V':True,'v':False}[t]:
            self.lineto(self.stager.svg_vec2([x,next(c)]), True)
          else:
            self.lineto(self.stager.svg_vec2([x,y+next(c)]), True)
      except StopIteration:
        pass

    def handle_m(self,t,c):
      self.moveto(self.stager.svg_vec2([next(c),next(c)]),{'M':True,'m':False}[t])			
      try:
        while True:
          self.lineto(self.stager.svg_vec2([next(c),next(c)]))
      except StopIteration:
        pass
        
    def parse(self, node):
      def read_tag():
        m = self.re_d_tag.match(d)
        assert m, 'cant match: %s' % d
        assert m.group(2) != d
        return m.group(1), m.group(2)
        
      def read_f(d,n=None):
        m = self.re_d_num.match(d)
        if not m and (n != None):
          return n,d
        assert m, "'%s' does not match" % d
        x = 1.0
        if m.group(2):
          x = math.e**float(m.group(2)[1:])
        assert m.group(3) != d
        return float(m.group(1))*x, m.group(3)
        
      def is_f():
        if not d:
          return False
        m = self.re_d_num.match(d)
        if not m:
          return False
        return True
      tagname = node.tagName.lower()
      if tagname == 'line':
        x1 = read_f(node.getAttribute('x1'), 0.0)[0]
        y1 = read_f(node.getAttribute('y1'), 0.0)[0]
        x2 = read_f(node.getAttribute('x2'), 0.0)[0]
        y2 = read_f(node.getAttribute('y2'), 0.0)[0]
        self.moveto([x1,y1])
        self.lineto([x2,y2])
        self.closepath()
      elif tagname == 'polyline':
        d = node.getAttribute('points')
        c = []
        while d:
          while is_f():
            n,d = read_f(d)
            c.append(n)
        self.handle_m('M',iter(c))
        self.closepath()
      elif tagname == 'polygon':
        d = node.getAttribute('points')
        c = []
        while d:
          while is_f():
            n,d = read_f(d)
            c.append(n)
        self.handle_m('M',iter(c))
        self.closepath(True)
      elif tagname == 'rect':
        #<rect class="Border" x="1" y="1" width="998" height="998" />
        rx = 0
        ry = 0
        if node.hasAttribute('rx'):
          rx = read_f(node.getAttribute('rx'), 0.0)[0]
        if node.hasAttribute('ry'):
          ry = read_f(node.getAttribute('ry'), 0.0)[0]
        x = read_f(node.getAttribute('x'), 0.0)[0]
        y = read_f(node.getAttribute('y'), 0.0)[0]
        width = read_f(node.getAttribute('width'), 0.0)[0]
        height = read_f(node.getAttribute('height'), 0.0)[0]
        if rx or ry:
          self.moveto([x+rx,y])
          self.lineto([x+width-rx,y])
          self.arcto([rx,ry],0.0,0,1,[x+width,y+ry])
          self.lineto([x+width,y+height-ry])
          self.arcto([rx,ry],0.0,0,1,[x+width-rx,y+height])
          self.lineto([x+rx,y+height])
          self.arcto([rx,ry],0.0,0,1,[x,y+height-ry])
          self.lineto([x,y+ry])
          self.arcto([rx,ry],0.0,0,1,[x+rx,y])
        else:
          self.moveto([x,y])
          self.lineto([x+width,y])
          self.lineto([x+width,y+height])
          self.lineto([x,y+height])
          self.lineto([x,y])
        self.closepath()
      elif tagname == 'circle':
        cx = read_f(node.getAttribute('cx'), 0.0)[0]
        cy = read_f(node.getAttribute('cy'), 0.0)[0]
        r = read_f(node.getAttribute('r'), 0.0)[0]
        for i in range(0, self.detail*4+1):
          a = i/float(self.detail*4)
          a *= math.pi * 2
          x = cx + r*math.cos(a)
          y = cy + r*math.sin(a)
          if (i == 0):
            self.moveto([x,y])
          else:
            self.lineto([x,y])
        self.closepath()
      elif tagname == 'ellipse':
        #<ellipse fill="#110070" cx="63.755" cy="59.375" rx="59.336" ry="59.375"/>
        cx = read_f(node.getAttribute('cx'), 0.0)[0]
        cy = read_f(node.getAttribute('cy'), 0.0)[0]
        rx = read_f(node.getAttribute('cx'), 0.0)[0]
        ry = read_f(node.getAttribute('cy'), 0.0)[0]
        for i in range(0, self.detail*4+1):
          a = i/float(self.detail*4)
          a *= math.pi * 2
          x = cx + rx*math.cos(a)
          y = cy + ry*math.sin(a)
          if (i == 0):
            self.moveto([x,y])
          else:
            self.lineto([x,y])
        self.closepath()
      elif tagname == 'path':
        d = node.getAttribute('d')
        assert d
    
        while d:
          tag,d =read_tag()
          c = []
          while is_f():
            n,d = read_f(d)
            c.append(n)
          methodname = 'handle_' + tag.lower()
          if hasattr(self,methodname):
            getattr(self,methodname)(tag,iter(c))
          else:
            log('not handling %s' % tag)
              
        self.closepath()
      else:
        assert False, node.tagName
  
  
  def svg_matrix(self, m):	
    a, b, c, d, e, f = m
    m = array([
      [a, c, e],
      [b, d, f],
      [0.0, 0.0, 1.0],
    ])
    return m
    
  def svg_vec2(self, xxx_todo_changeme3):
    (x,y) = xxx_todo_changeme3
    return [x, y]
    
  def matrix3x3_to_4x4(self, m):
    m11, m12, m13 = m[0]
    m21, m22, m23 = m[1]
    m31, m32, m33 = m[2]
    return array([
      [m11, m21, m31, 0.0],
      [m12, m22, m32, 0.0],
      [m13, m23, m33, 0.0],
      [0.0, 0.0, 0.0, 1.0],
    ])

  def __init__(self):
    self.next_light_index = 0
    self.materials = {}
    self.lights = {}
    self.meshes = {}
    self.cameras = {}
    self.mesh_actors = {}
    self.current_camera = None
    self.matrices = [self.svg_matrix([1.0,0.0,0.0,1.0,0.0,0.0])] 
    self.handle__document = self.visit_children
    self.gradients = {}
    self.bgcolor = (1.0,1.0,1.0,1.0)
    self.patterns = {}		
    self.ordered_mesh_actors = []		
    
  def handle_svg(self, node):
    self.builder.begin_document()
    w = parse_css_value(node.getAttribute('width'))
    h = parse_css_value(node.getAttribute('height'))
    viewbox = parse_css_value(node.getAttribute('viewBox'))
    #log('docwidth',w,'docheight',h,'viewBox',viewbox)
    self.builder.document_size(w,h)
    if viewbox and isinstance(viewbox,tuple):
      self.builder.document_viewbox(*viewbox)
      aspect = viewbox[3] / viewbox[2]
      s = min(w/viewbox[2], h/viewbox[3])
      mtx = self.svg_matrix([s,0.0,0.0,s,-viewbox[0],-viewbox[1]])
      self.multiply_matrix(mtx)
    self.camera_scale = [w,-h,1.0]
    self.pivot = w*0.5,h*0.5,0.0
    self.visit_children(node)
    self.builder.end_document()

  def identity(self):
    self.matrices[-1] = self.svg_matrix([1.0,0.0,0.0,1.0,0.0,0.0])
    
  def matrix_changed(self):
    #print len(self.matrices),self.matrices[-1]
    pass
    
  def multiply_matrix(self, m):
    self.matrices[-1] = matrixmultiply(self.matrices[-1], m)
    self.matrix_changed()

  def push_matrix(self):
    self.matrices.append(array(self.matrices[-1]))
    self.matrix_changed()
    
  def pop_matrix(self):
    self.matrices.pop()
    self.matrix_changed()
    
  def get_current_matrix(self):
    return self.matrices[-1]
    
  def handle_style(self, node, *args):
    styletype = node.getAttribute('type')
    if not hasattr(self, 'classes'):
      self.classes = {}
    styledata = node.childNodes[0].data.replace('\n',' ').replace('\r',' ').replace('\t', ' ')
    while styledata:
      m = re_css.match(styledata)
      name,style,styledata = m.group(1), m.group(2), m.group(3)
      self.classes[name] = style.strip()
      
  def parse_style(self, node, path, dostroke=False, dofill=False):
    self.builder.begin_material()
    attribs = CssStyle()
    id = node.getAttribute('id')
    if node.hasAttribute('class'):
      for classname in node.getAttribute('class').split(' '):
        attribs.parse(self.classes[classname])
    if node.hasAttribute('style'):
      attribs.parse(node.getAttribute('style'))
    if node.hasAttribute('fill'):
      attribs.parse('fill:%s' % node.getAttribute('fill'))
    if node.hasAttribute('stroke'):
      attribs.parse('stroke:%s' % node.getAttribute('stroke'))
    if node.hasAttribute('stroke-width'):
      attribs.parse('stroke-width:%s' % node.getAttribute('stroke-width'))
    mat = Material()
    mat.alpha = 1.0
    if dofill and 'fill-opacity' in attribs.attributes:
      mat.alpha *= attribs['fill-opacity']
    if dostroke and 'stroke-opacity' in attribs.attributes:
      mat.alpha *= attribs['stroke-opacity']
    if 'opacity' in attribs.attributes:
      mat.alpha *= attribs['opacity']
    def apply_fill(mat, value):
      if type(value) == tuple:
        self.builder.material_color(*value)
        self.builder.material_alpha(mat.alpha)
        mat.diffuse_color = value
      elif type(value) in (str,str):
        value = COLOR_KEYWORDS[value]
        self.builder.material_color(*value)
        self.builder.material_alpha(mat.alpha)
        mat.diffuse_color = value
      elif isinstance(value,CssURL):
        href = value.href				
        if href in self.gradients:
          mat.diffuse_color = (1.0,1.0,1.0)
          vmin, vmax = path.get_extents()
          vdelta = vdiff(vmax,vmin)
          vxscale = 1.0/max(vdelta)
          newgrad = Gradient()
          newgrad.inherit(self.gradients[href])
          self.builder.material_color(*list(newgrad.stops.values())[0][:3])
          pos1, pos2 = newgrad.get_position(True)
          newgrad.set_position(vscale(vdiff(pos1,vmin),vxscale), vscale(vdiff(pos2,vmin),vxscale))
          newgrad.reset_matrix()
          #newgrad.get_image(size=(64,64)).show()
          mat.diffuse_texture = newgrad
        elif href in self.patterns:
          pattern = self.patterns[href]
          mat.diffuse_color = (1.0,1.0,1.0)
          #mat.diffuse_texture = texture.Texture(path=path,invert=True)
          mat.diffuse_texture = pattern
          #mat.texture_matrix = Numeric.matrixmultiply(self.matrix3x3_to_4x4(self.parse_matrix(pattern.getAttribute('patternTransform'))), Numeric.array([[1.0,0.0,0.0,0.0],[0.0,1.0,0.0,0.0],[0.0,0.0,-1.0,0.0],[0.0,0.0,0.0,1.0]]))
          
        else:
          log('unknown href:',href)		
    retmat = None
    if attribs['stroke'] == (1.0,0.0,1.0):
      path.msgclass = "root"
    elif attribs['stroke'] == (0.0,1.0,0.0):
      path.msgclass = "anchor"
    elif attribs['stroke'] == (1.0,0.0,0.0):
      path.msgclass = "route"
    if node.hasAttribute('msgclass') and node.getAttribute('msgclass'):
      path.msgclass = node.getAttribute('msgclass')
    if not path.msgclass:
      if not (attribs['fill'] or attribs['stroke']):
        attribs.parse('fill:black;')
      if (attribs['fill'] != None):
        if dofill:
          if attribs['fill-rule'] != None:
            path.set_fill_rule(attribs['fill-rule'])
          apply_fill(mat, attribs['fill'])
          retmat = mat
      if attribs['stroke'] != None:
        if dostroke:
          if 'stroke-width' in attribs.attributes:
            path.set_stroke_width(attribs['stroke-width'])
          else:
            path.set_stroke_width(1.0)
          apply_fill(mat, attribs['stroke'])
          if node.getAttribute('strokeuvmap') == 'true':
            path.enable_stroke_uvmap(True)
          elif node.getAttribute('strokeuvmap') == 'invert':
            path.enable_stroke_uvmap(True,True)
          retmat = mat
      path.material = mat
        
    self.materials[id] = mat
    self.builder.end_material()
    return retmat
    
  def handle_stop(self, node, gradient):
    style = CssStyle()
    style.parse(node.getAttribute('style'))
    offset = float(node.getAttribute('offset'))
    color = style['stop-color']
    if 'stop-opacity' in style.attributes:
      opacity = style['stop-opacity']
    elif node.getAttribute('stop-opacity'):
      opacity = float(node.getAttribute('stop-opacity'))
    else:
      opacity = 1.0
    gradient.add_stop(offset, list(color) + [opacity])
  
  def parse_matrix(self, transform):
    m = self.re_matrix.match(transform)
    if m:
      return self.svg_matrix([float(m.group(i)) for i in range(1,7)])
    m = self.re_translate.match(transform)		
    if m:
      return self.svg_matrix([1.0,0.0,0.0,1.0] + [float(m.group(i)) for i in range(1,3)])
    m = self.re_scale.match(transform)
    if m:			
      return self.svg_matrix([float(m.group(1)),0.0,0.0,float(m.group(2)),0.0,0.0])		
    m = self.re_skewx.match(transform)
    if m:			
      a = math.tan(float(m.group(1)))
      return self.svg_matrix([1.0,0.0,a,1.0,0.0,0.0])
    m = self.re_skewy.match(transform)
    if m:			
      a = math.tan(float(m.group(1)))
      return self.svg_matrix([1.0,a,0.0,1.0,0.0,0.0])		
    m = self.re_rotate.match(transform)
    if m:
      # a c e
      # b d f
      a = math.cos(float(m.group(1)))
      b = math.sin(float(m.group(1)))
      c = -math.sin(float(m.group(1)))
      d = math.cos(float(m.group(1)))
      return self.svg_matrix([a,c,0.0,b,d,0.0])		
    assert m, "'%s' is not a known transform" % repr(transform)
    
  def handle_radialGradient(self, node, *args):
    id = node.getAttribute('id')
    grad = Gradient()
    self.gradients[id] = grad
    xlink_href = node.getAttribute('xlink:href')[1:]
    if not xlink_href:
      self.visit_children(node, grad)
    else:
      grad.inherit(self.gradients[xlink_href])
    grad.radial = True
    transform = node.getAttribute('gradientTransform')
    if transform:
      grad.matrix = self.parse_matrix(transform)
    position = [float(node.getAttribute(a)) for a in ('cx','cy','r') if node.getAttribute(a)]
    if position:
      cx,cy,r = position
      grad.set_position(self.svg_vec2((cx,cy)),self.svg_vec2((cx+r,cy+r)))
    
  def handle_linearGradient(self, node, *args):
    id = node.getAttribute('id')
    grad = Gradient()
    self.gradients[id] = grad
    xlink_href = node.getAttribute('xlink:href')[1:]
    if not xlink_href:
      self.visit_children(node, grad)
    else:
      grad.inherit(self.gradients[xlink_href])
    grad.radial = False
    transform = node.getAttribute('gradientTransform')
    if transform:
      grad.matrix = self.parse_matrix(transform)
    position = [float(node.getAttribute(a)) for a in ('x1','y1','x2','y2') if node.getAttribute(a)]
    if position:
      x1,y1,x2,y2 = position
      grad.set_position(self.svg_vec2((x1,y1)),self.svg_vec2((x2,y2)))

  def handle_image(self, node, *args):
    #pattern.image = node.getAttribute('xlink:href')
    pass

  def handle_pattern(self, node, defs):
    id = node.getAttribute('id')
    pattern = texture.Pattern()
    self.patterns[id] = pattern
    xlink_href = node.getAttribute('xlink:href')[1:]
    if not xlink_href:
      self.visit_children(node, defs, pattern)
    else:
      pattern.inherit(self.patterns[xlink_href])
    #~ transform = node.getAttribute('patternTransform')
    #~ if transform:
      #~ pattern.matrix = self.parse_matrix(transform)
    
  def handle_defs(self, node, *args):
    self.visit_children(node, node)
    
  def handle_polyline(self, node, *args):
    self.handle_path(node)

  def handle_polygon(self, node, *args):
    self.handle_path(node)
    
  def handle_line(self, node, *args):
    self.handle_path(node)

  def handle_rect(self, node, *args):
    self.handle_path(node)

  def handle_ellipse(self, node, *args):
    self.handle_path(node)

  def handle_circle(self, node, *args):
    self.handle_path(node)

  def handle_path(self, node, *args):
    self.builder.begin_object()
    id = node.getAttribute('id')
    #log('loading path "%s"' % id)
    style = node.getAttribute('style')
    
    strokepath = self.SVGPath(self)
    fillpath = self.SVGPath(self)
    
    transform = node.getAttribute('transform')
    if transform:
      self.push_matrix()			
      self.multiply_matrix(self.parse_matrix(transform))
    if node.getAttribute('detail'):
      fillpath.detail = int(node.getAttribute('detail'))
    fillpath.parse(node)
    fillmat = self.parse_style(node, fillpath, dofill=True)
    if fillmat:
      fillmesh = fillpath.get_mesh((0,0)) # unwrap arguments
    if node.getAttribute('detail'):
      strokepath.detail = int(node.getAttribute('detail'))
    strokepath.parse(node)
    strokemat = self.parse_style(node, strokepath, dostroke=True)
    if strokemat:
      strokemesh = strokepath.get_mesh((0,0)) # unwrap arguments
    if fillpath.msgclass:
      fillpath.id = node.getAttribute('id')
      fillpath.get_mesh_shape()
    if transform:
      self.pop_matrix()
    self.builder.end_object()
    
  def handle_g(self, node, *args):
    name = node.getAttributeNS(INKSCAPE_NS,"label") or node.getAttribute("id")
    self.builder.begin_group(name)
    transform = node.getAttribute('transform')
    if transform:
      self.push_matrix()
      self.multiply_matrix(self.parse_matrix(transform))
    for child in node.childNodes:
      self.visit(child, *args)
    if transform:
      self.pop_matrix()
    self.builder.end_group()

  def visit_children(self, node, *args):
    for child in node.childNodes:
      self.visit(child, *args)
    
  def handle_sodipodi_namedview(self, node):
    pagecolor = parse_css_value(node.getAttribute('pagecolor'))
    opacity = parse_css_value(node.getAttribute('inkscape:pageopacity'))
    self.bgcolor = pagecolor + (1.0,)
    
  def visit(self, node, *args):
    if node.nodeType in (node.ELEMENT_NODE,node.TEXT_NODE,node.DOCUMENT_NODE):
      methodname = 'handle_' + node.nodeName.replace('#','_').replace(':','_')
      if hasattr(self, methodname):
        getattr(self, methodname)(node, *args)
      else:
        pass

  def parse(self, svgpath, builder):
    self.builder = builder
    self.visit(xmlparse(svgpath))

class Builder:
  # the mesh building interface. implement all callbacks.
  def begin_document(self):
    print("begin_document")
    
  def document_size(self, w, h):
    print("document_size",w,h)
    
  def document_viewbox(self, x, y, w, h):
    print("document_viewbox",x,y,w,h)
    
  def mesh_matrix3x3(self, m):
    print("mesh_matrix3x3",m)

  def mesh_matrix4x4(self, m):
    print("mesh_matrix4x4",m)

  def begin_materials(self):
    print("begin_materials")

  def begin_material(self):
    print("begin_material")
    
  def material_gradient(self):
    print("material_gradient")
    
  def begin_object(self):
    print("begin_object")
    
  def end_object(self):
    print("end_object")
    
  def material_color(self, r, g, b):
    print("material_color",r,g,b)
    
  def material_alpha(self, a):
    print("material_alpha")
    
  def end_material(self):
    print("end_material")

  def end_materials(self):
    print("end_materials")
    
  def begin_group(self, g):
    print("begin_group",g)
    
  def end_group(self):
    print("end_group")
    
  def begin_shape(self,msgclass):
    print("begin_shape")
    
  def shape_point(self,x,y):
    print("shape_point_point",x,y)

  def shape_matrix3x3(self, m):
    print("shape_matrix3x3",m)

  def shape_matrix4x4(self, m):
    print("shape_matrix4x4",m)

  def end_shape(self):
    print("end_shape")
    
  def begin_mesh(self):
    print("begin_mesh")
    
  def mesh_material_ref(self, id):
    print("mesh_material_ref",id)
    
  def vertex(self,x,y,z):
    print("vertex",x,y,z)
    
  def color(self, r, g, b, a):
    print("color",r,g,b,a)
    
  def edge_flag(self,flag):
    print("edge_flag",flag)
    
  def error_point(self, x,y):
    print("error_point",x,y)
  
  def texcoord(self,u,v):
    print("texcoord",u,v)
    
  def face(self,i1,i2,i3):
    print("face",i1,i2,i3)

  def end_mesh(self):
    print("end_mesh")
    
  def end_document(self):
    print("end_document")

if __name__ == '__main__':
  builder = Builder()
  parser = Parser()
  parser.parse('platform.svg', builder)
