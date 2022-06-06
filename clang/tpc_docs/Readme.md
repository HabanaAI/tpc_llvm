POC for generating rst equivalent of doxygen output
===================================================

For documentation purpose, most of the projects under npu_stack generates rst file, which is then converted to html/pdf using sphinx.
This gives a uniform look and feel to all the habana documents. However, tpc_intrinsics.h/tpc_defs.h makes use of doxygen for documenting their code.

Doxygen currently does not support output in the rst format. But because of the merits of doxygen, we cannot do away with this tool.
So, we will be using doxyrest, an open source tool maintained by Tibbo Technology Inc. licensed under MIT License, which can convert the doxygen output into rst format.

The flow will be like this:


*.h -------\       ___________               ____________             __________
*.c -------\\_____|           |             |            |           |          |
            \_____|           |   *.xml     |            |  *.rst    |          |  *.html/.pdf
             _____|  Doxygen  |------------>|  Doxyrest  |---------->|  Sphinx  |---------------->
            /_____|           |             |            |           |          |
*.hpp -----//     |___________|             |____________|           |__________|
*.cpp -----/


Tools Installation
==================
1. Doxygen
   Install command: $sudo apt-get install doxygen
   To check if doxygen is installed successfully: $dpkg -l | grep doxygen
   For more info: https://www.doxygen.nl/manual/starting.html

2. Doxyrest
   Download the latest binaries from https://github.com/vovkos/doxyrest/releases
   Check if all the libs requirement of doxyrest is satisfied or not: ldd doxyrest-$ver-linux-amd64/bin/doxyrest
   For more info: https://github.com/vovkos/doxyrest

3. Sphinx
   $sudo pip install sphinx
   $sudo pip install sphinx-rtd-theme
   To check if sphinx is installed properly: $pip show sphinx
   For more info: https://docs.readthedocs.io/en/stable/intro/getting-started-with-sphinx.html


Configuration Settings
======================
1. Doxygen
      generate a configuration file: $doxygen -g $config_filename
      this will generate a default configuration file.

      Important fields to be modified:
            # Obviously, we do need XML:
            GENERATE_XML = YES

            # Next, choose the location of the resulting XML database:
            XML_OUTPUT = xml

            # Program listing vastly increases the size of XML so it's recommended
            # to turning it OFF:
            XML_PROGRAMLISTING = NO

            # The next one is essential! Sphinx uses lowercase reference IDs,
            # so Doxygen can't use mixed-case IDs:
            CASE_SENSE_NAMES = NO

            # The next one is important for C++ projects -- otherwise Doxygen
            # may generate lots of bogus links to template arguments:

            HIDE_UNDOC_RELATIONS = YES

            # The last one is not essential, but recommended if your project
            # sets AUTOLINK_SUPPORT to ON (like most projects do) -- otherwise
            # auto-generated links may point to discarded items:

            EXTRACT_ALL = YES

            # The most important one, INPUT. SPecify the files which contains
            # the documented code
            INPUT = "../../../../tpc_llvm10/llvm/docs/TPC/README_TPC.md" \
                    "../../../../tpc_llvm10/clang/lib/Headers/tpc-defs.h" \
                    "../../../../tpc_llvm10/clang/lib/Headers/tpc-intrinsics.h"

      config file used by tpc llvm project: tpc_llvm10/llvm/docs/TPC/tpcConfig.conf
      Detailed information on config fields: https://www.doxygen.nl/manual/config.html

2. Doxyrest
   a. Copy the default doxyrest config file from doxyrest-$version-linux-amd64/share/doxyrest/frame/doxyrest-config.lua and adjust the following settings:

      -- Specify input and output paths:
      OUTPUT_FILE = "rst/index.rst"
      INPUT_FILE = "xml/index.xml"
      FRAME_FILE = "index.rst.in"
      FRAME_DIR_LIST = { "doxyrest-frame-dir/cfamily", "doxyrest-frame-dir/common" }

      -- Usually, Doxygen-based documentation has a main page (created with
      -- the \mainpage directive). If that's the case, force-include
      -- the contents of 'page_index.rst' into 'index.rst':
      INTRO_FILE = "page_index.rst"

      -- If your documentation uses \verbatim directives for code snippets
      -- you can convert those to reStructuredText C++ code-blocks:
      VERBATIM_TO_CODE_BLOCK = "cpp"

      -- Asterisks, pipes and trailing underscores have special meaning in
      -- reStructuredText. If they appear in Doxy-comments anywhere except
      -- for code-blocks, they must be escaped:
      ESCAPE_ASTERISKS = true
      ESCAPE_PIPES = true
      ESCAPE_TRAILING_UNDERSCORES = true

   For more info on tags please refer: https://vovkos.github.io/doxyrest/manual/group_frame-config.html

3. Sphinx
   a. Copy the following contents in a conf.py file and place in the folder which contains rst files

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
   sys.path.append(os.path.abspath('../doxyrest-2.1.2-linux-amd64/share/doxyrest/sphinx'))


   # -- Project information -----------------------------------------------------

   project = 'TPC Intrinsics'
   copyright = '2021'
   author = 'Habana Labs'

   # The full version, including alpha/beta/rc tags
   release = '1.0'


   # -- General configuration ---------------------------------------------------

   # Add any Sphinx extension module names here, as strings. They can be
   # extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
   # ones.
   # Add Doxyrest extensions ``doxyrest`` and ``cpplexer``:
   extensions = ['doxyrest', 'cpplexer', 'sphinx_rtd_theme']

   # Add any paths that contain templates here, relative to this directory.
   templates_path = ['_templates']

   # List of patterns, relative to source directory, that match files and
   # directories to ignore when looking for source files.
   # This pattern also affects html_static_path and html_extra_path.
   # If you used INTRO_FILE in 'doxyrest-config.lua' to force-include it
   # into 'index.rst', exclude it from the Sphinx input (otherwise, there
   # will be build warnings):

   exclude_patterns = ['page_index.rst']


   # -- Options for HTML output -------------------------------------------------

   # The theme to use for HTML and HTML Help pages.  See the documentation for
   # a list of builtin themes.
   #
   html_theme = 'sphinx-rtd-theme'

   latex_engine = 'pdflatex'

   b. The same can be generated by running sphinx-quickstart


Running the pipeline
====================
1. doxygen $config_filename
2. doxyrest-$version-linux-amd64/bin/doxyrest -c doxyrest-config.lua
3. sphinx-build -b html $source $dest - for generating html
4. sphinx-build -M latexpdf $source $dest - for generating pdf


Example
=======
cd ~/trees/npu-stack/tpc_kernels/playground/amittal/intrinsics_documentation
doxygen doxygen.conf
doxyrest-2.1.2-linux-amd64/bin/doxyrest -c doxyrest-config.lua
sphinx-build -b html rst doxyrest_html - for generating html

Command to run Sphinx using rst2pdf pdfbuilder
==============================================
sphinx-build -b pdf $src $dest
