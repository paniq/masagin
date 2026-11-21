# masagin compiler

import types
import sys,os,re,math
import random
import copy
from tinymarshal import *

class CompileError(Exception):
	pass

class Token:
	def __init__(self, name, exp, vgroup = 1, rgroup = 2, **kargs):
		self.name = name
		self.rname = kargs.get('rname', self.name)
		self.exp = re.compile('^[ ]*(%s)[ ]*(.*)$' % exp)
		self.vgroup = vgroup
		self.rgroup = rgroup
		self.replaces = kargs.get('replaces', [])

def match_scan_try_funcs(postfix):		
	match = lambda self: self.match(getattr(self, 'tok_'+postfix))
	scan = lambda self: self.scan(getattr(self, 'tok_'+postfix))
	_try = lambda self: self._try(getattr(self, 'tok_'+postfix))
	return match, scan, _try

class Compiler:	
	string_replaces = [
		('\\"', '"'),
		("\\'", "'"),
		("\\t", "\t"),
		("\\n", "\n"),
		("\\r", "\r"),
	]
	for i in range(256):
		string_replaces.append(('\\x%02x' % i, chr(i)))
	tok_symbol = Token('symbol', r'[A-Za-z_][A-Za-z0-9_]*')
	tok_colon = Token('colon', r':')
	tok_bo = Token('bo', r'\(')
	tok_bc = Token('bc', r'\)')
	tok_sbo = Token('sbo', r'\[')
	tok_sbc = Token('sbc', r'\]')
	tok_eq = Token('eq', r'==')
	tok_equal = Token('equal', r'(=)([^=].*)', 2, 3)
	tok_ne = Token('ne', r'!=')
	tok_excl = Token('excl', r'!')
	tok_gt = Token('gt', r'(>)([^=].*)', 2, 3)
	tok_lt = Token('lt', r'(<)([^=].*)', 2, 3)
	tok_ge = Token('ge', r'>=')
	tok_le = Token('le', r'<=')
	tok_at = Token('at', r'\@')
	tok_asterisk = Token('asterisk', r'\*')
	tok_add = Token('add', r'(\+)([^\+].*)', 2, 3)
	tok_sub = Token('sub', r'(\-)([^\-].*)', 2, 3)
	tok_mul = Token('mul', r'(\*)([^\*].*)', 2, 3)
	tok_div = Token('div', r'(\/)([^\/].*)', 2, 3)
	tok_float = Token('float', r'(-?[0-9]*[.]?[0-9]+)([A-Za-z_0-9\%\xb0]+)?', (2,3), 4)
	tok_to = Token('to', r'->')
	tok_dot = Token('dot', r'\.')
	tok_comma = Token('comma', r',')
	tok_plus = Token('plus', r'\+')
	tok_minus = Token('minus', r'\-')
	tok_string1 = Token('string1', r'["]((\\"|[^"])*)["]', 2, 4, replaces=string_replaces)
	tok_string2 = Token('string2', r"[']((\\'|[^'])*)[']", 2, 4, replaces=string_replaces)
	
	tokens = [
		tok_colon,
		tok_string1, tok_string2,
		tok_float, tok_asterisk,
		tok_symbol, tok_excl, tok_dot,
		tok_bo, tok_bc, tok_sbo, tok_sbc, tok_to, tok_comma,
		tok_eq, tok_ne, tok_gt, tok_lt, tok_ge, tok_le,
		tok_add, tok_sub, tok_mul, tok_div, tok_at, tok_plus, tok_minus, tok_equal
	]

	
	def __init__(self):
		self.context = dict(file='',lineno=0,line='')
		self.level = 0
		self.line = ''		
		self.pos = 0
		self.consts = {}
		for tok in self.tokens:
			m,s,t = match_scan_try_funcs(tok.name)
			setattr(self, 'match_'+tok.name, types.MethodType(m,self))
			setattr(self, 'scan_'+tok.name, types.MethodType(s,self))
			setattr(self, 'try_'+tok.name, types.MethodType(t,self))

	def warning(self, msg, context=None):
		if not context:
			context = self.context
		traceback = self.format_context(context)
		print(traceback, file=sys.stderr)
		print("Warning: %s" % msg, file=sys.stderr)
		
	def format_context(self, context):
		return '  File "%s", line %i\n    %s' % (context['file'], context['lineno'], context['line'])

	def error(self, msg, context=None):
		if not context:
			context = self.context
		traceback = self.format_context(context)
		print(traceback, file=sys.stderr)
		print("Compile Error: %s" % msg, file=sys.stderr)
		raise CompileError
		
	def match(self,token):
		m = token.exp.match(self.line)
		if m:
			line = m.group(token.rgroup)
			if type(token.vgroup) in (list, tuple):
				value = [m.group(v) for v in token.vgroup]
			else:
				value = m.group(token.vgroup)
				for before,after in token.replaces:
					value = value.replace(before,after)
			return value, line
		return None
		
	def match_any(self):
		for tok in self.tokens:
			m = self.match(tok)
			if m:
				return tok, m
		return None
			
	def _try(self,token):
		m = self.match(token)
		if m:
			value, self.line = m
			return value
			
	def scan(self,token):
		value = self._try(token)
		if not value:
			self.error('%s expected at %r.' % (token.rname, self.line))
		return value
		
	def match_name(self):
		m = self.match_symbol()
		if m and (m[0] in self.consts):
			return None
		return m
		
	def try_name(self):
		m = self.match_symbol()
		if m and (m[0] in self.consts):
			return None
		value = self.try_symbol()
		if value:
			value = self.aliases.get(value, value)
		return value
		
	def scan_name(self):
		m = self.match_symbol()
		if m and (m[0] in self.consts):
			self.error('symbol expected, but got constant.')
		value = self.scan_symbol()
		if value:
			value = self.aliases.get(value, value)
		return value
		
	def parse(self):
		try:
			while next(self.lines):
				while self.line:
					tok,(value,line) = self.match_any()
					if not tok:
						self.error('syntax error.')
					else:
						value = self.scan(tok)
						print(self.level,tok.name,repr(value))
		except StopIteration:
			pass
			
	def scan_keyword(self, kw):
		v = self.scan_name()
		if not v:
			self.error('keyword %s expected.' % kw)
		if v != kw:
			self.error('keyword %s expected, got %s.' % (kw, v))
		return v
			
	def match_keyword(self, kw):
		m = self.match_name()
		if not m:
			return None
		value, line = m
		if value != kw:
			return None
		return value
		
	def try_keyword(self, kw):		
		m = self.match_keyword(kw)
		if not m:
			return
		self.scan_keyword(kw)
		return m
	
	def scan_lines(self, infile):
		prefixline = ''
		prefixlineno = 0
		lineno = 0
		for l in open(infile,'r',encoding='latin-1'):
			lineno += 1
			level = 0
			# strip commentary
			if '#' in l:
				l = l[:l.index('#')]
			# strip right side spaces
			l = l.rstrip()
			if prefixline:
				l = prefixline + l
				thisline = prefixlineno
				prefixline = ''
			else:
				thisline = lineno
			# if empty, skip
			if not l:
				continue
			# if right side is backspace, store to prefix
			if l.endswith('\\'):
				if not prefixlineno:
					prefixlineno = lineno
				l = l[:-1] + ' '
				prefixline += l
				continue
			else:
				prefixlineno = 0
			# count left hand tabs
			while l[0] == '\t':
				level += 1
				l = l[1:]
			# replace all other tabs with spaces
			l = l.replace('\t', ' ')
			# fold multiple spaces down to one
			while '  ' in l:
				l = l.replace('  ',' ')
			# now we should have a quite sane string
			self.context['file'] = infile
			self.context['lineno'] = thisline
			self.context['line'] = l
			self.level = level
			self.line = l
			self.pos = 0
			yield True
		yield False
		
	def write(self, outfile):
		pass
		
	def read(self, infile):
		self.infile = infile
		self.lines = self.scan_lines(infile)
		self.parse()

	def compile(self, infile,outfile):
		self.read(infile)
		self.write(outfile)

FVALUE_FLOAT = 0 # arg0 = value
FVALUE_RANDOM = 1 # arg0 = seed, arg1 = from, arg2 = to

class MSGFuncValue:
	_tinymarshal_members_ = [
		('valuetype', int),
		('arg0', float),
		('arg1', float),
		('arg2', float),
	]
	
	def __init__(self, f=0.0):
		self.valuetype = FVALUE_FLOAT
		self.arg0 = f
		self.arg1 = 0.0
		self.arg2 = 0.0
		self.addops = {
			(FVALUE_FLOAT,FVALUE_FLOAT) : self.add_float,
			(FVALUE_FLOAT,FVALUE_RANDOM) : self.add_float_random,
			(FVALUE_RANDOM,FVALUE_FLOAT) : self.add_random_float,
		}
		self.subops = {
			(FVALUE_FLOAT,FVALUE_FLOAT) : self.sub_float,
			(FVALUE_FLOAT,FVALUE_RANDOM) : self.sub_float_random,
			(FVALUE_RANDOM,FVALUE_FLOAT) : self.sub_random_float,
		}
		self.mulops = {
			(FVALUE_FLOAT,FVALUE_FLOAT) : self.mul_float,
			(FVALUE_FLOAT,FVALUE_RANDOM) : self.mul_float_random,
			(FVALUE_RANDOM,FVALUE_FLOAT) : self.mul_random_float,
		}
		self.divops = {
			(FVALUE_FLOAT,FVALUE_FLOAT) : self.div_float,
			(FVALUE_FLOAT,FVALUE_RANDOM) : self.div_float_random,
			(FVALUE_RANDOM,FVALUE_FLOAT) : self.div_random_float,
		}
		
	def copy_from(self, other):
		self.valuetype = other.valuetype
		self.arg0 = other.arg0
		self.arg1 = other.arg1
		self.arg2 = other.arg2
		
	def add_float(self, a, b):
		self.set_float(a.arg0 + b.arg0)
		
	def add_float_random(self, a, b):
		f = a.get_final()
		self.set_random(b.arg0, b.arg1+f, b.arg2+f)
		
	def add_random_float(self, a, b):
		self.add_float_random(b, a)

	def sub_float(self, a, b):
		self.set_float(a.arg0 - b.arg0)
		
	def sub_float_random(self, a, b):
		f = a.get_final()
		self.set_random(b.arg0, f-b.arg2, f-b.arg1)
		
	def sub_random_float(self, a, b):
		f = b.get_final()
		self.set_random(a.arg0, a.arg1-f, a.arg2-f)

	def mul_float(self, a, b):
		self.set_float(a.arg0 * b.arg0)
		
	def mul_float_random(self, a, b):
		f = a.get_final()
		self.set_random(b.arg0, b.arg1*f, b.arg2*f)
		
	def mul_random_float(self, a, b):
		self.mul_float_random(b, a)

	def div_float(self, a, b):
		self.set_float(a.arg0 / b.arg0)
		
	def div_float_random(self, a, b):
		f = a.get_final()
		self.set_random(b.arg0, f / b.arg2, f / b.arg1)
		
	def div_random_float(self, a, b):
		f = b.get_final()
		self.set_random(a.arg0, a.arg1/f, a.arg2/f)

	def set_float(self, f):
		assert type(f) in (float,int)
		self.valuetype = FVALUE_FLOAT
		self.arg0 = f

	def set_random(self, seed, a, b):
		assert type(seed) in (float,int)
		assert type(a) in (float,int)
		assert type(b) in (float,int)
		self.valuetype = FVALUE_RANDOM
		self.arg0 = seed
		self.arg1 = a
		self.arg2 = b
		
	def get_bounds(self):
		if self.valuetype == FVALUE_RANDOM:
			return [self.arg1, self.arg2]
		elif self.valuetype == FVALUE_FLOAT:
			return [self.arg0]
	
	def get_final(self):
		if self.valuetype == FVALUE_FLOAT:
			return self.arg0
		elif self.valuetype == FVALUE_RANDOM:
			if self.arg0 != -1:
				random.seed(self.arg0)
			return self.arg1 + random.random() * (self.arg2 - self.arg1)
		assert 0, self.valuetype
		
	def __repr__(self):
		return "<MSGFuncValue type=%s a0=%s a1=%s a2=%s>" % (self.valuetype,self.arg0,self.arg1,self.arg2)
		
	def __ge__(self, o):
		for x in self.get_bounds():
			for y in o.get_bounds():
				if x < y:
					return False
		return True
		
	def __add__(self, o):
		v = MSGFuncValue()
		v.addops[(self.valuetype,o.valuetype)](self, o)
		return v
		
	def __sub__(self, o):
		v = MSGFuncValue()
		v.subops[(self.valuetype,o.valuetype)](self, o)
		return v
		
	def __mul__(self, o):
		v = MSGFuncValue()
		v.mulops[(self.valuetype,o.valuetype)](self, o)
		return v
		
	def __div__(self, o):
		v = MSGFuncValue()
		v.divops[(self.valuetype,o.valuetype)](self, o)
		return v
	__truediv__ = __div__

def finalize(v):
	if type(v) in (int,float,bool):
		return v
	elif isinstance(v, MSGFuncValue):
		return v.get_final()
	raise TypeError("cant finalize type %r." % type(v))

FADER_SPRING = 0
FADER_SINE = 1
FADER_TRI = 2
FADER_SQUARE = 3
FADER_POLYNOME = 4
FADER_LOG = 5

FADERS = dict(
	spring = FADER_SPRING,
	sine = FADER_SINE,
	triangle = FADER_TRI,
	square = FADER_SQUARE,
	polynome = FADER_POLYNOME,
	log = FADER_LOG,
)

class MSGFader:
	_tinymarshal_members_ = [
		('fadertype', int),
		('fader', int),
		('k', MSGFuncValue),
		('mass', MSGFuncValue),
		('friction', MSGFuncValue),
		('phase', MSGFuncValue),
		('cycles', MSGFuncValue),
		('exp1', MSGFuncValue),
		('exp2', MSGFuncValue),
	]
	def __init__(self):
		self.fadertype = FADER_SPRING
		self.fader = -1
		self.k = MSGFuncValue(1.0)
		self.mass = MSGFuncValue(1.0)
		self.friction = MSGFuncValue(1.0)
		self.phase = MSGFuncValue(0.0)
		self.cycles = MSGFuncValue(1.0)
		self.exp1 = MSGFuncValue(1.0)
		self.exp2 = MSGFuncValue(1.0)

	def finalize(self):
		pass

OP_SCALE = 0
OP_TRANSLATE = 1
OP_ROTATE = 2
OP_TRANSLATE_ROUTE = 3
OP_ROTATE_ROUTE = 4
OP_LOGSCALE = 5
OP_SKEW = 6
OP_PROJECT = 7
OP_MUL_MATRIX = 10
OP_IDENTITY_MATRIX = 11
OP_STORE_MATRIX = 12
OP_INVERT_MATRIX = 13
OP_LOAD_MATRIX = 14
OP_SET_GLOBAL = 15
OP_NOP = 255

OP_FLAG_LOOP = (1<<0)
OP_FLAG_CLAMP = (1<<1)
OP_FLAG_UNIFORM = (1<<2)

VERB_FLAG_COLOR = (1<<0)
VERB_FLAG_ZWRITE = (1<<1)
VERB_FLAG_ZREAD = (1<<2)
VERB_FLAG_ZCLEAR0 = (1<<3)
VERB_FLAG_ZCLEAR1 = (1<<4)
VERB_FLAG_ALPHA = (1<<5)
VERB_FLAG_TEXTURE = (1<<6)
VERB_FLAG_ADDCOLOR = (1<<7)
VERB_FLAG_SUBCOLOR = (1<<8)
VERB_FLAG_MULCOLOR = (1<<9)

GLOBAL_BGCOLOR = 0

VERBFLAGS = dict(
	color=VERB_FLAG_COLOR,
	zwrite=VERB_FLAG_ZWRITE,
	zread=VERB_FLAG_ZREAD,
	zclear0=VERB_FLAG_ZCLEAR0,
	zclear1=VERB_FLAG_ZCLEAR1,
	alpha=VERB_FLAG_ALPHA,
	texture=VERB_FLAG_TEXTURE,
	addcolor=VERB_FLAG_ADDCOLOR,
	subcolor=VERB_FLAG_SUBCOLOR,
	mulcolor=VERB_FLAG_MULCOLOR,
	none=0,
)

GLOBALS = dict(
	bgcolor=GLOBAL_BGCOLOR,
)

MATRIX_OPS = [
	OP_MUL_MATRIX,
	OP_STORE_MATRIX,
	OP_IDENTITY_MATRIX,
	OP_INVERT_MATRIX,
	OP_LOAD_MATRIX,
	OP_ROTATE_ROUTE,
	OP_TRANSLATE_ROUTE,
	OP_ROTATE,
	OP_TRANSLATE,
	OP_SCALE
]

CUSTOM_MATRIX_OPS = [
	OP_MUL_MATRIX,
	OP_STORE_MATRIX,
	OP_IDENTITY_MATRIX,
	OP_INVERT_MATRIX,
	OP_LOAD_MATRIX
]

MTX_MODEL = 0
MTX_ROOT = 1
MTX_ANCHOR = 2
MTX_COLOR = 3
MTX_TEXTURE = 4
MTX_CAMERA = 5
MTX_MODELCAM = 6
MTX_AUX0 = 7
MTX_AUX1 = 8
MTX_PARENT_MODEL = 9
MTX_PARENT_ROOT = 10
MTX_PARENT_ANCHOR = 11
MTX_PARENT_COLOR = 12
MTX_PARENT_TEXTURE = 13
MTX_PARENT_MODELCAM = 14
MTX_PARENT_CAMERA = 15
MTX_PARENT_AUX0 = 16
MTX_PARENT_AUX1 = 17

MATRICES = dict(
	model=MTX_MODEL,
	root=MTX_ROOT,
	anchor=MTX_ANCHOR,
	bud=MTX_ANCHOR,
	color=MTX_COLOR,
	texture=MTX_TEXTURE,
	modelcam=MTX_MODELCAM,
	cam=MTX_CAMERA,
	camera=MTX_CAMERA,
	aux0=MTX_AUX0,
	aux1=MTX_AUX1,
	parent = MTX_PARENT_MODEL,
	parent_model = MTX_PARENT_MODEL,
	parent_root = MTX_PARENT_ROOT,
	parent_color = MTX_PARENT_COLOR,
	parent_anchor = MTX_PARENT_ANCHOR,
	parent_texture = MTX_TEXTURE,
	parent_bud = MTX_PARENT_ANCHOR,
	parent_modelcam=MTX_PARENT_MODELCAM,
	parent_aux0 = MTX_PARENT_AUX0,
	parent_aux1 = MTX_PARENT_AUX1,
	parent_cam = MTX_PARENT_CAMERA,
	parent_camera = MTX_PARENT_CAMERA,
)

class MSGOp:
	_tinymarshal_members_ = [
		('op', int),
		('index', int),
		('index2', int),
		('fromtime', MSGFuncValue),
		('totime', MSGFuncValue),
		('fromval', MSGFuncValue, MSGFuncValue, dict(islist=True)),
		('toval', MSGFuncValue, MSGFuncValue, dict(islist=True)),
		('flags', int),
		('fader', int),
	]
	
	def __init__(self, op=OP_NOP, index = MTX_MODEL):
		self.op = op
		self.fromtime = MSGFuncValue(0)
		self.totime = MSGFuncValue(0)
		self.flags = 0
		self.index = index
		self.index2 = MTX_MODEL
		self.fader = -1
		if op == OP_SCALE:
			self.fromval = [MSGFuncValue(1),MSGFuncValue(1),MSGFuncValue(1),MSGFuncValue(1)]
			self.toval = [MSGFuncValue(1),MSGFuncValue(1),MSGFuncValue(1),MSGFuncValue(1)]
		else:
			self.fromval = [MSGFuncValue(0),MSGFuncValue(0),MSGFuncValue(0),MSGFuncValue(0)]
			self.toval = [MSGFuncValue(0),MSGFuncValue(0),MSGFuncValue(0),MSGFuncValue(0)]
			
	def copy_from(self, other):
		self.op = other.op
		self.fromtime.copy_from(other.fromtime)
		self.totime.copy_from(other.totime)
		self.flags = other.flags
		self.index = other.index
		self.index2 = other.index2
		self.fader = other.fader
		for i in range(4):
			self.fromval[i].copy_from(other.fromval[i])
			self.toval[i].copy_from(other.toval[i])
			
	def finalize(self):
		pass

class MSGResolveValue:
	_tinymarshal_members_ = [
		('value', MSGFuncValue),
	]
	
	def __init__(self):
		self.value = MSGFuncValue(0.0)
		self.parent = False
		self.siblings = False
		
	def copy_from(self, other):
		self.value.copy_from(other.value)
		self.parent = other.parent
		self.siblings = other.siblings

class MSGVerb:
	_tinymarshal_members_ = [
		('flags', int),
		('ops', MSGOp, MSGOp, dict(islist=True)),
	]
	
	def __init__(self):
		self.ops = []
		self.flags = 0
		self.id = ''
		self.delay = MSGResolveValue()
		self.focus = MSGResolveValue()
		self.lifespan = MSGResolveValue()
		self.seed = 0

	def inherit_from(self, other):
		self.delay.copy_from(other.delay)
		self.focus.copy_from(other.focus)
		self.lifespan.copy_from(other.lifespan)
		self.flags |= other.flags
		for op in other.ops:
			newop = MSGOp()
			newop.copy_from(op)
			self.ops.append(newop)

	def finalize(self):
		if self.seed:
			random.seed(self.seed)
		custom = False
		customcam = False
		for op in self.ops:
			if op.op in MATRIX_OPS:
				if op.index in (MTX_MODELCAM, MTX_PARENT_MODELCAM):
					customcam = True
				elif op.index2 in (MTX_MODELCAM, MTX_PARENT_MODELCAM):
					customcam = True
			if op.op in CUSTOM_MATRIX_OPS:
				custom = True
		if not custom:
			self.ops = [MSGOp(OP_MUL_MATRIX,MTX_ROOT)] + self.ops + [MSGOp(OP_MUL_MATRIX,MTX_ANCHOR),MSGOp(OP_MUL_MATRIX,MTX_PARENT_MODEL)]
		if not customcam:
			self.ops = self.ops + [MSGOp(OP_STORE_MATRIX,MTX_MODELCAM)]
		for op in self.ops:
			op.finalize()

class MSGRef:
	def __init__(self):
		self.startset = []
		self.rules = {}

class MSGObj(MSGRef):
	_tinymarshal_members_ = [
		('source', str),
		('model', int),
		('group', str),
		('verb', int),
		('bud', int),
		('back', bool),
		('delay', MSGResolveValue),
		('focus', MSGResolveValue),
		('lifespan', MSGResolveValue),
		('children', 'MSGObj', 'MSGObj', dict(islist=True)),
	]
	
	def __init__(self):
		MSGRef.__init__(self)
		self.model = -1
		self.group = ""
		self.back = False
		self.id = ""
		self.bud = -1
		self.context = {}
		self.source = ""
		self.verb = -1
		self.parent = None
		self.delay = MSGResolveValue()
		self.focus = MSGResolveValue()
		self.lifespan = MSGResolveValue()
		self.children = []
		
	def get_tree(self):
		res = [(0, self)]
		for c in self.children:
			for l,e in c.get_tree():
				res += [(l+1,e)]
		return res
		
	def get_deepest_child(self):
		deepest = self
		for c in self.children:
			deepest = c.get_deepest_child()
		return deepest
		
	def find_object(self, id):
		if self.id == id:
			return self
		for c in self.children:
			r = c.find_object(id)
			if r:
				return r
		return None
		
	def get_deepest_children(self):
		kids = []
		for c in self.children:
			if c.children:
				kids += c.get_deepest_children()
			else:
				kids += [c]
		return kids
		
	def get_copy(self):
		obj = MSGObj()
		obj.model = self.model
		obj.back = self.back
		obj.bud = self.bud
		obj.id = self.id
		obj.context = self.context
		obj.verb = self.verb
		obj.group = self.group
		obj.delay.copy_from(self.delay)
		obj.focus.copy_from(self.focus)
		obj.lifespan.copy_from(self.lifespan)
		for c in self.children:
			n = c.get_copy()
			n.parent = obj
			obj.children.append(n)
		return obj
		
	def finalize(self, msg, st=[MSGFuncValue(0.0),MSGFuncValue(0.0),MSGFuncValue(0.0)]):
		if self.context:
			self.source = msg.format_context(self.context)
		for i,name in enumerate(['delay','focus','lifespan']):
			sibtime = st[i]
			v = getattr(self, name)
			if self.verb != -1:
				vv = getattr(msg.verbs[self.verb], name)
				v.value += vv.value
				v.siblings = v.siblings or vv.siblings
				v.parent = v.parent or vv.parent
			if v.siblings:
				sibtime += v.value
				v.value.copy_from(sibtime)
			else:
				sibtime.set_float(0.0)
			if v.parent and self.parent:
				v.value += getattr(self.parent, name).value
			st[i] = sibtime
		if self.parent and (self.lifespan.value.get_final() <= 0.0):
			msg.warning("entity will never die", self.context)
		s = [MSGFuncValue(0.0),MSGFuncValue(0.0),MSGFuncValue(0.0)]
		for c in self.children:
			s = c.finalize(msg,s)
		return st

register('MSGObj', MSGObj)

class MSGBranch(MSGObj):
	def __init__(self):
		MSGObj.__init__(self)

class MSGCompiler(Compiler):
	_tinymarshal_members_ = [
		('song', str),
		('bpm', float),
		('bgcolor', vec3),
		('faders', MSGFader, MSGFader, dict(islist=True)),
		('verbs', MSGVerb, MSGVerb, dict(islist=True)),
		('models', strlist),
		('root', MSGObj, MSGObj),
	]
	
	classes = dict(
		verb = MSGVerb,
		fader = MSGFader,
		branch = MSGBranch,
	)
	
	def __init__(self):
		Compiler.__init__(self)
		self.set_bpm(120.0)
		self.song = ""
		self.stack = []
		self.objects = {}
		self.verbs = []
		self.bgcolor = [1.0,1.0,1.0]
		self.faders = []
		self.models = []
		self.modelrefs = {}
		self.root = MSGObj()
		self.branches = {}
		self.aliases = {}
		self.branch = None
		self.nextid = 1
		
	def set_bpm(self, bpm):
		self.bpm = bpm
		self.spb = 60.0 / bpm
		
	def push(self, item):
		self.stack.append(item)
		
	def top(self, *classes):
		obj = self.stack[-1]
		found = False
		for classobj in classes:
			if classobj and isinstance(obj,classobj):
				found = True
		if not found:
			self.error('keyword can not be used with this parent type.')
		return obj
		
	def pop(self):
		self.stack.pop()
		
	def verify(self, x, msg):
		if not x:
			self.error(msg)
			
	def scan_number(self,allow_func=True,finalize=False):
		v = MSGFuncValue(0.0)
		if self.try_bo():
			while not self.try_bc():
				if self.try_keyword('random'):
					if not allow_func:
						self.error('number must be pure float.')
					fb = self.scan_number(allow_func=False, finalize=True)
					self.scan_keyword('to')
					fe = self.scan_number(allow_func=False, finalize=True)
					v.set_random(-1, fb, fe)
				elif self.match_symbol():
					name = self.scan_symbol()
					if not name in self.consts:
						self.error("constant expected, got %s." % name)
					v += self.consts[name]
				elif self.match_bo():
					v += self.scan_number(allow_func=allow_func)
				elif self.try_add():
					v += self.scan_number(allow_func=allow_func)
				elif self.try_sub():
					v -= self.scan_number(allow_func=allow_func)
				elif self.try_mul():
					v *= self.scan_number(allow_func=allow_func)
				elif self.try_div():
					v /= self.scan_number(allow_func=allow_func)
				elif self.match_float():
					v += MSGFuncValue(self.translate_float(self.scan_float()))
				else:
					self.error("')' expected.")
		elif self.match_symbol():
			name = self.scan_symbol()
			if not name in self.consts:
				self.error("constant expected, got %s." % name)
			v.copy_from(self.consts[name])
		else:
			v.set_float(self.translate_float(self.scan_float()))
		if finalize:
			return v.get_final()
		else:
			return v
		
	def translate_float(self, fe):
		(f,e) = fe
		f = float(f)
		if not e:
			return f
		elif e == '%':
			return f/100.0
		elif e in ('ms','msec','msecs','millisec','millisecs','milliseconds'):
			return f/1000.0
		elif e in ('s','sec','secs','second','seconds'):
			return f
		elif e in ('m','min','mins','minute','minutes'):
			return f*60.0
		elif e in ('hz',):
			if f == 0:
				self.error('invalid argument.')
			return 1.0/f
		elif e in ('h','hour','hours'):
			return f*60.0*60.0
		elif e in ('b','beat','beats'):
			return self.spb*f
		elif e in ('t','tick','ticks'):
			return (self.spb*f) / 12.0
		elif e in ('br','bar','bars'):
			return self.spb*4*f
		elif e in ('\xb0','deg','degree','degs','degrees'):
			return f*(math.pi / 180.0)
		self.error('unknown measuring unit: %s' % e)
		
	def try_time(self):
		f = 0
		if self.try_keyword('time'):
			f |= OP_FLAG_CLAMP
		elif self.try_keyword('seg'):
			pass
		else:
			return MSGFuncValue(0.0),MSGFuncValue(0.0),f
		a = self.scan_number()
		if self.try_to():
			b = self.scan_number()
			tb,te = a,b
		else:
			tb,te = MSGFuncValue(0.0),a
		self.verify(te >= tb, 'until time must be larger or equal (%s > %s)' % (tb,te))
		return tb,te,f
		
	def scan_number_list(self):
		c = []
		while True:
			x = self.scan_number()
			c.append(x)
			if not self.try_comma():
				break
		return c
		
	def scan_vec4(self, opcode):
		c = self.scan_number_list()
		if len(c) > 4:
			self.error('too many numbers in vector.')
		elif len(c) == 1:
			if opcode in (OP_SCALE, OP_LOGSCALE):
				c = [c[0],c[0],MSGFuncValue(1.0),MSGFuncValue(1.0)]
			else:
				c = [MSGFuncValue(0.0),MSGFuncValue(0.0),c[0],MSGFuncValue(0.0)]
		elif len(c) == 2:
			if opcode in (OP_SCALE, OP_LOGSCALE):
				c = [c[0],c[1],MSGFuncValue(1.0),MSGFuncValue(1.0)]
			else:
				c = [c[0],c[1],MSGFuncValue(0.0),MSGFuncValue(0.0)]
		elif len(c) == 3:
			if opcode in (OP_SCALE, OP_LOGSCALE):
				c = [c[0],c[1],c[2],MSGFuncValue(1.0)]
			else:
				c = [c[0],c[1],c[2],MSGFuncValue(0.0)]
		return c
		
	def try_range(self, opcode):
		rb = self.scan_vec4(opcode)
		if self.try_to():
			re = self.scan_vec4(opcode)
		else:
			re = rb
		return rb,re
		
	def try_matrix_index(self, default=MTX_MODEL):
		if not self.match_name():
			return default
		name = self.scan_name()
		if not name in MATRICES:
			self.error('unknown matrix specified.')
		return MATRICES[name]
		
	def try_global_index(self, default=-1):
		if not self.match_name():
			return default
		name = self.scan_name()
		if not name in GLOBALS:
			self.error('unknown global specified.')
		return GLOBALS[name]
		
	def scan_transform(self, opcode):
		obj = self.top(MSGVerb)
		index = self.try_matrix_index(MTX_MODEL)
		op = MSGOp(opcode, index)
		rb,re = self.try_range(opcode)
		if self.try_keyword('uniform'):
			op.flags |= OP_FLAG_UNIFORM
		tb,te,f = self.try_time()
		if (f & OP_FLAG_CLAMP) and (index == MTX_CAMERA):
			self.error("camera should be modified with seg, not time.")
		op.flags |= f
		if self.try_keyword('loop'):
			op.flags |= OP_FLAG_LOOP
		if self.try_keyword('fader'):
			fadername = self.scan_name()
			if fadername in list(self.objects.keys()):
				fader = self.objects[fadername]
				op.fader = self.faders.index(fader)
			else:
				self.error('unknown fader: %s.' % fadername)
		op.fromval = rb
		op.toval = re
		op.fromtime = tb
		op.totime = te
		obj.ops.append(op)
		return op
		
	def func_trans(self):
		self.func_multiply()
	def func_mul(self):
		self.func_multiply()
		
	def func_mtx_op(self, opcode):
		obj = self.top(MSGVerb)
		index = self.try_matrix_index(-1)
		if index == -1:
			self.error('need to specify a matrix.')
		op = MSGOp(opcode, index)
		op.index2 = self.try_matrix_index(MTX_MODEL)
		obj.ops.append(op)
		
	def func_multiply(self):
		self.func_mtx_op(OP_MUL_MATRIX)

	def func_identity(self):
		self.func_mtx_op(OP_IDENTITY_MATRIX)

	def func_invert(self):
		self.func_mtx_op(OP_INVERT_MATRIX)
	def func_store(self):
		self.func_mtx_op(OP_STORE_MATRIX)
	def func_load(self):
		self.func_mtx_op(OP_LOAD_MATRIX)
	def func_set(self):
		obj = self.top(MSGVerb)
		index = self.try_global_index(-1)
		if index == -1:
			self.error('need to specify a target.')
		op = MSGOp(OP_SET_GLOBAL, index)
		op.index2 = self.try_matrix_index(MTX_AUX1)
		obj.ops.append(op)

	def func_scale(self):
		self.scan_transform(OP_SCALE)
	def func_size(self):
		self.scan_transform(OP_SCALE)
	def func_logscale(self):
		self.scan_transform(OP_LOGSCALE)
	def func_logsize(self):
		self.scan_transform(OP_LOGSCALE)
	def func_rotate(self):
		self.scan_transform(OP_ROTATE)
	def func_turn(self):
		self.scan_transform(OP_ROTATE)
	def func_skew(self):
		self.scan_transform(OP_SKEW)
	def func_project(self):
		self.scan_transform(OP_PROJECT)
	def func_translate(self):
		self.scan_transform(OP_TRANSLATE)
	def func_move(self):
		self.scan_transform(OP_TRANSLATE)
		
	def func_route(self):
		if self.try_at():
			index = max(int(self.scan_number(finalize=True)+0.5)-1,0)
		else:
			index = 0
		target = self.scan_name()
		if target in ['turn', 'rotate']:
			op = self.scan_transform(OP_ROTATE_ROUTE)
		elif target in ['move', 'translate']:
			op = self.scan_transform(OP_TRANSLATE_ROUTE)
		else:
			self.error('"move" or "turn" after route expected')
		op.index2 = index

	def scan_timing(self, v):
		while True:
			if self.try_keyword('parent') or self.try_keyword('p'):
				v.parent = True
			if self.try_keyword('siblings') or self.try_keyword('s'):
				v.siblings = True
			if not self.try_plus():
				break
		v.value = v.value + self.scan_number()
		
	def func_delay(self):
		obj = self.top(MSGVerb,MSGObj)
		self.scan_timing(obj.delay)
	def func_focus(self):
		obj = self.top(MSGVerb,MSGObj)
		self.scan_timing(obj.focus)
		self.warning("focus is deprecated")
	def func_lifespan(self):
		obj = self.top(MSGVerb,MSGObj)
		self.scan_timing(obj.lifespan)
		
	def func_const(self):
		name = self.scan_name()
		if name in list(self.objects.keys()):
			self.error('already defined: %s', name)
		v = self.scan_number()
		self.objects[name] = v
		self.consts[name] = v
		
	def func_alias(self):
		name = self.scan_name()
		if name in list(self.objects.keys()):
			self.error('already defined: %s', name)
		oldname = self.scan_name()
		self.objects[name] = oldname
		self.aliases[name] = oldname
		
	def func_options(self):
		obj = self.top(MSGVerb)
		mode = 1
		while True:			
			if self.try_plus():
				mode = 1
			elif self.try_minus():
				mode = -1
			elif self.try_equal():
				mode = 0
			elif (mode != 0xffff) and self.match_name():
				flagname = self.scan_name()
				if not flagname in VERBFLAGS:
					self.error('unknown verb option: %s' % flagname)
				flag = VERBFLAGS[flagname]
				if mode == 0:
					obj.flags = flag
				elif mode == 1:
					obj.flags |= flag
				elif mode == -1:
					obj.flags &= ~flag
				mode = 0xffff
			else:
				break
		
	def func_like(self):
		obj = self.top(MSGVerb)
		name = self.scan_name()
		if name in list(self.objects.keys()):
			oobj = self.objects[name]
			obj.inherit_from(oobj)
		else:
			self.error('unknown verb: %s' % name)
			
	def func_seed(self):
		obj = self.top(MSGVerb)
		self.scan_keyword('is')
		obj.seed = self.scan_number(allow_func=False)
		
	def func_type(self):
		obj = self.top(MSGFader)
		name = self.scan_name()
		if not name in FADERS:
			self.error('unknown fadertype: %s' % name)
		obj.fadertype = FADERS[name]
			
	def func_k(self):
		obj = self.top(MSGFader)
		obj.k = self.scan_number()
	def func_mass(self):
		obj = self.top(MSGFader)
		obj.mass = self.scan_number()
	def func_friction(self):
		obj = self.top(MSGFader)
		obj.friction = self.scan_number()
	def func_phase(self):
		obj = self.top(MSGFader)
		obj.phase = self.scan_number()
	def func_cycles(self):
		obj = self.top(MSGFader)
		obj.cycles = self.scan_number()
	def func_exp1(self):
		obj = self.top(MSGFader)
		obj.exp1 = self.scan_number()
	def func_exp2(self):
		obj = self.top(MSGFader)
		obj.exp2 = self.scan_number()
		
	def func_bpm(self):
		if not self.level == 0:
			self.error('bpm must be declared toplevel.')
		self.set_bpm(self.scan_number(allow_func=False,finalize=True))

	def func_bgcolor(self):
		if not self.level == 0:
			self.error('bgcolor must be declared toplevel.')
		h = self.scan_number(allow_func=False,finalize=True)
		self.scan_comma()
		s = self.scan_number(allow_func=False,finalize=True)
		self.scan_comma()
		l = self.scan_number(allow_func=False,finalize=True)
		self.bgcolor = [h,s,l]

	def func_song(self):
		if not self.level == 0:
			self.error('bpm must be declared toplevel.')
		songpath = self.try_string1() or self.try_string2()
		if not songpath:
			self.error('invalid argument, string expected.')
		self.song = songpath

	def try_buds(self):
		buds = []
		defback = False
		defcount = 1
		if self.try_excl():
			defback = True
		if self.try_asterisk():
			defcount = max(int(self.scan_number(allow_func=False,finalize=True)+0.5), 1)
		if self.try_at():
			while True:
				back = defback
				if self.try_excl():
					back = not defback
				count = defcount
				bud = -1
				if self.match_name():
					budname = self.scan_name()
					bud = 0
				elif self.match_float():
					bud = max(int(self.scan_number(allow_func=False,finalize=True)+0.5)-1, -1)
				if self.try_asterisk():
					count = max(int(self.scan_number(allow_func=False,finalize=True)+0.5), 1)
				for n in range(count):
					buds.append([back,bud])
				if not self.try_comma():
					break
		if not buds:
			for n in range(defcount):
				buds.append([defback,-1])
		return buds
		
	def add_object(self,name,obj):
		if name in self.objects:
			self.error('%s already defined.' % name)
		self.objects[name] = obj
		
	def func_import(self):
		scriptpath = self.try_string1() or self.try_string2()
		if not scriptpath:
			self.error('path to .msg file expected.')
		infile = os.path.abspath(self.infile)
		if not os.path.isabs(scriptpath):
			scriptpath = os.path.abspath(os.path.join(os.path.dirname(infile), scriptpath))
		if not os.path.isfile(scriptpath):
			self.error('file not found: %s' % scriptpath)
		self.scan_keyword("as")
		ns = self.scan_name() # namespace
		delay = MSGFuncValue(0.0)
		if self.try_keyword("delay"):
			delay = self.scan_number()
		c = MSGCompiler()
		c.read(scriptpath)
		faderindex = len(self.faders)
		verbindex = len(self.verbs)
		modelindex = len(self.models)
		for fader in c.faders:
			self.faders.append(fader)
		for verb in c.verbs:
			for op in verb.ops:
				if op.fader != -1:
					if len(c.faders) <= op.fader:
						self.error("fader %i of verb %s > fader count %i" % (op.fader, verb.id, len(c.faders)))
					op.fader += faderindex
			self.verbs.append(verb)
		for model in c.models:
			if not os.path.isabs(model):
				model = os.path.join(os.path.dirname(scriptpath), model)
				pfx = os.path.commonprefix([model, infile])
				model = model[len(pfx):]
			self.models.append(model)
		def offsetindices(obj):
			if obj.model != -1:
				obj.model += modelindex
			pdelay = obj.delay.parent
			sdelay = obj.delay.siblings
			if obj.verb != -1:
				obj.verb += verbindex
				v = self.verbs[obj.verb]
				pdelay = pdelay or v.delay.parent
				sdelay = sdelay or v.delay.siblings
			if not pdelay:
				obj.delay.value += delay
			for child in obj.children:
				offsetindices(child)			
		offsetindices(c.root)
		branch = MSGBranch()
		branch.children = c.root.children
		for c in branch.children:
			c.parent = branch
		self.add_object(ns, branch)
		
	def try_branchbuds(self):
		ids = []
		if self.try_sbo():
			while not self.try_sbc():
				id = self.scan_name()
				if not self.match_sbc():
					self.scan_comma()
				ids.append(id)
		return ids
		
	def func_ref(self):
		parent = self.root
		if self.stack:
			parent = self.top(MSGRef,MSGObj)
		buds = self.try_buds()
		branchbuds = self.try_branchbuds()
		refname = self.scan_name()
		if not refname in list(self.objects.keys()):
			self.error('unknown reference: %s' % refname)
		obj = self.objects[refname]
		if not isinstance(obj, MSGBranch):
			self.error('%s is not a branch.' % refname)
		newkids = []
		if isinstance(parent, MSGObj):
			parents = [parent]
		else:
			parents = []
			if not branchbuds:
				for o in parent.startset:
					parents += [o.get_deepest_child()]
			else:
				for bbud in branchbuds:
					bbudobjs = []
					for o in parent.startset:
						bbudobj = o.find_object(bbud)
						if bbudobj:
							bbudobjs.append(bbudobj)
					if not bbudobjs:
						self.error('cant find entity %s' % bbud)
					parents += bbudobjs
			if not parents:
				parents = parent.parents
		for parent in parents:
			for back,bud in buds:
					for n in obj.children:
						newobj = n.get_copy()
						newobj.parent = parent
						parent.children.append(newobj)
						if newobj.bud == -1:
							newobj.back = back						
							newobj.bud = bud
						newkids.append(newobj)
		if self.try_colon():
			ref = MSGRef()
			ref.parents = parents
			ref.startset = newkids
			self.push(ref)
			
	def try_modelgroup(self):
		if self.try_dot():
			return self.scan_name()
		return ""
		
	def try_object_id(self):
		if self.try_bo():
			id = ""
			if self.try_keyword('name'):
				self.scan_equal()
				id = self.scan_name()
			self.scan_bc()
			return id
		return ""
			
	def func_rule(self):
		ref = self.top(MSGRef,MSGObj)
		verbname = self.scan_name()
		gfxname = self.scan_name()
		modelname = self.try_modelgroup()
		verb = self.objects.get(verbname)
		if not isinstance(verb, MSGVerb):
			self.error('%s is not a verb.' % verbname)
		if not gfxname in self.models:
			self.error('%s is not a model.' % gfxname)
		modelindex = self.models.index(gfxname)
		verbindex = self.verbs.index(verb)
		if (verbindex,modelindex) in ref.rules:
			self.error('rule for %s %s is already defined.' % (verbname,gfxname))
		self.scan_to()
		if self.try_keyword('ref'):
			refname = self.scan_name()
			if not refname in list(self.objects.keys()):
				self.error('unknown reference: %s' % refname)
			obj = self.objects[refname]
			if not isinstance(obj, MSGBranch):
				self.error('%s is not a branch.' % refname)
		else:
			name = self.scan_name()
			if not self.parse_classobj(name):
				self.error('can not interpret keyword %s here.' % name)
			obj = self.top(MSGBranch)
		ref.rules[(verbindex,modelindex,modelname)] = obj
		
	def iterate(self, objects, ref, count):
		if count == 0:
			return objects
		bottoms = []
		for obj in objects:
			found = False
			for key,branch in ref.rules.items():
				if (obj.verb,obj.model,obj.group) == key:
					found = True
					newkids = []
					for branchobj in branch.children:
						newobj = branchobj.get_copy()
						newobj.parent = obj
						obj.children.append(newobj)
						kids = newobj.get_deepest_children()
						if not kids:
							newkids.append(newobj)
						else:
							newkids += kids
					bottoms += self.iterate(newkids, ref, count-1)
					break
		return bottoms
		
	def func_iterate(self):
		ref = self.top(MSGRef,MSGObj)
		if isinstance(ref, MSGObj):
			if not ref.startset:
				ref.startset = [ref]
		count = max(int(self.scan_number(finalize=True)+0.5), 1)
		ref.startset = self.iterate(ref.startset, ref, count)
		ref.rules = {}
		
	def load_modelref(self, gfxname):
		pass
		#~ from svg2bsm import BSMBuilder
		#~ outpath = os.path.dirname(self.outfile)
		#~ filename = os.path.join(outpath, gfxname + '.bsm')
		#~ if not os.path.isfile(filename):
			#~ self.error("%s: no such model." % filename)
		#~ data = file(filename, "rb").read()
		#~ modelref = unpack(data, BSMBuilder(filename))
		#~ self.modelrefs[gfxname] = modelref
		
	def make_id(self, classname):
		id = "%s%08x" % (classname, self.nextid)
		self.nextid += 1
		return id
		
	def parse_classobj(self, name):
		classobj = self.classes.get(name, None)
		if classobj:
			obj = classobj()
			if self.match_name():
				id = self.scan_name()
			else:
				id = self.make_id(name)
			if id in list(self.objects.keys()):
				self.error('already defined: %s' % id)
			self.objects[id] = obj
			if isinstance(obj, MSGVerb):
				obj.id = id
				self.verbs.append(obj)
			elif isinstance(obj, MSGFader):
				if self.try_bo():
					fadername = self.scan_name()
					if not fadername in self.objects:
						self.error('unknown fader: %s' % id)
					f = self.objects[fadername]
					if not isinstance(f, MSGFader):
						self.error('%s is not a fader.' % id)
					obj.fader = self.faders.index(f)
					self.scan_bc()
				self.faders.append(obj)
			#~ while (not self.match_colon()) and (self.line):
				#~ if self.match_name():
					#~ attrname = self.scan_name()
				#~ else:
				#~ self.error('attribute expected.')
			self.scan_colon()
			self.push(obj)
			return True
		return False
		
	def parse_func(self, name):
		if not hasattr(self, 'func_'+name):
			self.error('unknown function: %s' % name)
		func = getattr(self, 'func_'+name)
		if func:
			func()

	def parse(self):
		try:
			while next(self.lines):
				if self.level > len(self.stack):
					self.error('unexpected indentation.')
				while self.level < len(self.stack):
					self.pop()
				name = self.try_name()
				if name in list(self.objects.keys()):
					obj = self.objects[name]
					if isinstance(obj, MSGVerb):
						buds = self.try_buds()
						gfxname = self.scan_name()
						modelgroup = self.try_modelgroup()
						if (gfxname != 'none') and (not gfxname in self.models):
							self.models.append(gfxname)
							self.load_modelref(gfxname)
						objid = self.try_object_id()
						parent = self.root
						if self.stack:
							parent = self.top(MSGObj)
						items = []
						for back,bud in buds:
							b = MSGObj()
							b.context = dict(self.context)
							if gfxname != 'none':
								b.model = self.models.index(gfxname)
							b.id = objid
							b.group = modelgroup
							b.verb = self.verbs.index(obj)
							b.parent = parent
							parent.children.append(b)
							b.back = back
							b.bud = bud
							items.append(b)
						self.push(b)
						while self.match_name():
							self.parse_func(self.scan_name())
						if self.try_colon() and len(buds) != 1:
							self.error('can not descend from declaration yielding multiple children.')
					else:
						self.error('cannot use %s at beginning of declaration.' % name)
				elif not self.parse_classobj(name) and name:
					self.parse_func(name)
				if self.line:
					self.error('unexpected characters at end of line: %s' % self.line)
		except StopIteration:
			self.error('unexpected end of file.')
			
	def finalize(self):
		for verb in self.verbs:
			verb.finalize()
		for fader in self.faders:
			fader.finalize()
		self.root.finalize(self)

	def write(self, outfile):
		self.finalize()
		f = open(outfile, 'wb')
		data = pack(self)
		f.write(data)
		f.close()

if __name__ == '__main__':
	compiler = MSGCompiler()
	try:
		compiler.compile(sys.argv[1], sys.argv[2])
	except CompileError:
		raise SystemExit(1)
