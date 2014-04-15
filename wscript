# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
VERSION='1.0'
NAME="NLSR"

from waflib import Build, Logs, Utils, Task, TaskGen, Configure
from waflib.Tools import c_preproc

def options(opt):
    opt.load('compiler_c compiler_cxx gnu_dirs c_osx')
    opt.load('boost openssl cryptopp', tooldir=['waf-tools'])

    opt = opt.add_option_group('NLSR Options')

    opt.add_option('--debug',action='store_true',default=False,dest='debug',help='''debugging mode''')


def configure(conf):
    conf.load("compiler_c compiler_cxx boost gnu_dirs c_osx openssl cryptopp")

    conf.check_openssl()

    if conf.options.debug:
        conf.define ('_DEBUG', 1)
        flags = ['-O0',
                 '-Wall',
                 # '-Werror',
                 '-Wno-unused-variable',
                 '-g3',
                 '-Wno-unused-private-field', # only clang supports
                 '-fcolor-diagnostics',       # only clang supports
                 '-Qunused-arguments',        # only clang supports
                 '-Wno-tautological-compare', # suppress warnings from CryptoPP
                 '-Wno-unused-function',      # another annoying warning from CryptoPP

                 '-Wno-deprecated-declarations',
                 ]

        conf.add_supported_cxxflags (cxxflags = flags)
    else:
        flags = ['-O3', '-g', '-Wno-tautological-compare', '-Wno-unused-function', '-Wno-deprecated-declarations']
        conf.add_supported_cxxflags (cxxflags = flags)


    conf.check_cfg(package='libndn-cpp-dev', args=['--cflags', '--libs'], uselib_store='NDN_CPP', mandatory=True)
    conf.check_cfg(package='nsync', args=['--cflags', '--libs'], uselib_store='nsync', mandatory=True)
    conf.check_cfg(package='sqlite3', args=['--cflags', '--libs'], uselib_store='SQLITE3', mandatory=True)
    conf.check_cryptopp(path=conf.options.cryptopp_dir, mandatory=True)
    conf.check_boost(lib='system iostreams thread unit_test_framework log', uselib_store='BOOST', mandatory=True)
    if conf.env.BOOST_VERSION_NUMBER < 105400:
        Logs.error ("Minimum required boost version is 1.54.0")
        Logs.error ("Please upgrade your distribution or install custom boost libraries")
        return




def build (bld):
    bld (
        features=['cxx', 'cxxprogram'],
        target="nlsr",
        source = bld.path.ant_glob('src/**/*.cpp'),
        use = 'NDN_CPP BOOST CRYPTOPP SQLITE3 nsync',
        includes = ". src"
        )
    bld.recurse("CertTool")

@Configure.conf
def add_supported_cxxflags(self, cxxflags):
    """
    Check which cxxflags are supported by compiler and add them to env.CXXFLAGS variable
    """
    self.start_msg('Checking allowed flags for c++ compiler')

    supportedFlags = []
    for flag in cxxflags:
        if self.check_cxx (cxxflags=[flag], mandatory=False):
            supportedFlags += [flag]

    self.end_msg (' '.join (supportedFlags))
    self.env.CXXFLAGS += supportedFlags
