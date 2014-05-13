# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

"""
Copyright (c) 2014  University of Memphis,
                    Regents of the University of California

This file is part of NLSR (Named-data Link State Routing).
See AUTHORS.md for complete list of NLSR authors and contributors.

NLSR is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
"""

VERSION='1.0'
NAME="NLSR"

from waflib import Build, Logs, Utils, Task, TaskGen, Configure
from waflib.Tools import c_preproc

def options(opt):
    opt.load(['compiler_cxx', 'gnu_dirs'])
    opt.load(['default-compiler-flags', 'coverage',
              'boost', 'protoc', 'openssl',
              'doxygen', 'sphinx_build'],
            tooldir=['.waf-tools'])

    nlsropt = opt.add_option_group('NLSR Options')

    nlsropt.add_option('--with-tests', action='store_true', default=False, dest='with_tests',
                       help='''build unit tests''')


def configure(conf):
    conf.load("compiler_cxx gnu_dirs boost openssl")

    conf.load('default-compiler-flags')

    conf.check_cfg(package='libndn-cxx', args=['--cflags', '--libs'],
                   uselib_store='NDN_CPP', mandatory=True)

    boost_libs = 'system chrono program_options iostreams thread regex'
    if conf.options.with_tests:
        conf.env['WITH_TESTS'] = 1
        conf.define('WITH_TESTS', 1);
        boost_libs += ' unit_test_framework'

    conf.check_boost(lib=boost_libs)

    if conf.env.BOOST_VERSION_NUMBER < 104800:
        Logs.error("Minimum required boost version is 1.48.0")
        Logs.error("Please upgrade your distribution or install custom boost libraries")
        return

    conf.load('protoc')

    conf.load('coverage')

    conf.define('DEFAULT_CONFIG_FILE', '%s/ndn/nlsr.conf' % conf.env['SYSCONFDIR'])

    conf.write_config_header('config.hpp')


def build (bld):
    nsync_objects = bld(
        target='nsync-objects',
        name='nsync-objects',
        features='cxx',
        source=bld.path.ant_glob(['nsync/**/*.cc', 'nsync/**/*.proto']),
        use='BOOST NDN_CPP OPENSSL',
        includes='nsync',
        export_includes='nsync',
        )

    nlsr_objects = bld(
        target='nlsr-objects',
        name='nlsr-objects',
        features='cxx',
        source=bld.path.ant_glob(['src/**/*.cpp'],
                                 excl=['src/main.cpp']),
        use='nsync-objects NDN_CPP BOOST',
        includes='. src',
        export_includes='. src',
        )

    nlsr = bld(
        target='bin/nlsr',
        features='cxx cxxprogram',
        source='src/main.cpp',
        use='nlsr-objects',
        )

    if bld.env['WITH_TESTS']:
        bld.recurse('tests')
        bld.recurse('tests-integrated')
