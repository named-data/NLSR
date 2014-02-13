# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

VERSION='1.0.0'
APPNAME='nlsr'

from waflib import Configure, Build, Logs

def options(opt):
    opt.load('compiler_cxx boost')
    # opt.load('ndnx', tooldir=["waf-tools"])

    syncopt = opt.add_option_group ("nlsr Options")

    syncopt.add_option('--debug',action='store_true',default=False,dest='debug',help='''debugging mode''')

def configure(conf):
    conf.load('compiler_cxx boost')

    if conf.options.debug:
        conf.define ('_DEBUG', 1)
        conf.add_supported_cxxflags (cxxflags = ['-O0',
                                                 '-Wall',
                                                 '-Wno-unused-variable',
                                                 '-g3',
                                                 '-Wno-unused-private-field', # only clang supports
                                                 '-fcolor-diagnostics',       # only clang supports
                                                 '-Qunused-arguments',         # only clang supports
                                                 '-Wno-tautological-compare', # suppress warnings from CryptoPP
                                                 '-Wno-deprecated-declarations'
                                                 ])
    else:
        conf.add_supported_cxxflags (cxxflags = ['-O3',
                                                 '-g',
                                                 '-Wno-unused-variable',
                                                 '-Wno-tautological-compare', 
                                                 '-Wno-unused-function', 
                                                 '-Wno-deprecated-declarations'
                                                 ])

    conf.check_cfg(package='libndn-cpp-dev', args=['--cflags', '--libs'], uselib_store='NDNCPP', mandatory=True)

    conf.check_boost(lib='system iostreams thread unit_test_framework', mandatory=True)
    
    conf.check_cfg(package='ChronoSync', args=['--cflags', '--libs'], uselib_store='ChronoSync', mandatory=True)



def build (bld):
    bld (
        # vnum = "1.0.0",
        features=['cxx', 'cxxprogram'],
        target="nlsr",
        source = bld.path.ant_glob('src/*.cpp'),
        use = 'BOOST NDNCPP ChronoSync',
        #cwd = bld.path.find_dir ("src"),
        #includes = ['src'],
        )

    #bld.install_files (
     #   dest = "%s/nlsr" % bld.env['INCLUDEDIR'], 
     #   files = bld.path.ant_glob(['*.hpp']), 
     #   cwd = bld.path.find_dir ("src"),
     #   relative_trick = True,
     #  )

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
