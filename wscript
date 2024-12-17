# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-
"""
Copyright (c) 2014-2024,  The University of Memphis,
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

import os
import subprocess
from waflib import Context, Logs, Utils

VERSION = '24.08'
APPNAME = 'nlsr'
GIT_TAG_PREFIX = 'NLSR-'

def options(opt):
    opt.load(['compiler_cxx', 'gnu_dirs'])
    opt.load(['default-compiler-flags',
              'coverage', 'sanitizers', 'boost',
              'doxygen', 'sphinx'],
             tooldir=['.waf-tools'])

    optgrp = opt.add_option_group('NLSR Options')

    optgrp.add_option('--with-chronosync', dest='with_chronosync', action='store_true', default=False,
                      help='Build with ChronoSync support')
    optgrp.add_option('--without-chronosync', dest='with_chronosync', action='store_false', default=False,
                      help='Build without ChronoSync support')

    optgrp.add_option('--with-psync', dest='with_psync', action='store_true', default=True,
                      help='Build with PSync support')
    optgrp.add_option('--without-psync', dest='with_psync', action='store_false', default=True,
                      help='Build without PSync support')

    optgrp.add_option('--with-svs', dest='with_svs', action='store_true', default=False,
                      help='Build with State Vector Sync support')
    optgrp.add_option('--without-svs', dest='with_svs', action='store_false', default=False,
                      help='Build without State Vector Sync support')

    optgrp.add_option('--with-tests', action='store_true', default=False,
                      help='Build unit tests')

def configure(conf):
    conf.load(['compiler_cxx', 'gnu_dirs',
               'default-compiler-flags', 'boost',
               'doxygen', 'sphinx'])

    conf.env.WITH_TESTS = conf.options.with_tests

    conf.find_program('dot', mandatory=False)

    # Prefer pkgconf if it's installed, because it gives more correct results
    # on Fedora/CentOS/RHEL/etc. See https://bugzilla.redhat.com/show_bug.cgi?id=1953348
    # Store the result in env.PKGCONFIG, which is the variable used inside check_cfg()
    conf.find_program(['pkgconf', 'pkg-config'], var='PKGCONFIG')

    pkg_config_path = os.environ.get('PKG_CONFIG_PATH', f'{conf.env.LIBDIR}/pkgconfig')
    conf.check_cfg(package='libndn-cxx', args=['libndn-cxx >= 0.9.0', '--cflags', '--libs'],
                   uselib_store='NDN_CXX', pkg_config_path=pkg_config_path)

    conf.check_boost()
    if conf.env.BOOST_VERSION_NUMBER < 107100:
        conf.fatal('The minimum supported version of Boost is 1.71.0.\n'
                   'Please upgrade your distribution or manually install a newer version of Boost.\n'
                   'For more information, see https://redmine.named-data.net/projects/nfd/wiki/Boost')

    if conf.env.WITH_TESTS:
        conf.check_boost(lib='unit_test_framework', mt=True, uselib_store='BOOST_TESTS')

    if conf.options.with_chronosync:
        conf.check_cfg(package='ChronoSync', args=['ChronoSync >= 0.5.6', '--cflags', '--libs'],
                       uselib_store='CHRONOSYNC', pkg_config_path=pkg_config_path)

    if conf.options.with_psync:
        conf.check_cfg(package='PSync', args=['PSync >= 0.5.0', '--cflags', '--libs'],
                       uselib_store='PSYNC', pkg_config_path=pkg_config_path)

    if conf.options.with_svs:
        conf.check_cfg(package='libndn-svs', args=['libndn-svs >= 0.1.0', '--cflags', '--libs'],
                       uselib_store='SVS', pkg_config_path=pkg_config_path)

    if not any((conf.options.with_chronosync, conf.options.with_psync, conf.options.with_svs)):
        conf.fatal('Cannot compile without any Sync protocol.\n'
                   'Specify at least one of --with-psync or --with-svs or --with-chronosync')

    if conf.env.WITH_TESTS and not conf.options.with_psync:
        conf.fatal('--with-tests requires --with-psync')

    conf.check_compiler_flags()

    # Loading "late" to prevent tests from being compiled with profiling flags
    conf.load('coverage')
    conf.load('sanitizers')

    conf.define_cond('WITH_TESTS', conf.env.WITH_TESTS)
    conf.define('DEFAULT_CONFIG_FILE', f'{conf.env.SYSCONFDIR}/ndn/nlsr.conf')
    # The config header will contain all defines that were added using conf.define()
    # or conf.define_cond().  Everything that was added directly to conf.env.DEFINES
    # will not appear in the config header, but will instead be passed directly to the
    # compiler on the command line.
    conf.write_config_header('config.hpp')

def build(bld):
    version(bld)

    bld(features='subst',
        name='version.hpp',
        source='src/version.hpp.in',
        target='src/version.hpp',
        install_path=None,
        VERSION_STRING=VERSION_BASE,
        VERSION_BUILD=VERSION)

    bld.objects(
        target='nlsr-objects',
        source=bld.path.ant_glob('src/**/*.cpp', excl=['src/main.cpp']),
        use='BOOST NDN_CXX CHRONOSYNC PSYNC SVS',
        includes='. src',
        export_includes='. src')

    bld.program(
        name='nlsr',
        target='bin/nlsr',
        source='src/main.cpp',
        use='nlsr-objects')

    bld.program(
        name='nlsrc',
        target='bin/nlsrc',
        source='tools/nlsrc.cpp',
        use='nlsr-objects')

    if bld.env.WITH_TESTS:
        bld.recurse('tests')

    # Install sample config
    bld.install_as('${SYSCONFDIR}/ndn/nlsr.conf.sample', 'nlsr.conf')

    if Utils.unversioned_sys_platform() == 'linux':
        bld(features='subst',
            name='systemd-units',
            source='systemd/nlsr.service.in',
            target='systemd/nlsr.service')

    if bld.env.SPHINX_BUILD:
        bld(features='sphinx',
            name='manpages',
            builder='man',
            config='docs/conf.py',
            outdir='docs/manpages',
            source=bld.path.ant_glob('docs/manpages/*.rst'),
            install_path='${MANDIR}',
            version=VERSION_BASE,
            release=VERSION)

def docs(bld):
    from waflib import Options
    Options.commands = ['doxygen', 'sphinx'] + Options.commands

def doxygen(bld):
    version(bld)

    if not bld.env.DOXYGEN:
        bld.fatal('Cannot build documentation ("doxygen" not found in PATH)')

    bld(features='subst',
        name='doxygen.conf',
        source=['docs/doxygen.conf.in',
                'docs/named_data_theme/named_data_footer-with-analytics.html.in'],
        target=['docs/doxygen.conf',
                'docs/named_data_theme/named_data_footer-with-analytics.html'],
        VERSION=VERSION,
        HAVE_DOT='YES' if bld.env.DOT else 'NO',
        HTML_FOOTER='../build/docs/named_data_theme/named_data_footer-with-analytics.html' \
                        if os.getenv('GOOGLE_ANALYTICS', None) \
                        else '../docs/named_data_theme/named_data_footer.html',
        GOOGLE_ANALYTICS=os.getenv('GOOGLE_ANALYTICS', ''))

    bld(features='doxygen',
        doxyfile='docs/doxygen.conf',
        use='doxygen.conf')

def sphinx(bld):
    version(bld)

    if not bld.env.SPHINX_BUILD:
        bld.fatal('Cannot build documentation ("sphinx-build" not found in PATH)')

    bld(features='sphinx',
        config='docs/conf.py',
        outdir='docs',
        source=bld.path.ant_glob('docs/**/*.rst'),
        version=VERSION_BASE,
        release=VERSION)

def version(ctx):
    # don't execute more than once
    if getattr(Context.g_module, 'VERSION_BASE', None):
        return

    Context.g_module.VERSION_BASE = Context.g_module.VERSION
    Context.g_module.VERSION_SPLIT = VERSION_BASE.split('.')

    # first, try to get a version string from git
    version_from_git = ''
    try:
        cmd = ['git', 'describe', '--abbrev=8', '--always', '--match', f'{GIT_TAG_PREFIX}*']
        version_from_git = subprocess.run(cmd, capture_output=True, check=True, text=True).stdout.strip()
        if version_from_git:
            if GIT_TAG_PREFIX and version_from_git.startswith(GIT_TAG_PREFIX):
                Context.g_module.VERSION = version_from_git[len(GIT_TAG_PREFIX):]
            elif not GIT_TAG_PREFIX and ('.' in version_from_git or '-' in version_from_git):
                Context.g_module.VERSION = version_from_git
            else:
                # no tags matched (or we are in a shallow clone)
                Context.g_module.VERSION = f'{VERSION_BASE}+git.{version_from_git}'
    except (OSError, subprocess.SubprocessError):
        pass

    # fallback to the VERSION.info file, if it exists and is not empty
    version_from_file = ''
    version_file = ctx.path.find_node('VERSION.info')
    if version_file is not None:
        try:
            version_from_file = version_file.read().strip()
        except OSError as e:
            Logs.warn(f'{e.filename} exists but is not readable ({e.strerror})')
    if version_from_file and not version_from_git:
        Context.g_module.VERSION = version_from_file
        return

    # update VERSION.info if necessary
    if version_from_file == Context.g_module.VERSION:
        # already up-to-date
        return
    if version_file is None:
        version_file = ctx.path.make_node('VERSION.info')
    try:
        version_file.write(Context.g_module.VERSION)
    except OSError as e:
        Logs.warn(f'{e.filename} is not writable ({e.strerror})')

def dist(ctx):
    ctx.algo = 'tar.xz'
    version(ctx)

def distcheck(ctx):
    ctx.algo = 'tar.xz'
    version(ctx)
