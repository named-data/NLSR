# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

from waflib import Build, Logs, Utils, Task, TaskGen, Configure

def options(opt):
    opt.load('compiler_c compiler_cxx gnu_dirs c_osx')
    opt.load('boost cryptopp', tooldir=['waf-tools'])

    opt = opt.add_option_group('Options')
    opt.add_option('--debug',action='store_true',default=False,dest='debug',help='''debugging mode''')

def configure(conf):
    conf.load("compiler_c compiler_cxx boost gnu_dirs c_osx cryptopp")

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
        flags = ['-O3', '-g','-Wno-unused-variable', '-Wno-tautological-compare',
                         '-Wno-unused-function', '-Wno-deprecated-declarations']
        conf.add_supported_cxxflags (cxxflags = flags)

    conf.check_cfg(package='libndn-cpp-dev', args=['--cflags', '--libs'], uselib_store='NDN_CPP', mandatory=True)
    conf.check_boost(lib='system iostreams thread unit_test_framework log', uselib_store='BOOST', version='1_55', mandatory=True)
    conf.check_cfg(package='nsync', args=['--cflags', '--libs'], uselib_store='nsync', mandatory=True)
    conf.check_cfg(package='sqlite3', args=['--cflags', '--libs'], uselib_store='SQLITE3', mandatory=True)
    conf.check_cryptopp(path=conf.options.cryptopp_dir, mandatory=True)

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
