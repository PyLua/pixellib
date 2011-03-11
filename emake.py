#! /usr/bin/env python
# -*- coding: utf-8 -*-
#======================================================================
#
# emake.py - emake version 2.11
#
# history of this file:
# 2009.08.20   skywind   create this file
# 2009.11.14   skywind   new install() method
# 2009.12.22   skywind   implementation execute interface
# 2010.01.18   skywind   new project info
# 2010.03.14   skywind   fixed source lex bug
# 2010.11.03   skywind   new 'import' to import config section 
# 2010.11.04   skywind   new 'export' to export .def, .lib for windll
# 2010.11.27   skywind   fixed link sequence with -Xlink -( -)
#
#======================================================================
import sys
import os
import ConfigParser


#----------------------------------------------------------------------
# preprocessor: C/C++ Ԥ������
#----------------------------------------------------------------------
class preprocessor(object):

	# ��������ӳ�䣬�������ַ�����ע���� "$"�� "`"���棬�ų���������
	def preprocess (self, text):
		content = text
		spaces = [' ', '\n', '\t', '\r']
		import cStringIO
		srctext = cStringIO.StringIO()
		memo = 0
		i = 0
		while i < len(content):
			if memo == 0:		# ������
				if content[i:i+2] == '/*':
					srctext.write('``')
					i = i + 2
					memo = 1
					continue
				if content[i:i+2] == '//':
					srctext.write('``')
					i = i + 2
					while (i < len(content)) and (content[i] != '\n'):
						if content[i] in spaces:
							srctext.write(content[i])
							i = i + 1
							continue						
						srctext.write('`')
						i = i + 1
					continue
				if content[i] == '\"':
					srctext.write('\"')
					i = i + 1
					memo = 2
					continue
				if content[i] == '\'':
					srctext.write('\'')
					i = i + 1
					memo = 3
					continue
				srctext.write(content[i])
			elif memo == 1:		# ע����
				if content[i:i+2] == '*/':
					srctext.write('``')
					i = i + 2
					memo = 0
					continue
				if content[i] in spaces:
					srctext.write(content[i])
					i = i + 1
					continue
				srctext.write('`')
			elif memo == 2:		# �ַ�����
				if content[i:i+2] == '\\\"':
					srctext.write('$$')
					i = i + 2
					continue
				if content[i:i+2] == '\\\\':
					srctext.write('$$')
					i = i + 2
					continue
				if content[i] == '\"':
					srctext.write('\"')
					i = i + 1
					memo = 0
					continue
				if content[i] in spaces:
					srctext.write(content[i])
					i = i + 1
					continue
				srctext.write('$')
			elif memo == 3:		# �ַ���
				if content[i:i+2] == '\\\'':
					srctext.write('$$')
					i = i + 2
					continue
				if content[i:i+2] == '\\\\':
					srctext.write('$$')
					i = i + 2
					continue
				if content[i] == '\'':
					srctext.write('\'')
					i = i + 1
					memo = 0
					continue
				if content[i] in spaces:
					srctext.write(content[i])
					i = i + 1
					continue
				srctext.write('$')
			i = i + 1
		return srctext.getvalue()

	# ���ҵ�һ�ļ���ͷ�ļ��������
	def search_reference(self, source, heads):
		content = ''
		del heads[:]
		try:
			fp = open(source, "r")
		except:
			return ''
		number = 1

		for line in fp:
			content = content + line
			number = number + 1
		fp.close()

		import cStringIO
		outtext = cStringIO.StringIO()
		for i in xrange(0, len(content)):
			if content[i] != '\r':
				outtext.write(content[i])
		content = outtext.getvalue()

		srctext = self.preprocess(content)
		blank = [ ' ', '\t', '\r', '\n', '/', '*', '$' ]
		space = [ ' ', '\t', '/', '*', '$' ]

		length = len(srctext)
		start = 0
		endup =-1
		number = 0

		while (start >= 0) and (start < length):
			start = endup + 1
			endup = srctext.find('\n', start)
			if (endup < 0):
				endup = length
			number = number + 1

			line = srctext[start:endup]
			offset1 = srctext.find('#', start, endup)
			if offset1 < 0: continue
			offset2 = srctext.find('include', offset1, endup)
			if offset2 < 0: continue
			offset3 = srctext.find('\"', offset2, endup)
			if offset3 < 0: continue
			offset4 = srctext.find('\"', offset3 + 1, endup)
			if offset4 < 0: continue

			check_range = [ i for i in xrange(start, offset1) ]
			check_range += [ i for i in xrange(offset1 + 1, offset2) ]
			check_range += [ i for i in xrange(offset2 + 7, offset3) ]
			check = 1

			for i in check_range:
				if not (srctext[i] in [' ', '`' ]):
					check = 0
			if check != 1:
				continue
			
			name = content[offset3 + 1 : offset4]
			heads.append([name, offset1, offset4, number])

		return content

	# �ϲ����õ�����ͷ�ļ����������ļ����������Ҳ�����ͷ�ļ�
	def parse_source(self, filename, history_headers, lost_headers):
		headers = []
		import cStringIO
		outtext = cStringIO.StringIO()
		if not os.path.exists(filename):
			sys.stderr.write('can not open %s\n'%(filename))
			return outtext.getvalue()
		content = self.search_reference(filename, headers)
		save_cwd = os.getcwd()
		file_cwd = os.path.dirname(filename)
		if file_cwd == '':
			file_cwd = '.'
		os.chdir(file_cwd)
		available = []
		for head in headers:
			if os.path.exists(head[0]):
				available.append(head)
		headers = available
		offset = 0
		for head in headers:
			name = os.path.realpath(head[0])
			name = os.path.normcase(os.path.normpath(name))
			if not (name in history_headers):
				history_headers.append(name)
				position = len(history_headers) - 1
				text = self.parse_source(name, history_headers, lost_headers)
				del history_headers[position]
				history_headers.append(name)
				outtext.write(content[offset:head[1]] + '\n')
				outtext.write('/*:: <%s> ::*/\n'%(head[0]))
				outtext.write(text + '\n/*:: </:%s> ::*/\n'%(head[0]))
				offset = head[2] + 1
			else:
				outtext.write(content[offset:head[1]] + '\n')
				outtext.write('/*:: skip including "%s" ::*/\n'%(head[0]))
				offset = head[2] + 1
		outtext.write(content[offset:])
		os.chdir(save_cwd)
		return outtext.getvalue()

	# ���˴���ע��
	def cleanup_memo(text):
		content = text
		outtext = ''
		srctext = self.preprocess(content)
		space = [ ' ', '\t', '`' ]
		start = 0
		endup = -1
		sized = len(srctext)
		while (start >= 0) and (start < sized):
			start = endup + 1
			endup = srctext.find('\n', start)
			if endup < 0:
				endup = sized
			empty = 1
			memod = 0
			for i in xrange(start, endup):
				if not (srctext[i] in space):
					empty = 0
				if srctext[i] == '`':
					memod = 1
			if empty and memod:
				continue
			for i in xrange(start, endup):
				if srctext[i] != '`':
					outtext = outtext + content[i]
			outtext = outtext + '\n'
		return outtext

	# ֱ�ӷ�������
	def dependence (self, filename):
		head = []
		lost = []
		text = self.parse_source(filename, head, lost)
		return head, lost, text


#----------------------------------------------------------------------
# configure: ȷ��gccλ�ò������ö���Ĭ������
#----------------------------------------------------------------------
class configure(object):

	# ���캯��
	def __init__ (self, ininame = ''):
		exepath = os.path.abspath(os.path.join(os.getcwd(), sys.argv[0]))
		self.dirpath = os.path.join(*os.path.split(exepath)[:-1])
		self.current = os.getcwd()
		if not ininame:
			ininame = 'emake.ini'
		self.ininame = ininame
		self.inipath = os.path.join(self.dirpath, self.ininame)
		self.haveini = False
		self.dirhome = ''
		self.config = {}
		self.cp = ConfigParser.ConfigParser()
		self.unix = 1
		self.xlink = 1
		self.searchdirs = None
		self.environ = {}
		for n in os.environ:
			self.environ[n] = os.environ[n]
		if sys.platform[:3] == 'win':
			self.unix = 0
			self.GetShortPathName = None
		if sys.platform[:6] == 'darwin':
			self.xlink = 0
		self.reset()
	
	# ������Ϣ��λ
	def reset (self):
		self.inc = {}
		self.lib = {}
		self.flag = {}
		self.pdef = {}
		self.link = {}
		self.param_build = ''
		self.param_compile = ''
	
	# ��ʼ�� MSVC
	def _initwin (self):
		msconfig = {}
		if 'msvc' in self.config:
			for n in self.config['msvc']:
				msconfig[n.upper()] = self.config['msvc'][n]
		for n in msconfig:
			msconfig[n] = self._expand(msconfig, self.environ, msconfig[n])
		self.msvc = ''
		env = msconfig.get('PATH', '') + ';' + self.environ.get('PATH', '')
		for path in env.split(';'):
			condition = True
			for name in [ 'cl.exe', 'link.exe', 'lib.exe' ]:
				if not os.path.exists(os.path.join(path, name)):
					condition = False
			if condition and (not self.msvc):
				self.msvc = path
		if self.msvc:
			self.msvc = self.pathshort(self.msvc)
			for n in msconfig:
				v = msconfig[n]
				if not v in os.environ:
					os.environ[n] = v
				else:
					result = {}
					for x in os.environ[n].split(';'):
						result[x.strip('\r\n\t ').upper()] = ''
					for x in v.split(';'):
						x = x.strip('\r\n\t ')
						if not x.upper() in result:
							os.environ[n] += ';' + x
		return self.msvc

	# չ�����ú�
	def _expand (self, section, environ, text):
		if not environ: environ = {}
		if not section: section = {}
		if not '%' in text: return text
		part = text.split('%')
		if len(part) % 2 == 0: return text
		lookup = {}
		for pos in xrange(len(part)):
			if pos % 2 == 1: lookup[part[pos]] = ''
		for name in lookup:
			if name in section:
				value = self._expand(section, environ, section[name])
			elif name in environ:
				value = self._expand(section, environ, environ[name])
			else:
				continue
			text = text.replace('%' + name + '%', value)
		return text
	
	# ȡ�ö��ļ���
	def pathshort (self, path):
		path = os.path.abspath(path)
		if self.unix:
			return path
		if not self.GetShortPathName:
			self.kernel32 = None
			self.textdata = None
			try:
				import ctypes
				self.kernel32 = ctypes.windll.LoadLibrary("kernel32.dll")
				self.textdata = ctypes.create_string_buffer('\000' * 1024)
				self.GetShortPathName = self.kernel32.GetShortPathNameA
				args = [ ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int ]
				self.GetShortPathName.argtypes = args
				self.GetShortPathName.restype = ctypes.c_uint32
			except: pass
		if not self.GetShortPathName:
			return path
		retval = self.GetShortPathName(path, self.textdata, 1024)
		shortpath = self.textdata.value
		if retval <= 0:
			return ''
		return shortpath
	
	# ��ȡini�ļ�
	def _readini (self, inipath):
		self.cp = ConfigParser.ConfigParser()
		if self.unix and '~' in inipath:
			inipath = os.path.expanduser(inipath)
		if os.path.exists(inipath):
			try: self.cp.read(inipath)
			except: pass
			for sect in self.cp.sections():
				for key, val in self.cp.items(sect):
					lowsect, lowkey = sect.lower(), key.lower()
					self.config.setdefault(lowsect, {})[lowkey] = val
			self.haveini = True
		return 0

	# ��ʼ��
	def init (self):
		self.config = {}
		self.reset()
		self._readini(self.inipath)
		if self.unix:
			self._readini('/etc/%s'%self.ininame)
			self._readini('/usr/local/etc/%s'%self.ininame)
			self._readini('~/%s'%self.ininame)
			self._readini('~/.%s'%os.path.splitext(self.ininame)[0])
		self.dirhome = self._getitem('default', 'home', '')
		if not self.haveini:
			#sys.stderr.write('warning: %s cannot be open\n'%(self.ininame))
			sys.stderr.flush()
		p1 = os.path.join(self.dirhome, 'bin/gcc.exe')
		p2 = os.path.join(self.dirhome, 'bin/gcc')
		if (not os.path.exists(p1)) and (not os.path.exists(p1)):
			self.dirhome = ''
		if sys.platform[:3] != 'win':
			if self.dirhome[1:2] == ':':
				self.dirhome = ''
		if not self.dirhome:
			self.dirhome = self.search()
		if not self.dirhome:
			sys.stderr.write('warning: cannot find gcc/mingw home\n')
			sys.stderr.write('warning: place me in the home dir of mingw\n')
			avail = ''
			for path in os.environ.get('PATH', '').split(';'):
				gccpath = os.path.abspath(os.path.join(path, 'gcc.exe'))
				if os.path.exists(gccpath):
					avail = os.path.abspath(os.path.join(path, '..'))		
			if avail:
				sys.stderr.write('warning: get gcc from $PATH env: ')
				sys.stderr.write(avail + '\n')
				self.dirhome = avail
			sys.stderr.flush()
		if not self.dirhome:
			sys.stderr.write('error: cannot find gcc/mingw home\n')
			sys.stderr.write('error: place me in the home dir of mingw\n')
			raise Exception('cannot find gcc/mingw home')
		self.dirhome = os.path.abspath(self.dirhome)
		if len(self.dirhome) > 1:
			if self.dirhome[-1] in ('/', '\\', ':'):
				self.dirhome = self.dirhome[:-1]
		self.refresh()
		if not self.unix:
			self._initwin()
		return 0

	# ��ȡ����
	def _getitem (self, sect, key, default = ''):
		return self.config.get(sect, {}).get(key, default)
	
	# ȡ���滻��$(HOME)������·��
	def path (self, path):
		path = path.replace('$(HOME)', self.dirhome).replace('\\', '/')
		path = self.cygpath(path)
		text = ''
		issep = False
		for n in path:
			if n == '/':
				if issep == False: text += n
				issep = True
			else:
				text += n
				issep = False
		return os.path.abspath(text)
	
	# ȡ�ÿ����ڲ������ı�·��
	def pathtext (self, name):
		name = os.path.normpath(name)
		name = self.cygpath(name)
		name = name.replace('"', '""')
		if ' ' in name:
			return '"%s"'%(name)
		if self.unix:
			name = name.replace('\\', '/')
		return name
	
	# ȡ�ö�·������ǰ·�������·��
	def pathrel (self, name):
		name = os.path.abspath(name)
		if 'relpath' in os.__dict__:
			name = os.path.relpath(name, os.getcwd())
			name = self.pathtext(name)
			return name
		current = os.getcwd().replace('\\', '/')
		if len(current) > 0:
				if current[-1] != '/':
						current += '/'
		name = self.path(name).replace('\\', '/')
		size = len(current)
		if name[:size] == current:
				name = name[size:]
		name = self.pathtext(name)
		return name
	
	# ת����cygwin·��
	def cygpath (self, path):
		if self.unix and path[1:2] == ':':
			path = '/cygdrive/%s%s'%(path[0], path[2:].replace('\\', '/'))
		return path

	# ���ͷ�ļ�Ŀ¼
	def push_inc (self, inc):
		path = self.path(inc)
		if not os.path.exists(path):
			#sys.stderr.write('warning: cannot open %s\n'%path)
			return -1
		if self.unix and path[:10] == '/cygdrive/':
			#sys.stderr.write('warning: ignore path %s\n'%path)
			return -2
		path = self.pathtext(path)
		self.inc[path] = 1
		return 0
	
	# ��ӿ��ļ�Ŀ¼
	def push_lib (self, lib):
		path = self.path(lib)
		if not os.path.exists(path):
			#sys.stderr.write('warning: cannot open %s\n'%path)
			return -1
		if self.unix and path[:10] == '/cygdrive/':
			#sys.stderr.write('warning: ignore path %s\n'%path)
			return -2
		path = self.pathtext(path)
		self.lib[path] = 1
		return 0
	
	# ��Ӳ���
	def push_flag (self, flag):
		self.flag[flag] = 1
	
	# ������ӿ�
	def push_link (self, link):
		if link[-2:].lower() == '.o':
			link = self.pathtext(self.path(link))
		else:
			link = '-l%s'%link.replace(' ', '_')
		self.link[link] = 1
		return 0
	
	# ���Ԥ����
	def push_pdef (self, define):
		self.pdef[define] = 1

	# ����gcc
	def search (self):
		dirpath = self.dirpath
		if sys.platform[:3] == 'win':
			if os.path.exists(os.path.join(dirpath, 'gcc.exe')):
				return os.path.abspath(os.path.join(dirpath, '..'))
			if os.path.exists(os.path.join(dirpath, 'bin/gcc.exe')):
				return os.path.abspath(os.path.join(dirpath, '.'))
			if os.path.exists(os.path.join(dirpath, '../bin/gcc.exe')):
				return os.path.abspath(os.path.join(dirpath, '..'))
		else:
			if os.path.exists('/bin/gcc'):
				return '/'
			if os.path.exists('/usr/bin/gcc'):
				return '/usr'
			if os.path.exists('/usr/local/bin/gcc'):
				return '/usr/local'
		return ''
	
	# дĬ�ϵ������ļ�
	def _write_default_ini (self):
		default = '''	[default]
						include=$(HOME)/include
						lib=$(HOME)/lib
				'''
		text = '\n'.join([ n.strip('\t\r\n ') for n in default.split('\n') ])
		if os.path.exists(self.inipath):
			return -1
		fp = open(self.inipath, 'w')
		fp.write(text)
		fp.close()
		return 0
	
	# ����·��
	def pathconf (self, path):
		path = path.strip(' \t')
		if path[:1] == '\'' and path[-1:] == '\'': path = path[1:-1]
		if path[:1] == '\"' and path[-1:] == '\"': path = path[1:-1]
		return path

	# ˢ������
	def refresh (self, sect = 'default'):
		f1 = lambda n: (n[:1] != '\'' or n[-1:] != '\'') and n
		config = lambda n: self._getitem(sect, n, '')
		for path in config('include').replace(';', ',').split(','):
			path = self.pathconf(path)
			if not path: continue
			self.push_inc(path)
		for path in config('lib').replace(';', ',').split(','):
			path = self.pathconf(path)
			if not path: continue
			self.push_lib(path)
		for link in config('link').replace(';', ',').split(','):
			link = self.pathconf(link)
			if not link: continue
			self.push_link(link)
		for flag in config('flag').replace(':', ',').split(','):
			flag = flag.strip(' \t\r\n')
			self.push_flag(flag)
		for pdef in config('define').replace(';', ',').split(','):
			pdef = pdef.strip(' \t\r\n')
			if not pdef: continue
			self.push_pdef(pdef.replace(' ', '_'))
		self.parameters()
		return 0

	# �������л��Ĳ���	
	def parameters (self):
		text = ''
		for inc in self.inc:
			text += '-I%s '%inc
		for lib in self.lib:
			text += '-L%s '%lib
		for flag in self.flag:
			text += '%s '%flag
		for pdef in self.pdef:
			text += '-D%s '%pdef
		self.param_compile = text.strip(' ')
		text = ''
		if self.xlink:
			text = '-Xlinker "-(" '
		for link in self.link:
			text += '%s '%link
		if self.xlink:
			text += ' -Xlinker "-)"'
		else:
			text = text + ' ' + text
		self.param_build = self.param_compile + ' ' + text
		return text

	# gcc ��search-dirs
	def __searchdirs (self):
		if self.searchdirs != None:
			return self.searchdirs
		path = os.path.abspath(os.path.join(self.dirhome, 'bin/gcc'))
		if not self.unix:
			name = self.pathshort(path)
			if (not name) and os.path.exists(path + '.exe'):
				name = self.pathshort(path + '.exe')
			if name: path = name
		cmdline = path + ' -print-search-dirs'
		fp = os.popen(cmdline, 'r')
		data = fp.read()
		fp.close()
		fp = None
		body = ''
		for line in data.split('\n'):
			if line[:10] == 'libraries:':
				body = line[10:].strip('\r\n ')
				if body[:1] == '=': body = body[1:]
				break
		part = []
		if sys.platform[:3] == 'win': part = body.split(';')
		else: part = body.split(':')
		data = []
		dict = {}
		for n in part:
			path = os.path.abspath(os.path.normpath(n))
			if not path in dict:
				if os.path.exists(path):
					data.append(path)
					dict[path] = 1
				else:
					dict[path] = 0
		self.searchdirs = data
		return data
	
	# �����Ƿ����
	def checklib (self, name):
		name = 'lib' + name + '.a'
		for n in self.__searchdirs():
			if os.path.exists(os.path.join(n, name)):
				return True
		for n in self.lib:
			if os.path.exists(os.path.join(n, name)):
				return True
		return False
	
	# ִ��GNU���߼�
	def execute (self, binname, parameters, printcmd = False):
		path = os.path.abspath(os.path.join(self.dirhome, 'bin', binname))
		if not self.unix:
			name = self.pathshort(path)
			if (not name) and os.path.exists(path + '.exe'):
				name = self.pathshort(path + '.exe')
			if name: path = name
		cmd = '%s %s'%(self.pathtext(path), parameters)
		#printcmd = True
		if printcmd:
			print '>', cmd
		sys.stdout.flush()
		sys.stderr.flush()
		os.system(cmd)

	# ���� gcc
	def gcc (self, parameters, needlink, printcmd = False):
		param = self.param_build
		if not needlink:
			param = self.param_compile
		parameters = '%s %s'%(parameters, param)
		self.execute('gcc', parameters, printcmd)

	# ����
	def compile (self, srcname, objname, printcmd = False):
		cmd = '-c %s -o %s'%(self.pathrel(srcname), self.pathrel(objname))
		self.gcc(cmd, False, printcmd)
	
	# ʹ�� dllwrap
	def dllwrap (self, parameters, printcmd = False):
		text = ''
		for lib in self.lib:
			text += '-L%s '%lib
		for link in self.link:
			text += '%s '%link
		parameters = '%s %s'%(parameters, text)
		self.execute('dllwrap', parameters, printcmd)
	
	# ����lib��
	def makelib (self, output, objs = [], printcmd = False):
		name = ' '.join([ self.pathrel(n) for n in objs ])
		parameters = 'crv %s %s'%(self.pathrel(output), name)
		self.execute('ar', parameters, printcmd)
	
	# ���ɶ�̬���ӣ�dll ���� so
	def makedll (self, output, objs = [], param = '', printcmd = False):
		if (not param) or (self.unix):
			param = '--shared -fpic'
			self.makeexe(output, objs, param, printcmd)
		else:
			name = ' '.join([ self.pathrel(n) for n in objs ])
			parameters = '%s -o %s %s'%(param, 
				self.pathrel(output), name)
			self.dllwrap(parameters, printcmd)
	
	# ����exe
	def makeexe (self, output, objs = [], param = '', printcmd = False):
		name = ' '.join([ self.pathrel(n) for n in objs ])
		if self.xlink:
			name = '-Xlinker "-(" ' + name + ' -Xlinker "-)"'
		parameters = '-o %s %s %s'%(self.pathrel(output), param, name)
		self.gcc(parameters, True, printcmd)

	# ����VC����
	def msvcexe (self, binname, parameters, printcmd = False):
		if self.unix:
			return -1
		if not self.msvc:
			msg = '%s: error: can not find msvc environment !!'%binname
			msg = msg + ' run vcvars32.bat first !!\n'
			sys.stderr.write(msg)
			sys.stderr.flush()
			return -2
		path = os.path.join(self.msvc, binname)
		cmd = '%s %s'%(path, parameters)
		if printcmd:
			print '>', cmd
		sys.stdout.flush()
		sys.stderr.flush()
		os.system(cmd)
		return 0



#----------------------------------------------------------------------
# coremake: ���Ĺ��̱��룬�ṩ Compile/Link/Build
#----------------------------------------------------------------------
class coremake(object):
	
	# ���캯��
	def __init__ (self, ininame = ''):
		self.ininame = ininame
		self.config = configure(self.ininame)
		self.unix = self.config.unix
		self.inited = 0
		self.reset()
	
	# ��λ����
	def reset (self):
		self.config.reset()
		self._out = ''		# ����������ļ�������abc.exe
		self._int = ''		# �м��ļ���Ŀ¼
		self._mode = 'exe'	# exe win dll lib
		self._src = []		# Դ����
		self._obj = []		# Ŀ���ļ�
		self._export = {}	# DLL��������
		
	# ��ʼ�������ù������֣����ͣ��Լ��м��ļ���Ŀ¼
	def init (self, out = 'a.out', mode = 'exe', intermediate = ''):
		if not mode in ('exe', 'win', 'dll', 'lib'):
			raise Exception("mode must in ('exe', 'win', 'dll', 'lib')")
		self.reset()
		if not self.inited:
			self.config.init()
			self.inited = 1
		self._mode = mode
		self._out = os.path.abspath(out)
		self._int = intermediate
		self._out = self.outname(self._out, mode)
	
	# ȡ��Դ�ļ���Ӧ��Ŀ���ļ�������Դ�ļ������м��ļ�Ŀ¼��
	def objname (self, srcname, intermediate = ''):
		part = os.path.splitext(srcname)
		ext = part[1].lower()
		if ext in ('.c', '.cpp', '.c', '.cc', '.cxx', '.s', '.asm'):
			if intermediate:
				name = os.path.join(intermediate, os.path.split(part[0])[-1])
				name = os.path.abspath(name + '.o')
			else:
				name = os.path.abspath(part[0] + '.o')
			return name
		if not ext in ('.o', '.obj'):
			raise Exception('unknow ext-type of %s\n'%srcname)
		return srcname
	
	# ȡ������ļ����ļ���
	def outname (self, output, mode = 'exe'):
		if not mode in ('exe', 'win', 'dll', 'lib'):
			raise Exception("mode must in ('exe', 'win', 'dll', 'lib')")
		part = os.path.splitext(os.path.abspath(output))
		output = part[0]
		if mode == 'exe':
			if self.unix == 0 and part[1] == '':
				output += '.exe'
			elif part[1]:
				output += part[1]
		elif mode == 'win':
			if self.unix == 0 and part[1] == '':
				output += '.exe'
			elif part[1]:
				output += part[1]
		elif mode == 'dll':
			if not part[1]: 
				if not self.unix: output += '.dll'
				else: output += '.so'
			else:
				output += part[1]
		elif mode == 'lib':
			if not part[1]: output += '.a'
			else: output += part[1]
		return output
	
	# ɨ��Ŀ���ļ�
	def scan (self):
		self._obj = [ self.objname(n, self._int) for n in self._src ]
		return 0
	
	# ���Դ�ļ�
	def push (self, srcname):
		self._src.append(os.path.abspath(srcname))
	
	# ����Ŀ¼
	def mkdir (self, path):
		if os.path.exists(path):
			return 0
		name = ''
		part = os.path.abspath(path).replace('\\', '/').split('/')
		if self.unix:
			name = '/'
		if (not self.unix) and (path[1:2] == ':'):
			part[0] += '/'
		for n in part:
			name = os.path.abspath(os.path.join(name, n))
			if not os.path.exists(name):
				os.mkdir(name)
		return 0
	
	# ɾ��Ŀ¼
	def remove (self, path):
		try: os.remove(path)
		except: pass
		return 0
	
	# DLL����
	def dllwrap (self, name):
		if sys.platform[:3] != 'win':
			return -1
		if self._mode != 'dll':
			return -2
		name = name.lower()
		main = os.path.splitext(os.path.abspath(self._out))[0]
		main = os.path.split(main)[-1]
		main = os.path.abspath(os.path.join(self._int, main))
		if name == 'def':
			self._export['def'] = main + '.def'
		elif name == 'lib':
			self._export['lib'] = main + '.a'
		elif name in ('hidden', 'hide', 'none'):
			self._export['hide'] = 1
		elif name in ('msvc', 'MSVC'):
			self._export['def'] = main + '.def'
			self._export['msvc'] = main + '.lib'
			self._export['msvc64'] = 0
		elif name in ('msvc64', 'MSVC64'):
			self._export['def'] = main + '.def'
			self._export['msvc'] = main + '.lib'
			self._export['msvc64'] = 1
		return 0
	
	# DLL export�Ĳ���
	def _dllparam (self):
		defname = self._export.get('def', '')
		libname = self._export.get('lib', '')
		msvclib = self._export.get('msvc', '')
		hidden = self._export.get('hide', 0)
		if (not defname) and (not libname):
			return ''
		param = ''
		if not hidden: param += '--export-all '
		if defname:
			param += '--output-def %s '%self.config.pathrel(defname)
		if libname:
			param += '--implib %s '%self.config.pathrel(libname)
		return param
	
	# DLL ������ɺ������
	def _dllpost (self):
		defname = self._export.get('def', '')
		libname = self._export.get('lib', '')
		msvclib = self._export.get('msvc', '')
		dllname = self._out
		if not msvclib:
			return 0
		if not os.path.exists(defname):
			return -1
		machine = '/machine:i386'
		msvc64 = self._export.get('msvc64', 0)
		if msvc64:
			machine = '/machine:x64'
		defname = self.config.pathtext(self.config.pathrel(defname))
		msvclib = self.config.pathtext(self.config.pathrel(msvclib))
		parameters = '-nologo ' + machine + ' /def:' + defname
		parameters += ' /out:' + msvclib
		self.config.msvcexe('LIB.EXE', parameters, False)
		return 0
	
	# ���룺skipexist(�Ƿ���Ҫ�������е�obj�ļ�)
	def compile (self, skipexist = False, printmode = 0):
		self.scan()
		self.mkdir(os.path.abspath(self._int))
		retval = 0
		printcmd = False
		if printmode & 4:
			printcmd = True
		if printmode & 2:
			print 'compiling ...'
		for i in xrange(len(self._src)):
			srcname = self._src[i]
			objname = self._obj[i]
			if srcname == objname:
				continue
			if skipexist and os.path.exists(objname):
				continue
			try: os.remove(os.path.abspath(objname))
			except: pass
			if printmode & 1:
				name = self.config.pathrel(srcname)
				if name[:1] == '"':
					name = name[1:-1]
				print name
			self.config.compile(srcname, objname, printcmd)
			if not os.path.exists(objname):
				retval = -1
				break
		return retval
	
	# ���ӣ�(�Ƿ��������е��ļ�)
	def link (self, skipexist = False, printmode = 0):
		retval = 0
		self.scan()
		printcmd = False
		if printmode & 4:
			printcmd = True
		if printmode & 2:
			print 'linking ...'
		output = self._out
		if skipexist and os.path.exists(output):
			return 0
		self.remove(output)
		self.mkdir(os.path.split(output)[0])
		if self._mode == 'exe':
			self.config.makeexe(output, self._obj, '', printcmd)
		elif self._mode == 'win':
			param = '-mwindows'
			self.config.makeexe(output, self._obj, param, printcmd)
		elif self._mode == 'dll':
			param = self._dllparam()
			self.config.makedll(output, self._obj, param, printcmd)
			if param and os.path.exists(output): 
				self._dllpost()
		elif self._mode == 'lib':
			self.config.makelib(output, self._obj, printcmd)
		if not os.path.exists(output):
			return ''
		return output
	
	# ����������
	def build (self, skipexist = False, printmode = 0):
		if self.compile(skipexist, printmode) != 0:
			return -1
		output = self.link(skipexist, printmode)
		if output == '':
			return -2
		return output



#----------------------------------------------------------------------
# iparser: ���̷���������������������Ϣ
#----------------------------------------------------------------------
class iparser (object):
	
	# ���캯��
	def __init__ (self, ininame = ''):
		self.preprocessor = preprocessor()
		self.coremake = coremake(ininame)
		self.reset()

	# ���ø�λ
	def reset (self):
		self.src = []
		self.inc = []
		self.lib = []
		self.imp = []
		self.exp = []
		self.link = []
		self.flag = []
		self.mode = 'exe'
		self.define = {}
		self.name = ''
		self.home = ''
		self.info = 3
		self.out = ''
		self.int = ''
		self.incdict = {}
		self.libdict = {}
		self.srcdict = {}
		self.impdict = {}
		self.expdict = {}
		self.linkdict = {}
		self.flagdict = {}
		self.mainfile = ''
		self.makefile = ''
	
	# ȡ���ļ���Ŀ���ļ�����
	def __getitem__ (self, key):
		return self.srcdict[key]
	
	# ȡ��ģ�����
	def __len__ (self):
		return len(self.srcdict)
	
	# ����Ƿ����ģ��
	def __contains__ (self, key):
		return (key in self.srcdict)
	
	# ȡ�õ�����
	def __iter__ (self):
		return self.srcdict.__iter__()
	
	# ��Ӵ���
	def push_src (self, filename):
		filename = os.path.abspath(filename)
		if filename in self.srcdict:
			return -1
		self.srcdict[filename] = ''
		self.src.append(filename)
		return 0
	
	# �������
	def push_link (self, linkname):
		if linkname in self.linkdict:
			return -1
		self.linkdict[linkname] = len(self.link)
		self.link.append(linkname)
		return 0
	
	# ���ͷ·��
	def push_inc (self, inc):
		if inc in self.incdict:
			return -1
		self.incdict[inc] = len(self.inc)
		self.inc.append(inc)
		return 0

	# ��ӿ�·��
	def push_lib (self, lib):
		if lib in self.libdict:
			return -1
		self.libdict[lib] = len(self.lib)
		self.lib.append(lib)
		return 0
	
	# ��Ӳ���
	def push_flag (self, flag):
		if flag in self.flagdict:
			return -1
		self.flagdict[flag] = len(self.flag)
		self.flag.append(flag)
		return 0
	
	# ��Ӻ궨��
	def push_define (self, define, value = 1):
		self.define[define] = value
		return 0
	
	# ��ӵ�������
	def push_imp (self, name, fname = '', lineno = -1):
		if name in self.impdict:
			return -1
		self.impdict[name] = len(self.imp)
		self.imp.append((name, fname, lineno))
		return 0
	
	# ����������
	def push_exp (self, name, fname = '', lineno = -1):
		if name in self.expdict:
			return -1
		self.expdict[name] = len(self.exp)
		self.exp.append((name, fname, lineno))
	
	# ������ʼ
	def parse (self, mainfile):
		self.reset()
		mainfile = os.path.abspath(mainfile)
		makefile = os.path.splitext(mainfile)[0] + '.mak'
		part = os.path.split(mainfile)
		self.home = part[0]
		self.name = os.path.splitext(part[1])[0]
		if not (os.path.exists(makefile) or os.path.exists(mainfile)):
			sys.stderr.write('error: %s cannot be open'%(mainfile))
			sys.stderr.flush()
			return -1
		if os.path.splitext(mainfile)[1].lower() == '.mak':
			mainfile = ''
		if os.path.exists(makefile) and makefile:
			self.makefile = makefile
			if self.scan_makefile() != 0:
				return -2
		if os.path.exists(mainfile) and mainfile:
			self.mainfile = mainfile
			if self.scan_mainfile() != 0:
				return -3
		if not self.out:
			self.out = os.path.splitext(makefile)[0]
		self.out = self.coremake.outname(self.out, self.mode)
		self._scan_obj()
		return 0
	
	# ȡ�����·��
	def pathrel (self, name, current = ''):
		if not current:
			current = os.getcwd()
		current = current.replace('\\', '/')
		if len(current) > 0:
			if current[-1] != '/':
				current += '/'
		name = self.path(name).replace('\\', '/')
		size = len(current)
		if name[:size] == current:
			name = name[size:]
		return name

	# ����·��
	def pathconf (self, path):
		path = path.strip(' \r\n\t')
		if path[:1] == '\'' and path[-1:] == '\'': path = path[1:-1]
		if path[:1] == '\"' and path[-1:] == '\"': path = path[1:-1]
		return path
	
	# ɨ������� �ؼ�ע�͵Ĺ�����Ϣ
	def _scan_memo (self, filename, prefix = '!'):
		command = []
		content = open(filename, 'U').read()
		srctext = self.preprocessor.preprocess(content)
		srcline = [ 0 for i in xrange(len(srctext)) ]
		length = len(srctext)
		lineno = 1
		for i in xrange(len(srctext)):
			srcline[i] = lineno
			if srctext[i] == '\n':
				lineno += 1
		start = 0
		endup = 0
		while (start >= 0) and (start < length):
			start = endup
			endup = srctext.find('`', start)
			if endup < 0:
				break
			start = endup
			head = content[start:start + 2]
			body = ''
			if head == '//':
				endup = srctext.find('\n', start)
				if endup < 0: endup = length
				body = content[start + 2:endup]
				endup += 1
			elif head == '/*':
				endup = content.find('*/', start)
				if endup < 0: endup = length
				body = content[start + 2:endup]
				endup += 2
			else:
				Exception ('error comment')
			if body[:len(prefix)] != prefix:
				continue
			pos = start + 2 + len(prefix)
			body = body[len(prefix):]
			if pos >= length: pos = length - 1
			lineno = srcline[pos]
			for n in body.split('\n'):
				command.append((lineno, n.strip('\r\n').strip(' \t')))
				lineno += 1
		return command
	
	# ɨ�����ļ�
	def scan_mainfile (self):
		command = self._scan_memo(self.mainfile)
		savedir = os.getcwd()
		os.chdir(os.path.split(self.mainfile)[0])
		retval = 0
		for lineno, text in command:
			if self._process(self.mainfile, lineno, text) != 0:
				retval = -1
				break
		os.chdir(savedir)
		self.push_src(self.mainfile)
		return retval

	# ɨ�蹤���ļ�
	def scan_makefile (self):
		savedir = os.getcwd()
		os.chdir(os.path.split(self.makefile)[0])
		lineno = 1
		retval = 0
		for text in open(self.makefile, 'U'):
			if self._process(self.makefile, lineno, text) != 0:
				retval = -1
				break
			lineno += 1
		os.chdir(savedir)
		return retval
	
	# �������
	def error (self, text, fname = '', line = -1):
		message = ''
		if fname and line > 0:
			message = '%s:%d: '%(fname, line)
		sys.stderr.write(message + text + '\n')
		sys.stderr.flush()
		return 0
	
	# ����Դ�ļ�
	def _process_src (self, textline, fname = '', lineno = -1):
		for name in textline.replace(';', ',').split(','):
			srcname = self.pathconf(name)
			if not srcname:
				continue
			absname = os.path.abspath(srcname)
			if not os.path.exists(absname):
				self.error('error: %s: No such file'%srcname, \
					fname, lineno)
				return -1
			extname = os.path.splitext(absname)[1].lower()
			if not extname in ('.c', '.cpp', '.cc', '.cxx', '.asm', '.s'):
				self.error('error: %s: Unknow file type'%absname)
				return -2
			self.push_src(absname)
		return 0

	# ����������Ϣ
	def _process (self, fname, lineno, text):
		text = text.strip(' \t\r\n')
		if not text:					# ����
			return 0
		if text[:1] in (';', '#'):		# ����ע��
			return 0
		pos = text.find(':')
		if pos < 0:
			self.error('unknow emake command', fname, lineno)
			return -1
		command, body = text[:pos].lower(), text[pos + 1:]
		if command in ('out', 'output'):
			self.out = os.path.abspath(self.pathconf(body))
			return 0
		if command in ('int', 'intermediate'):
			self.int = os.path.abspath(self.pathconf(body))
			return 0
		if command in ('src', 'source'):
			retval = self._process_src(body, fname, lineno)
			return retval
		if command in ('mode', 'mod'):
			body = body.lower().strip(' \r\n\t')
			if not body in ('exe', 'win', 'lib', 'dll'):
				self.error('error: %s: mode is not supported'%body, \
					fname, lineno)
				return -1
			self.mode = body
			return 0
		if command == 'link':
			for name in body.replace(';', ',').split(','):
				srcname = self.pathconf(name)
				if not srcname:
					continue
				self.push_link(srcname)
			return 0
		if command in ('inc', 'lib'):
			for name in body.replace(';', ',').split(','):
				srcname = self.pathconf(name)
				if not srcname:
					continue
				absname = os.path.abspath(srcname)
				if not os.path.exists(absname):
					self.error('error: %s: No such directory'%srcname, \
						fname, lineno)
					return -1
				if command == 'inc': 
					self.push_inc(absname)
				elif command == 'lib':
					self.push_lib(absname)
			return 0
		if command == 'flag':
			for name in body.replace(';', ',').split(','):
				srcname = self.pathconf(name)
				if not srcname:
					continue
				if srcname[:2] in ('-o', '-I', '-B', '-L'):
					self.error('error: %s: invalid option'%srcname, \
						fname, lineno)
				self.push_flag(srcname)
			return 0
		if command == 'define':
			for name in body.replace(';', ',').split(','):
				srcname = self.pathconf(name).replace(' ', '_')
				if not srcname:
					continue
				self.push_define(srcname)
			return 0
		if command == 'info':
			body = body.strip(' \t\r\n').lower()
			if body in ('0', 'false', 'off'):
				self.info = 0
			else:
				try: info = int(body)
				except: info = 3
				self.info = info
			return 0
		if command in ('cexe', 'clib', 'cdll' ,'cwin', 'exe', 'dll', 'win'):
			if not self.int:
				self.int = os.path.abspath('obj')
			self.mode = command[-3:]
			retval = self._process_src(body, fname, lineno)
			return retval
		if command in ('imp', 'import'):
			for name in body.replace(';', ',').split(','):
				name = self.pathconf(name)
				if not name:
					continue
				self.push_imp(name, fname, lineno)
			return 0
		if command in ('exp', 'export'):
			self.dllexp = 'yes'
			for name in body.replace(';', ',').split(','):
				name = self.pathconf(name).lower()
				if not name:
					continue
				self.push_exp(name, fname, lineno)
			return 0
		self.error('error: %s: invalid command'%command, fname, lineno)
		return -1
	
	# ɨ�貢ȷ��Ŀ���ļ�
	def _scan_obj (self):
		for fn in self.src:
			obj = self.coremake.objname(fn, self.int)
			self.srcdict[fn] = os.path.abspath(obj)
		return 0


#----------------------------------------------------------------------
# dependence: ���̱��룬Compile/Link/Build
#----------------------------------------------------------------------
class dependence (object):
	
	def __init__ (self, parser = None):
		self.parser = parser
		self.preprocessor = preprocessor()
		self.reset()
	
	def reset (self):
		self._mtime = {}
		self._dirty = {}
		self._depinfo = {}
		self._depname = ''
		self._outchg = False
	
	def mtime (self, fname):
		fname = os.path.abspath(fname)
		if fname in self._mtime:
			return self._mtime[fname]
		try: mtime = os.path.getmtime(fname)
		except: mtime = 0.0
		self._mtime[fname] = mtime
		return mtime
	
	def _scan_src (self, srcname):
		srcname = os.path.abspath(srcname)
		if not srcname in self.parser:
			return None
		if not os.path.exists(srcname):
			return None
		objname = self.parser[srcname]
		head, lost, src = self.preprocessor.dependence(srcname)
		filelist = [srcname] + head
		dependence = []
		for fn in filelist:
			name = os.path.abspath(fn)
			dependence.append((name, self.mtime(name)))
		return dependence
	
	def _update_dep (self, srcname):
		srcname = os.path.abspath(srcname)
		if not srcname in self.parser:
			return -1
		retval = 0
		debug = 0
		if debug: print '\n<dep:%s>'%srcname
		objname = self.parser[srcname]
		srctime = self.mtime(srcname)
		objtime = self.mtime(objname)
		update = False
		info = self._depinfo.setdefault(srcname, {})
		if len(info) == 0: 
			update = True
		if not update:
			for fn in info:
				if not os.path.exists(fn):
					update = True
					break
				oldtime = info[fn]
				newtime = self.mtime(fn)
				if newtime > oldtime:
					update = True
					break
		if update:
			dependence = self._scan_src(srcname)
			info = {}
			self._depinfo[srcname] = info
			if not dependence:
				return -2
			for fname, mtime in dependence:
				info[fname] = mtime
		info = self._depinfo[srcname]
		for fn in info:
			oldtime = info[fn]
			if oldtime > objtime:
				self._dirty[srcname] = 1
				retval = 1
				break
		if debug: print '</dep:%s>\n'%srcname
		return retval
	
	def _load_dep (self):
		lineno = -1
		retval = 0
		if os.path.exists(self._depname):
			for line in open(self._depname, 'U'):
				line = line.strip(' \t\r\n')
				if not line: continue
				pos = line.find('=')
				if pos < 0: continue
				src, body = line[:pos], line[pos + 1:]
				src = os.path.abspath(src)
				if not os.path.exists(src): continue
				item = body.replace(';', ',').split(',')
				count = len(item) / 2
				info = {}
				self._depinfo[src] = info
				for i in xrange(count):
					fname = item[i * 2 + 0].strip(' \r\n\t')
					mtime = item[i * 2 + 1].strip(' \r\n\t')
					fname = self.parser.pathconf(fname)
					info[fname] = float(mtime)
			retval = 0
		for fn in self.parser:
			self._update_dep(fn)
		return retval

	def _save_dep (self):
		path = os.path.split(self._depname)[0]
		if not os.path.exists(path):
			self.parser.coremake.mkdir(path)
		fp = open(self._depname, 'w')
		names = self._depinfo.keys()
		names.sort()
		for src in names:
			info = self._depinfo[src]
			fp.write('%s = '%(src))
			part = []
			keys = info.keys()
			keys.sort()
			for fname in keys:
				mtime = info[fname]
				if ' ' in fname: fname = '"%s"'%fname
				part.append('%s, %s'%(fname, mtime))
			fp.write(', '.join(part) + '\n')
		fp.close()
		return 0
	
	def process (self):
		self.reset()
		parser = self.parser
		depname = parser.name + '.p'
		self._depname = os.path.join(parser.home, depname)
		if parser.int:
			self._depname = os.path.join(parser.int, depname)
		self._depname = os.path.abspath(self._depname)
		self._load_dep()
		self._save_dep()
		for info in self._depinfo:
			dirty = (info in self._dirty) and 1 or 0
			#print info, '=', dirty
		return 0


#----------------------------------------------------------------------
# emake: ���̱��룬Compile/Link/Build
#----------------------------------------------------------------------
class emake (object):
	
	def __init__ (self, ininame = ''):
		if ininame == '': ininame = 'emake.ini'
		self.parser = iparser(ininame)
		self.coremake = self.parser.coremake
		self.dependence = dependence(self.parser)
		self.config = self.coremake.config
		self.unix = self.coremake.unix
		self.loaded = 0
	
	def reset (self):
		self.parser.reset()
		self.coremake.reset()
		self.dependence.reset()
		self.loaded = 0
	
	def open (self, mainfile):
		self.reset()
		retval = self.parser.parse(mainfile)
		if retval != 0:
			return -1
		parser = self.parser
		self.coremake.init(parser.out, parser.mode, parser.int)
		#print 'open', parser.out, parser.mode, parser.int
		for src in self.parser:
			self.coremake.push(src)
		if self._config() != 0:
			return -2
		self.dependence.process()
		self.loaded = 1
		return 0
	
	def _config (self):
		for inc in self.parser.inc:
			self.config.push_inc(inc)
			#print 'inc', inc
		for lib in self.parser.lib:
			self.config.push_lib(lib)
			#print 'lib', lib
		for flag in self.parser.flag:
			self.config.push_flag(flag)
			#print 'flag', flag
		for link in self.parser.link:
			self.config.push_link(link)
			#print 'link', link
		for pdef in self.parser.define:
			self.config.push_pdef(pdef)
			#print 'pdef', pdef
		for name, fname, lineno in self.parser.imp:
			if not name in self.config.config:
				self.parser.error('error: %s: No such config section'%name, \
					fname, lineno)
				return -1
			self.config.refresh(name)
		for name, fname, lineno in self.parser.exp:
			self.coremake.dllwrap(name)
		self.config.parameters()
		return 0
	
	def compile (self, printmode = -1):
		if not self.loaded:
			return -10
		dirty = 0
		for src in self.parser:
			if src in self.dependence._dirty:
				obj = self.parser[src]
				self.coremake.remove(obj)
				dirty += 1
		if dirty:
			self.coremake.remove(self.parser.out)
		retval = self.coremake.compile(True, self.parser.info)
		return retval
	
	def link (self, printmode = -1):
		if not self.loaded:
			return ''
		update = False
		outname = self.parser.out
		outtime = self.dependence.mtime(outname)
		for src in self.parser:
			obj = self.parser[src]
			mtime = self.dependence.mtime(obj)
			if mtime == 0 or mtime > outtime:
				update = True
				break
		if update:
			self.coremake.remove(self.parser.out)
		retval = self.coremake.link(True, self.parser.info)
		return retval
	
	def build (self, printmode = -1):
		retval = self.compile(printmode)
		if retval != 0:
			return retval
		retval = self.link(printmode)
		if not retval:
			return -20
		return 0
	
	def clean (self):
		for src in self.parser:
			obj = self.parser[src]
			self.coremake.remove(obj)
		if self.loaded:
			self.coremake.remove(self.parser.out)
		return 0
	
	def rebuild (self, printmode = -1):
		self.clean()
		return self.build(printmode)

	def info (self, name = ''):
		name = name.lower()
		if name == '': name = 'out'
		if name in ('out', 'outname'):
			print self.parser.out
		elif name in ('home', 'base'):
			print self.parser.home
		elif name in ('list'):
			for src in self.parser:
				print src
		elif name in ('dirty', 'changed'):
			for src in self.parser:
				if src in self.dependence._dirty:
					print src
		return 0

		
#----------------------------------------------------------------------
# speed up
#----------------------------------------------------------------------
def _psyco_speedup():
	try:
		import psyco
		psyco.bind(preprocessor)
		psyco.bind(configure)
		psyco.bind(coremake)
		psyco.bind(emake)
		#print 'full optimaze'
	except:
		return False
	return True



#----------------------------------------------------------------------
# distribution
#----------------------------------------------------------------------
def install():
	filepath = os.path.abspath(sys.argv[0])
	if not os.path.exists(filepath):
		print 'error: cannot open "%s"'%filepath
		return -1
	if sys.platform[:3] == 'win':
		print 'error: install must under unix'
		return -2
	try:
		f1 = open(filepath, 'r')
	except:
		print 'error: cannot read "%s"'%filepath
		return -3
	content = f1.read()
	f1.close()
	name2 = '/usr/local/bin/emake.py'
	name3 = '/usr/local/bin/emake'
	if os.path.exists(name2):
		print '/usr/local/bin/emake.py already exists, you should delete it'
		return -6
	if os.path.exists(name3):
		print '/usr/local/bin/emake already exists, you should delete it'
		return -7
	try:
		f2 = open(name2, 'w')
	except:
		print 'error: cannot write "%s"'%name2
		return -4
	try:
		f3 = open(name3, 'w')
	except:
		print 'error: cannot write "%s"'%name3
		f2.close()
		return -5
	f2.write(content)
	f3.write(content)
	f2.close()
	f3.close()
	os.system('chmod 755 /usr/local/bin/emake.py')
	os.system('chmod 755 /usr/local/bin/emake')
	os.system('chown root /usr/local/bin/emake.py 2> /dev/nul')
	os.system('chown root /usr/local/bin/emake 2> /dev/nul')
	print 'install completed. you can uninstall by deleting the following two files:'
	print '/usr/local/bin/emake.py'
	print '/usr/local/bin/emake'
	return 0

__updated_files = {}

def __update_file(name, content):
	source = ''
	name = os.path.abspath(name)
	if name in __updated_files:
		return 0
	__updated_files[name] = 1
	try: 
		fp = open(name, 'r')
		source = fp.read()
		fp.close()
	except:
		source = ''
	if content == source:
		print '%s up-to-date'%name
		return 0
	try:
		fp = open(name, 'w')
		fp.write(content)
		fp.close()
	except:
		print 'can not write to %s'%name
		return -1
	print '%s update succeeded'%name
	return 1

def getemake():
	import urllib2
	url1 = 'http://easymake.googlecode.com/svn/trunk/emake.py'
	url2 = 'http://www.joynb.net/php/getemake.php'
	success = True
	content = ''
	print 'fetching ' + url1 + ' ...',;
	sys.stdout.flush();
	try:
		content = urllib2.urlopen(url1).read()
	except urllib2.URLError, e:
		success = False
		print 'failed '
		print e
	if not content: 
		success = False
	if success:
		print 'ok'
		return content
	success = True
	print 'fetching ' + url2 + ' ...',;
	sys.stdout.flush();
	try:
		content = urllib2.urlopen(url2).read()
	except urllib2.URLError, e:
		success = False
		print 'failed '
		print e
	if success:
		print 'ok'
		return content
	return ''

def update():
	content = getemake()
	if not content:
		print 'update failed'
		return -1
	name1 = os.path.abspath(sys.argv[0])
	name2 = '/usr/local/bin/emake.py'
	name3 = '/usr/local/bin/emake'
	__update_file(name1, content)
	if sys.platform[:3] == 'win':
		return 0
	r1 = __update_file(name2, content)
	r2 = __update_file(name3, content)
	if r1 > 0:
		os.system('chmod 755 /usr/local/bin/emake.py')
		os.system('chown root /usr/local/bin/emake.py 2> /dev/null')
	if r2 > 0:
		os.system('chmod 755 /usr/local/bin/emake')
		os.system('chown root /usr/local/bin/emake 2> /dev/null')
	print 'update finished !'
	return 0


#----------------------------------------------------------------------
# execute program
#----------------------------------------------------------------------
def execute(filename):
	path = os.path.abspath(filename)
	base = os.path.split(path)[0]
	name = os.path.split(path)[1]
	save = os.getcwd()
	os.chdir(base)
	part = os.path.splitext(name)
	extname = part[-1].lower()
	if sys.platform[:3] == 'win':
		os.system('"%s"'%part[0])
	else:
		cext = ('.c', '.cpp', '.cxx', '.cc', '.m', '.mm')
		dext = ('.asm', '.s', '.bas', '.pas')
		if (extname in  cext) or (extname in dext):
			os.system('"%s"'%os.path.join(base, part[0]))
		elif extname in ('.py', '.pyw', '.pyc', '.pyo'):
			os.system('python "%s"'%name)
		elif extname in ('.sh',):
			os.system('bash "%s"'%name)
		elif extname in ('.pl',):
			os.system('perl "%s"'%name)
		else:
			os.system('"%s"'%path)
	os.chdir(save)
	return 0


#----------------------------------------------------------------------
# main program
#----------------------------------------------------------------------
def main():
	# using psyco to speed up
	_psyco_speedup()

	# create main object
	make = emake()
	
	if len(sys.argv) == 1:
		print 'usage: "emake.py [option] srcfile" (emake v2.11 Dec.17 2010)'
		print 'options  :  -b | -build      build project'
		print '            -c | -compile    compile project'
		print '            -l | -link       link project'
		print '            -r | -rebuild    rebuild project'
		print '            -e | -execute    call the execution of the src'
		print '            -i | -install    install emake on unix'
		print '            -u | -update     update itself from google code'
		return 0

	cmd, name = 'build', ''

	if len(sys.argv) == 2:
		name = sys.argv[1].strip(' ')
		if name in ('-i', '--i', '-install', '--install'):
			install()
			return 0
		if name in ('-u', '--u', '-update', '--update'):
			update()
			return 0
		if name in ('-msvc', '--msvc'):
			print 'usage: emake.py --msvc [parameters of cl.exe]'
			return 0

	if len(sys.argv) >= 3:
		cmd = sys.argv[1].strip(' ').lower()
		name = sys.argv[2]
	
	ext = os.path.splitext(name)[-1].lower() 
	ft1 = ('.c', '.cpp', '.cxx', '.cc')
	ft2 = ('.h', '.hpp', '.hxx', '.hh', '.inc')
	ft3 = ('.mak', '.proj', '.prj')

	if cmd in ('e', '-e', '--e', 'execute', '-execute', '--execute'):
		execute(name)
		if len(sys.argv) > 3:
			sys.stdout.write('press enter to continue ...')
			sys.stdout.flush()
			n = raw_input()
		return 0

	if cmd in ('-msvc', '--msvc'):
		config = configure()
		config.init()
		parameters = ''
		for n in  [ sys.argv[i] for i in xrange(2, len(sys.argv)) ]:
			if ' ' in n: n = '"' + n + '"'
			parameters += n + ' '
		config.msvcexe('cl.exe', parameters)
		return 0

	if not ((ext in ft1) or (ext in ft2) or (ext in ft3)):
		sys.stderr.write('error: %s: unknow file type\n'%(name))
		sys.stderr.flush()
		return -1

	if cmd in ('b', '-b', '--b', 'build', '-build', '--build'):
		make.open(name)
		make.build(3)
	elif cmd in ('c', '-c', '--c', 'compile', '-compile', '--compile'):
		make.open(name)
		make.compile(3)
	elif cmd in ('l', '-l', '--l', 'link', '-link', '--link'):
		make.open(name)
		make.link(3)
	elif cmd in ('e', '-e', '--e', 'clean', '-clean', '--clean'):
		make.open(name)
		make.clean()
	elif cmd in ('r', '-r', '--r', 'rebuild', '-rebuild', '--rebuild'):
		make.open(name)
		make.rebuild(3)
	elif cmd in ('d', '-d', '--d', 'detail', '-detail', '--detail'):
		make.open(name)
		make.info('outname');
	elif cmd in ('dirty', '-dirty', '--dirty'):
		make.open(name)
		make.info('dirty')
	elif cmd in ('list', '-list', '--list'):
		make.open(name)
		make.info('list')
	elif cmd in ('home', '-home'):
		make.open(name)
		make.info('home')
	return 0


#----------------------------------------------------------------------
# testing case
#----------------------------------------------------------------------
if __name__ == '__main__':
	def test1():
		make = coremake()
		name = 'e:/zombie/demo01.c'
		make.mkdir(r'e:\lab\malloc\obj list')
		make.mkdir(r'e:\lab\malloc\abc c\01 2\3 4\5\6')
		make.init('mainmod', 'exe', 'malloc\obj')
		make.push('malloc/main.c')
		make.push('malloc/mod1.c')
		make.push('malloc/mod2.c')
		make.push('malloc/mod3.c')
		make.build(printmode = 7)
		print os.path.getmtime('malloc/main.c')
	def test2():
		pst = preprocessor()
		head, lost, text = pst.dependence('voice/fastvoice/basewave.cpp')
		for n in head: print n
		pp = pst.preprocess(file('voice/fastvoice/basewave.cpp', 'U').read())
		print pp
	def test3():
		parser = iparser()
		parser._pragma_scan('malloc/main.c')
	def test4():
		parser = iparser()
		cmaker = coremake()
		parser.parse('malloc/main.c')
		print '"%s", "%s", "%s"'%(parser.out, parser.int, parser.mode)
		print parser.home, parser.name
		for n in parser:
			print 'src:', n, '->', cmaker.objname(n, ''), parser[n]
	def test5():
		parser = iparser()
		parser.parse('malloc/main.c')
		dep = dependence(parser)
		dep.process()
	def test6():
		make = emake()
		make.open('malloc/main.c')
		make.clean()
		make.build(3)
	def test7():
		config = configure()
		config.init()
		print config.checklib('liblinwei.a')
		print config.checklib('winmm')
		print config.checklib('pixia')
		config.push_lib('d:/dev/local/lib')
		print config.checklib('pixia')
	
	main()
	#install()

