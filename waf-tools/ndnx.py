#! /usr/bin/env python
# encoding: utf-8

'''

When using this tool, the wscript will look like:

	def options(opt):
	        opt.tool_options('ndnx')

	def configure(conf):
		conf.load('compiler_c ndnx')

	def build(bld):
		bld(source='main.cpp', target='app', use='NDNX')

Options are generated, in order to specify the location of ndnx includes/libraries.


'''
import sys, re
from waflib import Utils, Logs, Errors, Options, ConfigSet
from waflib.Configure import conf

NDNX_DIR=['/usr','/usr/local','/opt/local','/sw']
NDNX_VERSION_FILE='ndn/ndn.h'
NDNX_VERSION_CODE='''
#include <ndn/ndn.h>
#include <stdio.h>
int main() { printf ("%d.%d.%d", ((NDN_API_VERSION/100000) % 100), ((NDN_API_VERSION/1000) % 100), (NDN_API_VERSION % 1000)); return 0; }
'''

@conf
def __ndnx_get_version_file(self,dir):
	# Logs.pprint ('CYAN', '  + %s/%s/%s' % (dir, 'include', NDNX_VERSION_FILE))
	try:
		return self.root.find_dir(dir).find_node('%s/%s' % ('include', NDNX_VERSION_FILE))
	except:
		return None
@conf
def ndnx_get_version(self,dir):
	val=self.check_cc(fragment=NDNX_VERSION_CODE,includes=['%s/%s' % (dir, 'include')],execute=True,define_ret = True, mandatory=True)
	return val
@conf
def ndnx_get_root(self,*k,**kw):
	root=Options.options.ndnx_dir or (k and k[0]) or kw.get('path',None)
        
	if root:
                if self.__ndnx_get_version_file(root):
                        return root
		self.fatal('NDNx not found in %s'%root)
                
	for dir in NDNX_DIR:
		if self.__ndnx_get_version_file(dir):
			return dir
        self.fatal('NDNx not found, please provide a --ndnx argument (see help)')

@conf
def check_openssl(self,*k,**kw):
        root = k and k[0] or kw.get('path',None) or Options.options.openssl
        mandatory = kw.get('mandatory', True)
        var = kw.get('var', 'SSL')

        CODE = """
#include <openssl/crypto.h>
#include <stdio.h>

int main(int argc, char **argv) {
	(void)argc;
        printf ("%s", argv[0]);

	return 0;
}
"""
        if root:
                testApp = self.check_cc (lib=['ssl', 'crypto'],
                                         header_name='openssl/crypto.h',
                                         define_name='HAVE_%s' % var,
                                         uselib_store=var,
                                         mandatory = mandatory,
                                         cflags="-I%s/include" % root,
                                         linkflags="-L%s/lib" % root,
                                         execute = True, fragment = CODE, define_ret = True)
        else:
                testApp = libcrypto = self.check_cc (lib=['ssl', 'crypto'],
                                                     header_name='openssl/crypto.h',
                                                     define_name='HAVE_%s' % var,
                                                     uselib_store=var,
                                                     mandatory = mandatory,
                                                     execute = True, fragment = CODE, define_ret = True)

        if not testApp:
                return

        self.start_msg ('Checking if selected openssl matches NDNx')

        ndn_var = kw.get('ndn_var', "NDNX")
        if Utils.unversioned_sys_platform () == "darwin":
                def otool (binary):
                        p = Utils.subprocess.Popen (['/usr/bin/otool', '-L', binary], 
                                                    stdout = Utils.subprocess.PIPE, )
                        for line in p.communicate()[0].split ('\n'):
                                if re.match ('.*/libcrypto\..*', line):
                                        return line

                selected_crypto = otool (testApp)
                ndnd_crypto = otool ('%s/bin/ndnd' % self.env['%s_ROOT' % ndn_var])

                if ndnd_crypto != selected_crypto:
                        self.fatal ("Selected openssl does not match used to compile NDNx (%s != %s)" % 
                                    (selected_crypto.strip (), ndnd_crypto.strip ()))
                self.end_msg (True)

        elif Utils.unversioned_sys_platform () == "linux" or  Utils.unversioned_sys_platform () == "freebsd":
                def ldd (binary):
                        p = Utils.subprocess.Popen (['/usr/bin/ldd', binary], 
                                                        stdout = Utils.subprocess.PIPE, )
                        for line in p.communicate()[0].split ('\n'):
                                if re.match ('libcrypto\..*', line):
                                        return line

                selected_crypto = ldd (testApp)
                ndnd_crypto = ldd ('%s/bin/ndnd' % self.env['%s_ROOT' % ndn_var])

                if ndnd_crypto != selected_crypto:
                        self.fatal ("Selected openssl does not match used to compile NDNx (%s != %s)" % 
                                    (selected_crypto.strip (), ndnd_crypto.strip ()))
                self.end_msg (True)
        else:
                self.end_msg ("Don't know how to check", 'YELLOW')

@conf
def check_ndnx(self,*k,**kw):
	if not self.env['CC']:
		self.fatal('load a c compiler first, conf.load("compiler_c")')

	var=kw.get('uselib_store', 'NDNX')
	self.start_msg('Checking for NDNx')
	root = self.ndnx_get_root(*k,**kw);
	self.env.NDNX_VERSION=self.ndnx_get_version(root)

	self.env['INCLUDES_%s' % var]= '%s/%s' % (root, "include");
	self.env['LIB_%s' % var] = "ndn"
	self.env['LIBPATH_%s' % var] = '%s/%s' % (root, "lib")

        self.env['%s_ROOT' % var] = root

	self.end_msg("%s in %s " % (self.env.NDNX_VERSION, root))
	if Logs.verbose:
		Logs.pprint('CYAN','	NDNx include : %s'%self.env['INCLUDES_%s' % var])
		Logs.pprint('CYAN','	NDNx lib     : %s'%self.env['LIB_%s' % var])
		Logs.pprint('CYAN','	NDNx libpath : %s'%self.env['LIBPATH_%s' % var])

def options(opt):
        """
        NDNx options
        """
        ndnopt = opt.add_option_group("NDNx Options")
	ndnopt.add_option('--ndnx',type='string',default=None,dest='ndnx_dir',help='''path to where NDNx is installed, e.g. /usr/local''')
        ndnopt.add_option('--openssl',type='string',default='',dest='openssl',help='''path to openssl, should be the same NDNx is compiled against''')
