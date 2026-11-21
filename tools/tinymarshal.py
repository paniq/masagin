# Better Demo Engine for Python
# Copyright (C) 2005,2006 Leonard Ritter (contact@leonard-ritter.com)

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

"""
better.tinymarshal provides functions to de/serialize nested data structures
into a data format that uses as little space as possible. tinymarshal
can not deserialize data without knowing the original data structures.
"""

import struct

class UnpackException(Exception):
	pass

PACK_PREFIX = '<'

class intlist(type):
	pass
class uintlist(type):
	pass
class strlist(type):
	pass
class uint(type):
	pass
class ushort(type):
	pass
class vec2(type):
	pass
class vec3(type):
	pass
class vec4(type):
	pass
class vec2list(type):
	pass
class vec3list(type):
	pass
class vec4list(type):
	pass
class int3list(type):
	pass
class short2list(type):
	pass
class shortlist(type):
	pass
class ushort2list(type):
	pass
class short3list(type):
	pass
class vec3p(type):
	pass

def _pack_atomic(value):
	return value

def _pack_float(value):
	return struct.pack(PACK_PREFIX + 'f',value)

def _unpack_float(value):
	return struct.unpack(PACK_PREFIX + 'f',value[:4])[0],value[4:]

def _pack_int(value):
	return struct.pack(PACK_PREFIX + 'i',int(value))

def _unpack_int(value):
	return struct.unpack(PACK_PREFIX + 'i',value[:4])[0],value[4:]


def _pack_str(value):
	if isinstance(value, str):
		value = value.encode('utf-8', 'surrogateescape')
	assert len(value) < 32767
	return _pack_short(len(value)) + struct.pack(PACK_PREFIX + '%is' % (len(value)),value)

def _unpack_str(value):
	length,value = _unpack_short(value)
	return struct.unpack(PACK_PREFIX + '%is' % length,value[:length])[0].decode('utf-8', 'surrogateescape'),value[length:]

def _pack_short(value):
	return struct.pack(PACK_PREFIX + 'h',int(value))

def _unpack_short(value):
	return struct.unpack(PACK_PREFIX + 'h',value[:2])[0],value[2:]

def _pack_ushort(value):
	return struct.pack(PACK_PREFIX + 'H',int(value))

def _unpack_ushort(value):
	return struct.unpack(PACK_PREFIX + 'H',value[:2])[0],value[2:]

def _pack_uint(value):
	return struct.pack(PACK_PREFIX + 'I',int(value))

def _unpack_uint(value):
	return struct.unpack(PACK_PREFIX + 'I',value[:4])[0],value[4:]

def _pack_vec3_or_none(value):
	if value == None:
		return struct.pack(PACK_PREFIX + 'b',0)
	return struct.pack(PACK_PREFIX + 'bfff',1,*value)
	
def _unpack_vec3_or_none(value):
	is_set,value = struct.unpack(PACK_PREFIX + 'b',value[:1])[0],value[1:]
	if is_set:
		return struct.unpack(PACK_PREFIX + 'fff',value[:12]),value[12:]
	else:
		return None,value

def _pack_vec2(value):
	return struct.pack(PACK_PREFIX + 'ff',*value)
	
def _unpack_vec2(value):
	return struct.unpack(PACK_PREFIX + 'ff',value[:8]),value[8:]

def _pack_vec3(value):
	return struct.pack(PACK_PREFIX + 'fff',*value)
	
def _unpack_vec3(value):
	return struct.unpack(PACK_PREFIX + 'fff',value[:12]),value[12:]

def _pack_int3(value):
	return struct.pack(PACK_PREFIX + 'iii',*[int(v) for v in value])

def _unpack_int3(value):
	return struct.unpack(PACK_PREFIX + 'iii',value[:12]),value[12:]

def _pack_short2(value):
	return struct.pack(PACK_PREFIX + 'hh',*[int(v) for v in value])

def _unpack_short2(value):
	return struct.unpack(PACK_PREFIX + 'hh',value[:4]),value[4:]

def _pack_ushort2(value):
	return struct.pack(PACK_PREFIX + 'HH',*[int(v) for v in value])

def _unpack_ushort2(value):
	return struct.unpack(PACK_PREFIX + 'HH',value[:4]),value[4:]

def _pack_short3(value):
	return struct.pack(PACK_PREFIX + 'hhh',*[int(v) for v in value])

def _unpack_short3(value):
	return struct.unpack(PACK_PREFIX + 'hhh',value[:6]),value[6:]

def _pack_vec4(value):
	return struct.pack(PACK_PREFIX + 'ffff',*value)
	
def _unpack_vec4(value):
	return struct.unpack(PACK_PREFIX + 'ffff',value[:16]),value[16:]

def _pack_object(value):
	return value

def _pack_entity(object):
	return pack_netlist(object,entity_exports)

def _unpack_entity(data):
	return unpack_netlist(data,ClientEntity,entity_exports)
	
def _pack_list(values): # struct.pack a list of strings
	return struct.pack(PACK_PREFIX + 'I',len(values)) + b''.join(values)

def _unpack_list(data,func,*args): # struct.unpack to a list of elements
	count,data = struct.unpack(PACK_PREFIX + 'I',data[:4])[0],data[4:]
	values = [None] * count
	for index in range(count):
		values[index],data = func(data,*args)
	return values,data

def _unpack_list_nd(data, sdef):
	ofs = struct.calcsize(sdef)
	count = struct.unpack(PACK_PREFIX + 'I',data[:4])[0]
	values = [None] * count
	di = 4
	n = 0
	for index in range(count):
		values[index] = struct.unpack(sdef,data[di:di+ofs])
		di += ofs
	return values,data[di:]

def _pack_int_list(value):
	return _pack_list([_pack_int(object) for object in value])

def _unpack_int_list(data):
	return _unpack_list(data,_unpack_int)

def _pack_uint_list(value):
	return _pack_list([_pack_uint(object) for object in value])

def _unpack_uint_list(data):
	return _unpack_list(data,_unpack_uint)

def _pack_int3_list(value):
	return _pack_list([_pack_int3(object) for object in value])

def _unpack_int3_list(data):
	return _unpack_list_nd(data, PACK_PREFIX + 'iii')

def _pack_vec2_list(value):
	return _pack_list([_pack_vec2(object) for object in value])

def _unpack_vec2_list(data):
	return _unpack_list_nd(data, PACK_PREFIX + 'ff')

def _pack_str_list(value):
	return _pack_list([_pack_str(object) for object in value])

def _unpack_str_list(data):
	return _unpack_list(data, _unpack_str)

def _pack_short2_list(value):
	return _pack_list([_pack_short2(object) for object in value])

def _unpack_short2_list(data):
	return _unpack_list_nd(data, PACK_PREFIX + 'hh')

def _pack_short_list(value):
	return _pack_list([_pack_short(object) for object in value])

def _unpack_short_list(data):
	return _unpack_list_nd(data, PACK_PREFIX + 'h')

def _pack_ushort2_list(value):
	return _pack_list([_pack_ushort2(object) for object in value])

def _unpack_ushort2_list(data):
	return _unpack_list_nd(data, PACK_PREFIX + 'HH')

def _pack_short3_list(value):
	return _pack_list([_pack_short3(object) for object in value])

def _unpack_short3_list(data):
	return _unpack_list_nd(data, PACK_PREFIX + 'hhh')

def _pack_vec3_list(value):
	return _pack_list([_pack_vec3(object) for object in value])

def _unpack_vec3_list(data):
	return _unpack_list_nd(data, PACK_PREFIX + 'fff')

def _pack_vec4_list(value):
	return _pack_list([_pack_vec4(object) for object in value])

def _unpack_vec4_list(data):	
	return _unpack_list_nd(data, PACK_PREFIX + 'ffff')

def _pack_entities(value):
	return _pack_list([_pack_entity(object) for object in value])

def _pack_dirty_entities(value):
	return _pack_list([_pack_entity(object) for object in value if object.dirty])

def _unpack_entities(data):
	return _unpack_list(data,_unpack_entity)

def pack_netarg(object,export):
	if export[1] in _type2funcs:
		return _type2funcs[export[1]][0](getattr(object,export[0]))	
	else:
		if isinstance(export[1], str):
			classobj = _name2class[export[1]]
		else:
			classobj = export[1]
		if len(export) >= 4:
			attributes = export[3]
		else:
			attributes = {}
		if attributes.get('islist',False):
			return _pack_list([pack_netlist(element, classobj._tinymarshal_members_) for element in getattr(object,export[0])])
		else:			
			return pack_netlist(getattr(object,export[0]), classobj._tinymarshal_members_)

def pack_netlist(object,exports = None):
	if not exports:
		exports = object._tinymarshal_members_
	return b''.join([pack_netarg(object,export) for export in exports])

def unpack_netarg(data,object,export):
	exportobj = export[1]
	if type(exportobj) == str:
		exportobj = _name2class.get(exportobj, exportobj)
	if exportobj in _type2funcs:
		value,data = _type2funcs[exportobj][1](data)
	else:
		if len(export) < 3:
			unpacktype = exportobj
		else:
			unpacktype = export[2]
			if type(unpacktype) == str:
				unpacktype = _name2class.get(unpacktype, unpacktype)
		if len(export) >= 4:
			attributes = export[3]
		else:
			attributes = {}
		if attributes.get('islist',False):
			if not hasattr(exportobj, '_tinymarshal_members_'):
				raise UnpackException("type %r has not member: _tinymarshal_members_ (trying to parse %r)" % (exportobj, export))
			value,data = _unpack_list(data, unpack_netlist_from_class, unpacktype, exportobj._tinymarshal_members_)
		else:
			if not hasattr(exportobj, '_tinymarshal_members_'):
				raise UnpackException("type %r has not member: _tinymarshal_members_ (trying to parse %r)" % (exportobj, export))
			value,data = unpack_netlist(data, unpacktype(), exportobj._tinymarshal_members_)
	setattr(object,export[0],value)
	return data


def unpack_netlist_from_class(data,classtype,exports = None):
	try:
		return unpack_netlist(data,classtype(),exports)
	except TypeError:
		import traceback
		traceback.print_exc()
		raise UnpackException(classtype, exports)

def unpack_netlist(data,node,exports = None):
	if not exports:
		exports = node._tinymarshal_members_	
	for export in exports:
		data = unpack_netarg(data,node,export)
	return node,data

_type2funcs = {
int : (_pack_int, _unpack_int),
bool : (_pack_int, _unpack_int),
str : (_pack_str, _unpack_str),
strlist : (_pack_str_list, _unpack_str_list),
intlist : (_pack_int_list, _unpack_int_list),
uintlist : (_pack_uint_list, _unpack_uint_list),
int3list : (_pack_int3_list, _unpack_int3_list),
vec2list : (_pack_vec2_list, _unpack_vec2_list),
vec3list : (_pack_vec3_list, _unpack_vec3_list),
vec4list : (_pack_vec4_list, _unpack_vec4_list),
short2list : (_pack_short2_list, _unpack_short2_list),
shortlist : (_pack_short_list, _unpack_short_list),
ushort2list : (_pack_ushort2_list, _unpack_ushort2_list),
short3list : (_pack_short3_list, _unpack_short3_list),
uint : (_pack_uint, _unpack_uint),
vec3 : (_pack_vec3, _unpack_vec3),
vec3p : (_pack_vec3_or_none, _unpack_vec3_or_none),
vec4 : (_pack_vec4, _unpack_vec4),
vec2 : (_pack_vec2, _unpack_vec2),
float : (_pack_float, _unpack_float),
}

_name2class = {
}

def register(name, classobj):
	_name2class[name] = classobj

def pack(object,exports = None):
	return pack_netlist(object,exports)
	
def unpack(data,node,exports = None):
	return unpack_netlist(data,node,exports)

__all__ = [
	'pack',
	'unpack',
	'intlist',
	'uintlist',
	'strlist',
	'int3list',
	'vec2list',
	'vec3list',
	'vec4list',
	'short2list',
	'ushort2list',
	'short3list',
	'shortlist',
	'uint',
	'ushort',
	'vec3',
	'vec4',
	'vec3p',
	'vec2',
	'register',
]

if __name__ == '__main__':
	class TestObj:
		_tinymarshal_members_ = [
			('number', int),
			('text', str),
		]
	# run some tests
	to = TestObj()
	to.number = 5
	to.text = "hello"
	test = pack(to)
	print(repr(test))
	te = unpack(test, TestObj())[0]
	print(te.number)
	print(te.text)
