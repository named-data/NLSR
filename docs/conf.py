# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import importlib.util
import sys

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'NLSR'
copyright = '2014-2026, Named Data Networking Project'
author = 'Named Data Networking Project'

# The short X.Y version.
#version = ''

# The full version, including alpha/beta/rc tags.
#release = ''

# There are two options for replacing |today|: either, you set today to some
# non-false value, then it is used:
#today = ''
# Else, today_fmt is used as the format for a strftime call.
today_fmt = '%Y-%m-%d'


# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

needs_sphinx = '7.0'
extensions = [
    'sphinx.ext.extlinks',
    'sphinx.ext.todo',
]

def addExtensionIfExists(extension: str):
    try:
        if importlib.util.find_spec(extension) is None:
            raise ModuleNotFoundError(extension)
    except (ImportError, ValueError):
        sys.stderr.write(f'WARNING: Extension {extension!r} not found. '
                         'Some documentation may not build correctly.\n')
    else:
        extensions.append(extension)

addExtensionIfExists('sphinxcontrib.doxylink')

exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# Disable syntax highlighting of code blocks by default.
highlight_language = 'none'

# Generate warnings for all missing references.
nitpicky = True


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'furo'
html_logo = 'ndn-logo.svg'
html_copy_source = False
html_show_sourcelink = False

html_theme_options = {
    'light_css_variables': {
        'color-brand-primary': '#bc4010',
        'color-brand-content': '#c85000',
        'color-brand-visited': '#c85000',
    },
    'dark_css_variables': {
        'color-brand-primary': '#fd861a',
        'color-brand-content': '#f87e00',
        'color-brand-visited': '#f87e00',
    },
    'source_repository': 'https://github.com/named-data/NLSR',
    'source_branch': 'master',
}

pygments_style = 'tango'
pygments_dark_style = 'material'


# -- Options for manual page output ------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-manual-page-output

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    ('manpages/nlsr',      'nlsr',      'Named Data Link State Routing daemon', [], 1),
    ('manpages/nlsr.conf', 'nlsr.conf', 'Named Data Link State Routing daemon configuration file', [], 5),
    ('manpages/nlsrc',     'nlsrc',     'command-line utility to interact with and collect statistics from NLSR', [], 1),
]


# -- Misc options ------------------------------------------------------------

doxylink = {
    'nlsr': ('NLSR.tag', 'doxygen/'),
}

extlinks = {
    'issue': ('https://redmine.named-data.net/issues/%s', 'issue #%s'),
}
