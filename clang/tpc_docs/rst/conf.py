# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#

import os
import sys

sys.path.append(os.path.abspath('.'))
version = ""

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

extensions = ['sphinx_rtd_theme', 'rst2pdf.pdfbuilder']

# -- Options for PDF output -------------------------------------------------
pdf_documents = [('index', u'TPCIntrinsics', u'TPC Intrinsics', u'Habana Labs'),]
pdf_stylesheets = ['style.style']
pdf_language = "en_US"
pdf_fit_mode = "shrink"
pdf_break_level = 1
pdf_use_index = False
pdf_use_modindex = False
pdf_use_coverpage = True
pdf_cover_template = 'cover.tmpl'
pdf_page_template = 'cutePage'
pdf_use_toc = True


# -- Options for HTML output -------------------------------------------------
html_theme = 'sphinx_rtd_theme'

latex_engine = 'pdflatex'