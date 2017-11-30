# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

"""
Copyright (c) 2014-2017,  The University of Memphis,
                          Regents of the University of California,
                          Arizona Board of Regents.

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

VERSION = "0.4.0"
APPNAME = "nlsr"
BUGREPORT = "https://redmine.named-data.net/projects/nlsr"
URL = "https://named-data.net/doc/NLSR/"
GIT_TAG_PREFIX = "NLSR-"

from waflib import Logs, Utils, Context
import os

def options(opt):
    opt.load(['compiler_cxx', 'gnu_dirs'])
    opt.load(['default-compiler-flags', 'coverage', 'sanitizers',
              'boost', 'doxygen', 'sphinx_build'],
            tooldir=['.waf-tools'])

    nlsropt = opt.add_option_group('NLSR Options')

    nlsropt.add_option('--with-tests', action='store_true', default=False, dest='with_tests',
                       help='''build unit tests''')


def configure(conf):
    conf.load(['compiler_cxx', 'gnu_dirs',
               'default-compiler-flags', 'boost',
               'doxygen', 'sphinx_build'])

    if 'PKG_CONFIG_PATH' not in os.environ:
        os.environ['PKG_CONFIG_PATH'] = Utils.subst_vars('${LIBDIR}/pkgconfig', conf.env)

    conf.check_cfg(package='libndn-cxx', args=['--cflags', '--libs'],
                   uselib_store='NDN_CXX', mandatory=True)

    conf.check_cfg(package='liblog4cxx', args=['--cflags', '--libs'],
                   uselib_store='LOG4CXX', mandatory=True)

    boost_libs = 'system chrono program_options iostreams thread regex filesystem'
    if conf.options.with_tests:
        conf.env['WITH_TESTS'] = 1
        conf.define('WITH_TESTS', 1);
        boost_libs += ' unit_test_framework'

    conf.check_boost(lib=boost_libs)

    if conf.env.BOOST_VERSION_NUMBER < 104800:
        Logs.error("Minimum required boost version is 1.48.0")
        Logs.error("Please upgrade your distribution or install custom boost libraries")
        return

    conf.check_cfg(package='ChronoSync', args=['ChronoSync >= 0.3', '--cflags', '--libs'],
                   uselib_store='SYNC', mandatory=True)

    conf.load('coverage')

    conf.load('sanitizers')

    conf.define('DEFAULT_CONFIG_FILE', '%s/ndn/nlsr.conf' % conf.env['SYSCONFDIR'])

    conf.write_config_header('config.hpp')


def build(bld):
    version(bld)

    bld(features="subst",
        name='version',
        source='src/version.hpp.in',
        target='src/version.hpp',
        install_path=None,
        VERSION_STRING=VERSION_BASE,
        VERSION_BUILD=VERSION,
        VERSION=int(VERSION_SPLIT[0]) * 1000000 +
                int(VERSION_SPLIT[1]) * 1000 +
                int(VERSION_SPLIT[2]),
        VERSION_MAJOR=VERSION_SPLIT[0],
        VERSION_MINOR=VERSION_SPLIT[1],
        VERSION_PATCH=VERSION_SPLIT[2])

    nlsr_objects = bld(
        target='nlsr-objects',
        name='nlsr-objects',
        features='cxx',
        source=bld.path.ant_glob(['src/**/*.cpp'],
                                 excl=['src/main.cpp']),
        use='NDN_CXX BOOST LOG4CXX SYNC',
        includes='. src',
        export_includes='. src')

    nlsr = bld(
        target='bin/nlsr',
        features='cxx cxxprogram',
        source='src/main.cpp',
        use='nlsr-objects')

    nlsrc = bld(
        target='bin/nlsrc',
        features='cxx cxxprogram',
        source='tools/nlsrc.cpp',
        use='nlsr-objects BOOST')

    bld(features="subst",
        source='nlsr.conf',
        target='nlsr.conf.sample',
        install_path="${SYSCONFDIR}/ndn")

    if bld.env['WITH_TESTS']:
        bld.recurse('tests')

    if bld.env['SPHINX_BUILD']:
        bld(features="sphinx",
            builder="man",
            outdir="docs/manpages",
            config="docs/conf.py",
            source=bld.path.ant_glob('docs/manpages/**/*.rst'),
            install_path="${MANDIR}/",
            VERSION=VERSION)

def docs(bld):
    from waflib import Options
    Options.commands = ['doxygen', 'sphinx'] + Options.commands

def doxygen(bld):
    version(bld)

    if not bld.env.DOXYGEN:
        Logs.error("ERROR: cannot build documentation (`doxygen' not found in $PATH)")
    else:
        bld(features="subst",
            name="doxygen-conf",
            source="docs/doxygen.conf.in",
            target="docs/doxygen.conf",
            VERSION=VERSION_BASE)

        bld(features="doxygen",
            doxyfile='docs/doxygen.conf',
            use="doxygen-conf")

def sphinx(bld):
    version(bld)

    if not bld.env.SPHINX_BUILD:
        bld.fatal("ERROR: cannot build documentation (`sphinx-build' not found in $PATH)")
    else:
        bld(features="sphinx",
            outdir="docs",
            source=bld.path.ant_glob('docs/**/*.rst'),
            config="docs/conf.py",
            VERSION=VERSION_BASE)

def version(ctx):
    if getattr(Context.g_module, 'VERSION_BASE', None):
        return

    Context.g_module.VERSION_BASE = Context.g_module.VERSION
    Context.g_module.VERSION_SPLIT = [v for v in VERSION_BASE.split('.')]

    didGetVersion = False
    try:
        cmd = ['git', 'describe', '--always', '--match', '%s*' % GIT_TAG_PREFIX]
        p = Utils.subprocess.Popen(cmd, stdout=Utils.subprocess.PIPE,
                                   stderr=None, stdin=None)
        out = str(p.communicate()[0].strip())
        didGetVersion = (p.returncode == 0 and out != "")
        if didGetVersion:
            if out.startswith(GIT_TAG_PREFIX):
                Context.g_module.VERSION = out[len(GIT_TAG_PREFIX):]
            else:
                Context.g_module.VERSION = "%s-commit-%s" % (Context.g_module.VERSION_BASE, out)
    except OSError:
        pass

    versionFile = ctx.path.find_node('VERSION')

    if not didGetVersion and versionFile is not None:
        try:
            Context.g_module.VERSION = versionFile.read()
            return
        except (OSError, IOError):
            pass

    # version was obtained from git, update VERSION file if necessary
    if versionFile is not None:
        try:
            version = versionFile.read()
            if version == Context.g_module.VERSION:
                return # no need to update
        except (OSError, IOError):
            Logs.warn("VERSION file exists, but not readable")
    else:
        versionFile = ctx.path.make_node('VERSION')

    if versionFile is None:
        return

    try:
        versionFile.write(Context.g_module.VERSION)
    except (OSError, IOError):
        Logs.warn("VERSION file is not writeable")

def dist(ctx):
    version(ctx)

def distcheck(ctx):
    version(ctx)
