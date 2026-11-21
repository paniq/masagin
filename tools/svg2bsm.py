#svg 2 billstedt mesh converter

import math
import svgmesh
import sys
import math
from numpy import dot as matrixmultiply
from tinymarshal import *
from svgmesh import cross,vdiff

def vextents(vl):
	return [min([v[n] for v in vl]) for n in range(len(vl[0]))], [max([v[n] for v in vl]) for n in range(len(vl[0]))]
 
MSGCLASS = {
	'anchor' : 0,
	'root' : 1,
	'route' : 2,
}

class Matrix4x4:
	_tinymarshal_members_ = [
		('i', vec4),
		('j', vec4),
		('k', vec4),
		('l', vec4),
	] 

class BSMShape:
	_tinymarshal_members_ = [
		('id', str),
		('msgclass', int),
		('points', vec2list),
	]
	 
	def __init__(self):
		self.id = ""
		self.msgclass = 0
		self.points = [] 

class BSMMesh:
	_tinymarshal_members_ = [
		('color', vec4),
		('vertices', vec3list),
		#~ ('normals', short2list),
		('texcoords', ushort2list),
		('colors', uintlist),
		('faces', short3list),
	]
	
	def flip_faces(self):
		for i,(i1,i2,i3) in enumerate(self.faces):
			d1 = vdiff(self.vertices[i2][:3],self.vertices[i1][:3])
			d2 = vdiff(self.vertices[i3][:3],self.vertices[i1][:3])
			if cross(d2,d1)[1] > 0:
				self.faces[i] = [i2,i1,i3]
	
	def __init__(self):
		self.matrix = []
		self.color = (0,0,0,1)
		self.vertices = []
		#~ self.normals = []
		self.faces = []
		self.texcoords = []
		self.eflags = []
		self.colors = []

class BSMGroup:
	_tinymarshal_members_ = [
		('id', str),
		('meshes', intlist),
		('shapes', intlist),
	]
	def __init__(self):
		self.id = ""
		self.meshes = []
		self.shapes = []

def norm(x,y):
	if not x and x == y:
		return 0.0,0.0
	ln = 1.0 / ((x*x + y*y)**0.5)
	return x*ln,y*ln

def addnorm(u,v):
	if u == None:
		u = [0.0,0.0]
	if v == None:
		v = [0.0,0.0]
	nx = u[0]+v[0]
	ny = u[1]+v[1]
	return norm(nx,ny)

class BSMBuilder:
	_tinymarshal_members_ = [
		('size', vec2),
		('viewbox', vec4),
		('meshes', BSMMesh, BSMMesh, dict(islist=True)),
		('shapes', BSMShape, BSMShape, dict(islist=True)),
		('groups', BSMGroup, BSMGroup, dict(islist=True)),
	]
	
	def __init__(self, filename):
		self.filename = filename
		self.size = (0,0)
		self.viewbox = (0,0,0,0)
		self.meshes = []
		self.mesh = None
		self.verticecount = 0
		self.facecount = 0
		self.pointcount = 0
		self.shapes = []
		self.eflags = []
		self.shape = None
		self.groups = []
		self.groupstack = []
		
	def document_size(self, w, h):
		self.size = (w,h)

	def document_viewbox(self, x, y, w, h):
		self.viewbox = (x,y,w,h)

	def begin_document(self):
		pass
		
	def end_document(self):
		print("merging vertices and normalizing scale...")
		extents = []
		for mesh in self.meshes:
			extents += vextents(mesh.vertices)
		#~ for shape in self.shapes:
			#~ vmin,vmax = vextents(shape.points)
			#~ vmin = [vmin[0], 0.0, vmin[1], 1.0]
			#~ vmax = [vmax[0], 0.0, vmax[1], 1.0]
			#~ extents += [vmin, vmax]
		vmin, vmax = vextents(extents)
		dx = vmax[0] - vmin[0]
		dy = 0.0 #vmax[1] - vmin[1]
		dz = vmax[2] - vmin[2]
		t = 1.0 / max(dx,dz)
		s = 2.0 / max(dx,dz)
		fx = 32767.0
		for ind,mesh in enumerate(self.meshes):
			print("working on mesh #%i of %i, %i vertices, %i faces" % (ind+1,len(self.meshes), len(mesh.vertices), len(mesh.faces)))
			vertices = []
			texcoords = []
			colors = []
			#~ normals = []
			backrefs = {}
			for i,w in enumerate(mesh.vertices):
				if (i > 0) and ((i % 10000) == 0):
					print("%i vertices done" % i)
				mesh.vertices[i] = [(w[0]-vmin[0]-dx*0.5)*t+0.5, (w[2]-vmin[2]-dz*0.5)*t+0.5, w[1]]
				v = mesh.vertices[i]
				if v not in vertices:
					vertices.append(v)
					texcoords.append(mesh.texcoords[i])
					if mesh.colors:
						colors.append(mesh.colors[i])
					#~ normals.append(mesh.normals[i])
					vi = len(vertices)-1
				else:					
					vi = vertices.index(v)
					if mesh.texcoords[i] != texcoords[vi]:
						vertices.append(v)
						texcoords.append(mesh.texcoords[i])
						if mesh.colors:
							colors.append(mesh.colors[i])
						#~ normals.append(mesh.normals[i])
						vi = len(vertices)-1
				backrefs[i] = vi
			for j in range(len(mesh.faces)):
				i1,i2,i3 = mesh.faces[j]
				mesh.faces[j] = (backrefs[i1],backrefs[i2],backrefs[i3])
			for i1,i2,i3 in mesh.faces:
				assert (i1 >= 0) and (i1 < len(vertices)), str(i1)
				assert (i2 >= 0) and (i2 < len(vertices)), str(i1)
				assert (i3 >= 0) and (i3 < len(vertices)), str(i1)
			oldvertices = mesh.vertices
			mesh.vertices = vertices
			mesh.texcoords = texcoords
			mesh.colors = colors
			#~ mesh.normals = [[int(x*32767.0+0.5),int(y*32767.0+0.5)] for (x,y) in normals]
			self.verticecount += len(mesh.vertices)
			self.facecount += len(mesh.faces)
			print("adding mesh with %i (%i) vertices, %i faces" % (len(mesh.vertices), len(oldvertices), len(mesh.faces)))
		print("total %i meshes = %i vertices, %i faces" % (len(self.meshes), self.verticecount, self.facecount))
		for shape in self.shapes:
			for i,v in enumerate(shape.points):
				vnew = [(v[0]-vmin[0]-dx*0.5)*s, (v[1]-vmin[2]-dz*0.5)*s]
				shape.points[i] = vnew
			print("adding shape \"%s\" (msgclass=%i) with %i points" % (shape.id, shape.msgclass,len(shape.points)))
			self.pointcount += len(shape.points)
		print("total %i shapes = %i points" % (len(self.shapes), self.pointcount))
		for group in self.groups:
			print("adding group \"%s\" with %i meshes, %i shapes" % (group.id,len(group.meshes),len(group.shapes)))
		print("total %i groups" % (len(self.groups)))
		self.file = open(self.filename, 'wb')
		s = pack(self)
		print("writing %i bytes" % (len(s)))
		self.file.write(s)
		self.file.close()
		
	def begin_material(self):
		self._color = (0,0,0,1)
		
	def material_color(self, r,g,b):
		self._color = r,g,b,1
		
	def material_alpha(self, a):
		r,g,b,w = self._color
		self._color = r,g,b,a
		
	def end_material(self):
		pass
		
	def error_point(self, x,y):
		pass
		
	def begin_mesh(self):
		self.mesh = BSMMesh()
		
	def mesh_matrix3x3(self, m):
		pass

	def mesh_matrix4x4(self, m):
		self.mtx = m
		mtx = Matrix4x4()
		mtx.i = m[0]
		mtx.j = m[1]
		mtx.k = m[2]
		mtx.l = m[3]
		mtx.l[0] = mtx.k[0]
		mtx.l[1] = mtx.k[1]
		self.mesh.matrix = mtx

	def vertex(self,x,y,z):
		x,y,z,w = matrixmultiply([x,y,z,1.0], self.mtx)
		self.mesh.vertices.append([x,0.0,-y,1.0])
		self.mesh.eflags.append(self.eflag)

	def edge_flag(self,flag):
		self.eflag = int(flag)

	def texcoord(self,u,v):
		self.mesh.texcoords.append([int(u*65535.0) % 65536,int(v*65535.0) % 65536])
		
	def color(self, r, g, b, a):
		c = int(b*255) | (int(g*255)<<8) | (int(r*255)<<16) | (int(a*255)<<24)
		self.mesh.colors.append(c)
		
	def begin_shape(self,id,msgclass):
		self.shape = BSMShape()
		self.shape.id = str(id)
		self.shape.msgclass = MSGCLASS[msgclass]
		
	def shape_point(self,x,y):
		ox,oy = x,y
		x,y,z,w = matrixmultiply([x,y,0,1], self.mtx)
		self.shape.points.append([x,-y])

	def shape_matrix3x3(self, m):
		#self.mtx = m
		pass

	def shape_matrix4x4(self, m):
		self.mtx = m
		self.mtx[3][0] = self.mtx[2][0]
		self.mtx[3][1] = self.mtx[2][1]

	def end_shape(self):
		self.shapes.append(self.shape)
		for group in self.groupstack:
			group.shapes.append(self.shapes.index(self.shape))
		self.shape = None
		
	def begin_object(self):
		pass
		
	def end_object(self):
		pass

	def begin_group(self, g):
		group = BSMGroup()
		group.id = str(g)
		self.groups.append(group)
		self.groupstack.append(group)
		
	def end_group(self):
		self.groupstack.pop()

	def face(self,i1,i2,i3):
		self.mesh.faces.append((i1,i2,i3))

	def end_mesh(self):
		self.mesh.color = self._color
		#~ self.mesh.normals = [None]*len(self.mesh.vertices)
		self.mesh.flip_faces()
		edges = {}
		for f in self.mesh.faces:
			for i in range(3):
				r = i,(i+1) % 3
				vi = [f[x] for x in r]
				v = [tuple(self.mesh.vertices[vi[x]]) for x in range(2)]
				if (v[1],v[0]) in edges:
					edges[(v[1],v[0])].append(vi)
				elif (v[0],v[1]) in edges:
					edges[(v[0],v[1])].append(vi)
				else:
					edges[(v[1],v[0])] = [vi]
		#~ for v,lvi in edges.iteritems():
			#~ if len(lvi) == 1:
				#~ v1 = v[0]
				#~ v2 = v[1]
				#~ nx = v2[0] - v1[0]
				#~ ny = v2[2] - v1[2]
				#~ n = norm(ny,-nx)
				#~ for j,w in enumerate(self.mesh.vertices):
					#~ if tuple(w) == v[0]:
						#~ self.mesh.normals[j] = addnorm(self.mesh.normals[j], n)
					#~ if tuple(w) == v[1]:
						#~ self.mesh.normals[j] = addnorm(self.mesh.normals[j], n)
		#~ self.mesh.normals = [n or [0.0,0.0] for n in self.mesh.normals]
		self.meshes.append(self.mesh)
		for group in self.groupstack:
			group.meshes.append(self.meshes.index(self.mesh))
		self.mesh = None		

if __name__ == '__main__':
	#~ try:
		#~ import psyco
		#~ psyco.full()
	#~ except:
		#~ print >> sys.stderr, "psyco not installed. install psyco to speed up compilation (http://psyco.sf.net)"
	if not len(sys.argv) == 3:
		print("usage: svg2bsm <source.svg> <target.bsm>")
		raise SystemExit
	srcname = sys.argv[1]
	dstname = sys.argv[2]
	builder = BSMBuilder(dstname)
	parser = svgmesh.Parser()
	parser.parse(srcname, builder)
