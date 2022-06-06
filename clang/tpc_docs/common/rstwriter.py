# *****************************************************************************
# Copyright (C) 2020 HabanaLabs, Ltd.
# All Rights Reserved.
#
# Unauthorized copying of this file, via any medium is strictly prohibited.
# Proprietary and confidential.
#
# Author:
# amittal@habana.ai
# ******************************************************************************
import os
class RstWriter():

    def __init__(self, is_sphinx_mode):
        self._data = []
        self.is_sphinx_mode = is_sphinx_mode

    def get_data(self):
        """
        Returns the class variable _data
        """
        return self._data

    def print_data(self):
        """
            Prints the content in _data on the standard console
        """
        print('\n'.join(self._data))

    def create_rst_file(self, filename):
        """
        Writes the content in _data to a file.
        Parameter:
            filename - path of the file in which data has to be written
        """
        with open(filename, 'w') as f:
            f.write('\n'.join(self._data))
            f.write('\n')

    def sphinx_toc_tree(self):
        """
        Appends toc directive in the format required by sphinx
        """
        directive = '.. toctree::\n  :maxdepth: 2\n  :caption: Contents:\n'
        self._update_data(directive)

    def table_of_contents(self, title = "Table of Contents", numbered = True, depth = -1):
        """
        Appends content directive to the _data list which is responsible for creating table of contents
        Parameters:
            title(str):     Title of the contents table(Default is "Table of Contents")
            numbered(bool): Whether the sections will be numbered or not. Default is True. If false then bullets will be used
                            instead of numbers.
            depth(int):     Specifies upto which level the sub headings will be included in the table
                            (Default is -1, i.e., all sub headings will be included)
        """
        directive = ''
        if numbered:
            directive += '.. sectnum::\n'
        self._update_data(directive)

    def title(self, text):
        """
        Adds document title to _data
        Parameters:
            text:   title of document
        """
        if self.is_sphinx_mode:
            text = text.strip().split('\n')
            heading = '\n'.join('#' * len(para.strip()) + '\n' + para.strip() + '\n' + '#' * len(para.strip()) for para in text if para.strip())
        else:
            indent = 1
            paragraph = '\n\n'.join(' ' * indent + para.strip() for para in text.strip().split('\n'))
            paragraph += '\n'
            heading = '.. class:: title\n\n' + paragraph
        self._update_data(heading)

    def subtitle(self, text):
        """
        Adds document subtitle to _data
        Parameters:
            text:   subtitle of document
        """
        indent = 1
        paragraph = '\n\n'.join(' ' * indent + para.strip() for para in text.strip().split('\n'))
        paragraph += '\n'
        heading = '.. class:: subtitle\n\n' + paragraph
        self._update_data(heading)

    def customText(self, text, customClass):
        indent = 1
        paragraph = '\n\n'.join(' ' * indent + para.strip() for para in text.strip().split('\n'))
        paragraph += '\n'
        content = '.. class:: ' + customClass + '\n\n' + paragraph
        self._update_data(content)

    def subtitle2(self, text):
        """
        Adds document subtitle to _data
        Parameters:
            text:   subtitle of document
        """
        indent = 1
        paragraph = '\n\n'.join(' ' * indent + para.strip() for para in text.strip().split('\n'))
        paragraph += '\n'
        heading = '.. class:: subtitle2\n\n' + paragraph
        self._update_data(heading)

    def h1(self, text):
        """
        Adds h1 heading to _data
        Parameters:
            text:   Heading title
        """
        text = text.strip()
        heading = '\n'.join([text, '*' * len(text)])
        self._update_data(heading)

    def h2(self, text):
        """
        Adds h2 heading to _data
        Parameters:
            text:   Heading title
        """
        text = text.strip()
        heading = '\n'.join([text, '~' * len(text)])
        heading = '.. raw:: pdf\n\n FrameBreak 100\n\n' + heading
        self._update_data(heading)

    def h3(self, text):
        """
        Adds h3 heading to _data
        Parameters:
            text:   Heading title
        """
        text = text.strip()
        heading = '\n'.join([text, '=' * len(text)])
        self._update_data(heading)

    def h4(self, text):
        """
        Adds h4 heading to _data
        Parameters:
            text:   Heading title
        """
        text = text.strip()
        heading = '\n'.join([text, '-' * len(text)])
        self._update_data(heading)

    def h5(self, text):
        """
        Adds h5 heading to _data
        Parameters:
            text:   Heading title
        """
        text = text.strip()
        heading = '\n'.join([text, '+' * len(text)])
        self._update_data(heading)

    def h6(self, text):
        """
        Adds h6 heading to _data
        Parameters:
            text:   Heading title
        """
        text = text.strip()
        heading = '\n'.join([text, '^' * len(text)])
        self._update_data(heading)

    def paragraph(self, text, indent = 0):
        """
        Adds paragraphs to _data.
        Parameters:
            text:   string of characters to be inserted as paragraph.
                    If there are multiple paragraphs then each should be delimited by newline(\n) character.
            indent: integer representing the required indentation of each paragraph.
                    Default is 0, i.e., no indentation.
        Output Requirements:
            Paragraph should end with a blank line.
        """
        text = text.split('\n')
        paragraph = '\n\n'.join(' ' * indent + para.strip() for para in text if para.strip())
        paragraph += '\n'
        frameBreakPt = 50
        if "Private Structure:" in paragraph:
            frameBreakPt = 100
        if (not self.is_sphinx_mode):
            paragraph = '.. raw:: pdf\n\n FrameBreak {0}\n\n'.format(frameBreakPt) + paragraph
        self._update_data(paragraph)

    def new_line(self):
        """
        Adds new line break to _data.
        """
        self._update_data('\n|\n')

    def insert_code(self, code, lang = "C++"):
        """
        Appends code to _data
        Parameters:
            code(str):  piece of code
            lang(str):  code language. Default is "C++"
        """
        directive = '.. code-block:: {0}'.format(lang) + '\n\n '
        indented_code = '\n '.join(code.split('\n'))
        indented_code = indented_code.replace('       //','//')
        code_block = directive + indented_code + '\n'
        self._update_data(code_block)

    def new_section(self):
        """
        Adds new section to _data
        """
        section_separator = '\n' + '-' * 5 + '\n'
        self._update_data(section_separator)

    def get_unordered_list_str(self, uno_list, indent = 0):
        """
        Constructs the string for representing uno_list in rst format.
        Parameters:
            uno_list(list): list of strings. Can be nested list also.
            indent(int):    left indentation. Default is 0
        Returns:
            list_str(str):  string representation of uno_list in rst format
        Output Requirements:
            List should end with a new line.
            Each sublist should have a new line at the beginning and at the end of list.
        """
        list_str = '\n'
        for i, li in enumerate(uno_list):
            if isinstance(li, list):
                list_str += self.get_unordered_list_str(li, indent + 2)
                if i != len(uno_list) - 1:
                    list_str += '\n'
            else:
                li = li.split('\n')
                list_str += ' ' * indent + '- ' + li[0].strip()
                for c in range(1, len(li)):
                    list_str += '\n\n' + ' ' * indent + '  ' + li[c].strip()
                list_str += '\n'
        return list_str

    def unordered_list(self, uno_list, indent = 0):
        """
        Adds string for representing uno_list in rst format to _data.
        Parameters:
            uno_list(list): list of strings. Can be nested list also.
            indent(int):    left indentation. Default is 0
        """
        list_str = self.get_unordered_list_str(uno_list, indent)
        self._update_data(list_str)

    def _pad(self, text, length, char = ' '):
        """
        Pads the text with character to make it of specified length
        Parameter:
            text(str):      data which needs to be padded
            length(int):    length of the padded string
            char(char):     character with which the text is padded. Default is space(' ')
        Returns:
            if length is greater than the length of the text then padded string else the input string.
        """
        if length <= len(text):
            return text
        text = text + char * (length - len(text))
        return text

    def list_table(self, data, header_row = [], widths = "", indent = 0, caption = ""):
        starting_spaces = ' ' + ' ' * indent
        tdata = starting_spaces
        if header_row:
            ## Between each row there should be a blank line
            tdata += '\n' + starting_spaces + '* '
            for col in header_row:
                ## each column begins on a new line and has the format:
                ## "starting_spaces   [2 spaces for col_sep alignment]- [1 space for col text alignment]text"
                tdata += '\n' + starting_spaces + '  - '
                ## If it is a multiline column then there should be a blank line between each line and
                ## trailing spaces should be equal to len(starting_spaces   [2 spaces for col_sep alignment]- [1 space for col text alignment])
                for i,line in enumerate(col.split('\n')):
                    line = line.strip()
                    if i == 0:
                        tdata += '\n' + starting_spaces + '    ' + line
                    else:
                        tdata += '\n\n' + starting_spaces + '    ' + line

        for row in data:
            ## Spacing is same as above
            tdata += '\n' + starting_spaces + '* '
            for col in row:
                tdata += '\n' + starting_spaces + '  - '
                for i,line in enumerate(col.split('\n')):
                    line = line.strip()
                    if i == 0:
                        tdata += '\n' + starting_spaces + '    ' + line
                    else:
                        tdata += '\n\n' + starting_spaces + '    ' + line

        table_str = '.. list-table:: {0}\n'.format(caption)
        if widths:
            table_str += ' :widths: {0}\n'.format(widths)
        if header_row:
            table_str += ' :header-rows: 1\n'
        table_str += tdata + '\n'
        self._update_data(table_str)

    def _construct_table(self, num_rows, num_cols, data, header = [], indent = 0):
        """
        Contstructs the rst string for table
        Parameters:
            num_rows(int):      number of rows in table
            num_cols(int):      number of columns in table
            data(list of list): list of list where each sub-list represents a row in table
            header(list):       table header. Default is none.
            indent(int):        left indentation. Default is 0
        Returns:
            rst equivalent string for grided table
        """
        tdata = ' ' + ' ' * indent
        row_separator = '+'
        header_separator = '+'
        th = '|'
        col_len = [0] * num_cols
        for i in range(num_cols):
            col_len[i] = max([len(rd) for row in data for rd in row[i].split('\n')])
            if header:
                col_len[i] = max(col_len[i], len(header[i]))
                th += self._pad(header[i], col_len[i]) + '|'
                header_separator += '=' * col_len[i] + '+'
            row_separator += '-' * col_len[i] + '+'
        if header:
            tdata += ('\n ' + ' ' * indent).join([row_separator, th, header_separator]) + ('\n ' + ' ' * indent)
        else:
            tdata += row_separator + ('\n ' + ' ' * indent)

        for row in data:
            tr = '|'
            multi_line_row = []
            for col in row:
                multi_line_row.append(col.split('\n'))
            ## Finds the maximum lines any column in a row spans to
            max_line = max([len(col) for col in multi_line_row])
            for j in range(max_line):
                if j != 0:
                    tr += '\n ' + ' ' * indent + '|'
                for i, cell in enumerate(multi_line_row):
                    if j < len(cell):
                        tr += self._pad(cell[j], col_len[i]) + '|'
                    else:
                        tr += self._pad(' ', col_len[i]) + '|'
            tdata += ('\n ' + ' ' * indent).join([tr, row_separator]) + ('\n ' + ' ' * indent)
        return tdata

    def table(self, num_rows, num_cols, data, header = [], width = "", isGUIDtable=False, isSummary=False,  caption= "", isDescTable=False, isExt=False):
        """
        Adds the table to _data
        Parameters:
            num_rows(int):      number of rows
            num_cols(int):      number of columns
            data(list of list): list of list where each sub-list represents a row in table
            header(list):       table header. Default is none.
            width(int):         width of each column
            isGUIDtable(bool):  whether table is for GUID or not. Default is False
            caption(string):    caption for the table (optional)
            isSummary(bool):    whether a table is for summary or not. Default is False
        """
        table_str = ""
        if isSummary and (not isExt):
            table_str = '\n.. class:: summarytable\n\n'
        elif isGUIDtable and (not isExt):
            table_str = '\n.. class:: guidtable\n\n'
        elif isDescTable:
            if caption :
                if "Cast" in caption :
                    if (not isExt):
                        table_str = '\n.. class:: casttable\n.. table:: {0}\n\n'.format(caption)
                    else:
                        table_str = '\n.. table:: {0}\n\n'.format(caption)
                else :
                    if (not isExt):
                        table_str = '\n.. class:: desctable\n.. table:: {0}\n\n'.format(caption)
                    else:
                        table_str = '\n.. table:: {0}\n\n'.format(caption)
            else :
                if not isExt:
                    table_str = '\n.. class:: desctable\n\n'
        elif width:
            table_str = '\n.. table:: {0}\n :widths: {1}\n\n'.format(caption,width)
        else:
            table_str = '.. table:: {0}\n\n'.format(caption)
        table_str += self._construct_table(num_rows, num_cols, data, header)
        if isGUIDtable and (not isExt):
            table_str = ".. raw:: pdf\n\n FrameBreak 50\n\n" + table_str
        self._update_data(table_str)

    def field_list(self, field, value):
        """
        Adds field_list to _data
        """
        line = ':{0}: {1}'.format(field.strip(), value.strip()) + '\n'
        self._update_data(line)

    def definition_list(self, term, value):
        """
        Adds field_list to _data
        """
        value = value.replace('\n', ' ')
        line = '\t*{0}*\n \t\t{1}'.format(term.strip(), value.strip()) + '\n'
        self._update_data(line)

    def get_bold(self, text):
        """
        Returns the thext in bold
        """
        return '**{0}**'.format(text)

    def insert_image(self, path, width = "", height = "", align = "", caption = ""):
        directive = '.. figure:: ' + path + '\n'
        if width:
            directive += ' :width: '+ width + '\n'
        if align:
            directive += ' :align: ' + align + '\n'
        if height:
            directive += ' :height: ' + height + '\n'
        if caption:
            directive += '\n Figure :counter:`figure`. ' + caption + '\n'
        self._update_data(directive)

    def insert_formula(self, equation, label = ""):
        directive = '\n.. math::\n\n '
        eq = '\n '.join(line for line in equation.split('\n'))
        directive += eq + '\n\n'
        # if label:
        #     directive += ' :label: ' + label + '\n'
        self._update_data(directive)

    def page_break(self, mode=0, template=''):
        """
        Break to a new page
        Parameters:
            mode(int):      1: Break to next odd numbered page
                            2: Break to next even numbered page
                            default: Break to next page
            template(str):  coverPage, oneColumn(default), twoColumn, threeColumn, cutePage
        """
        pb = 'PageBreak'
        if mode == 1:
            pb = 'OddPageBreak'
        elif mode == 2:
            pb = 'EvenPageBreak'
        directive = '.. raw:: pdf\n\n {0} {1}\n'.format(pb, template)
        self._update_data(directive)

    def page_counter(self, start_number=1, template='arabic'):
        """
        Counter to the page
        Parameters:
            start_number(int):  1: Page number to be started with.
                                default: Break is 1.
            template(str):      arabic, lowerroman, roman, loweralpha, alpha
                                default: arabic
        """
        pb = 'SetPageCounter'
        directive = '.. raw:: pdf\n\n {0} {1} {2}\n'.format(pb, start_number, template)
        self._update_data(directive)

    def _header_or_footer(self, content):
        """
        Constructs rst string for header or footer
        Parameters:
            content(str or list):  text to be inserted as header or footer
        Returns:
            constructed rst string
        """
        directive = ' .. class:: headerfootertable\n\n'
        if not isinstance(content, list):
            content = [content]
        content_with_align = []
        for i, part in enumerate(content):
            align = 'center'
            if i == 0:
                align = 'left'
            elif (i + 1) == len(content):
                align = 'right'
            p_w_a = '.. class:: {0}\n\n {1}'.format(align, part)
            content_with_align.append(p_w_a)
        table_str = directive + self._construct_table(1, len(content), [content_with_align], indent = 1)
        return table_str

    def _header(self, content):
        """
        Constructs rst string for header
        Parameters:
            content(str or list):  text to be inserted as header
        Returns:
            constructed rst string
        """
        directive = ' .. class:: headertable\n\n'
        if not isinstance(content, list):
            content = [content]
        content.append("  |logo|")
        content_with_align = []
        align = 'left'
        p_w_a = '.. class:: {0}\n\n{1}'.format(align, content[0])
        p_w_b = '.. class:: {0}\n\n{1}'.format('right', content[1])
        content_with_align.append(p_w_a)
        content_with_align.append(p_w_b)
        table_str = directive + self._construct_table(1, len(content), [content_with_align], indent = 1)
        COMPANY_ICON_PATH = os.path.join(os.environ["TPC_KERNELS_ROOT"], "doc/common/habana-Icon_BLUE.png")
        self.substitution("\n.. |logo| image:: {0}".format(COMPANY_ICON_PATH) + "\n\t:width: 1cm\n")
        return table_str

    def _footer(self, content):
        """
        Constructs rst string for footer
        Parameters:
            content(str or list):  text to be inserted as footer
        Returns:
            constructed rst string
        """
        directive = ' .. class:: footertable\n\n'
        content_with_align = []
        for i, part in enumerate(content):
            align = 'center'
            if i == 0:
                align = 'left'
            elif (i + 1) == len(content):
                align = 'right'
            p_w_a = '.. class:: {0}\n\n {1}'.format(align, part)
            content_with_align.append(p_w_a)
        table_str = directive + self._construct_table(1, len(content), [content_with_align], indent = 1)
        return table_str

    def header(self, text = ""):
        """
        Inserts header
        Parameters:
            text(str):  text to be inserted as header
        """
        directive = '.. header::\n\n'
        header = directive + self._header(text)
        self._update_data(header)

    def footer(self, text = ""):
        """
        Inserts footer
        Parameters:
            text(str):  text to be inserted as footer
        """
        directive = '.. footer::\n\n'
        footer = directive + self._footer(text)
        self._update_data(footer)

    def substitution(self, sub):
        self._update_data(sub)

    def _update_data(self, content):
        """
        Adds content to _data
        """
        if isinstance(content, list):
            self._data.extend(content)
        else:
            self._data.append(content)
