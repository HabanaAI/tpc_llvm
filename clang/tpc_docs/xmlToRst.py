import xml.etree.ElementTree as ET

## imports for system utilties
from sys import argv, path, exit
from os import listdir, environ, getcwd
from os.path import isfile, join, expandvars, basename, isdir, exists, dirname, abspath
from subprocess import check_output

script_dir = dirname(abspath(__file__))
cwd_dir = getcwd()
path.append(join(script_dir, "doc"))

## import for rst infrastructure
from common.rstwriter import RstWriter

def isList(tag):
    if tag == "itemizedlist":
        return True
    return False

def parseList(itemized_list):
    parsed_list = []
    for para_item in itemized_list.findall('listitem/para'):
        content  = para_item.text
        for child in para_item:
            if isList(child.tag):
                sublist = parseList(child)
                if content.strip():
                    parsed_list.append(content)
                parsed_list.append(sublist)
                content = ""
            else:
                for line in child.itertext():
                    content = content + line
        if content.strip():
            parsed_list.append(content.strip())
    return parsed_list


def get_branch_version():
    gitroot = expandvars('$TPC_LLVM_ROOT/../.git')
    curr_branch = check_output(["git", "--git-dir",  gitroot, "rev-parse", "--abbrev-ref", "HEAD"]).decode("utf8")
    return curr_branch.strip()

################################################################################################
## Contructs dictionary from xml data with section name as specified in tpc-intrinsics.h as key
################################################################################################
def get_section_wise_content():
    section_xml_dict = dict()
    for section_def in root.iter('sectiondef'):
        if section_def.attrib['kind'] == 'user-defined':
            section_title = ""
            sect1_element = section_def.find('memberdef/detaileddescription/sect1')
            if not sect1_element:
                print(section_def, " does not have any section name and will be skipped!!")
                continue
            section_title = sect1_element.find('title').text
            if section_title in section_xml_dict:
                section_xml_dict[section_title].append(section_def)
            else:
                section_xml_dict[section_title] = [section_def]
    return section_xml_dict
################################################################################################


################################################################################################
## Parses the xml section wise and converts it into equivalent rst format
################################################################################################
def parse_xml(section_xml_dict):
    for section in section_xml_dict:
        rst.h1(section)
        for subsection in section_xml_dict[section]:
            subsection_title = ""
            brief_description = ""
            detailed_description = ""
            subsection_intrinsics_prototype = []
            parameters = []
            return_val_description = ""
            ## Accumulate the prototype of all the intrinsics belonging to one group
            for member_def_ele in subsection:
                definition_ele = member_def_ele.find('definition')
                args_string_ele = member_def_ele.find('argsstring')
                intrinsic_prototype = definition_ele.text.strip() + args_string_ele.text.strip()
                subsection_intrinsics_prototype.append([intrinsic_prototype])

            # Since DISTRIBUTE_GROUP_DOC flag in doxygen.conf file is set to No,
            # therefore only first member in each section contains description
            first_member_def_ele = subsection[0]
            detailed_description_ele = first_member_def_ele.find('detaileddescription')

            sect1_ele = detailed_description_ele.find('sect1')
            innermost_section_ele = sect1_ele
            sect2_ele = sect1_ele.find('sect2')
            ## If subsection title is not present and more than one groups have same section title then
            ## throw an error
            if not sect2_ele and len(section_xml_dict[section]) > 1:
                print("One or more subsections of section ", section, " are not titled!!")
                exit
            ## If subsection title is present
            if sect2_ele:
                subsection_title = sect2_ele.find('title').text
                rst.h2(subsection_title)
                innermost_section_ele = sect2_ele

            ## Get intrinsics brief description
            brief_description_ele = first_member_def_ele.find('briefdescription/para')
            for line in brief_description_ele.itertext():
                brief_description = brief_description + line
            if brief_description.strip():
                rst.paragraph(brief_description.strip())

            ## Get intrinsics detailed description
            for simple_sect_ele in innermost_section_ele.findall('para/simplesect'):
                detailed_description += simple_sect_ele.tail
            if detailed_description.strip():
                rst.paragraph(detailed_description.strip())

            if subsection_intrinsics_prototype:
                rst.paragraph("**Intrinsics Definition:**")
                rst.table(len(subsection_intrinsics_prototype), 1, subsection_intrinsics_prototype)

            ## Parse parameter name and parameter description
            for parameter_item_ele in first_member_def_ele.iter('parameteritem'):
                parameter_name = parameter_item_ele.find('parameternamelist/parametername').text
                parameter_description = ""
                for line in parameter_item_ele.find('parameterdescription/para').itertext():
                    parameter_description = parameter_description + line
                parameters.append([parameter_name, parameter_description])
            if(parameters):
                rst.paragraph("**Parameters:**")
                rst.list_table(parameters, widths="20 80")

            for simple_sect_ele in innermost_section_ele.findall('para/simplesect'):
                if(simple_sect_ele.attrib['kind'] == 'return'):
                    for line in simple_sect_ele.find('para').itertext():
                        return_val_description = return_val_description + line
                elif (simple_sect_ele.attrib['kind'] == 'par'):
                    title = simple_sect_ele.find('title')
                    if title.text == "Allowed switches:":
                        rst.paragraph("**Switches:**")
                        para_ele = simple_sect_ele.find('para')
                        if para_ele:
                            if para_ele.text:
                                if para_ele.text.strip():
                                    rst.paragraph(para_ele.text.strip())
                        for para_child in para_ele:
                            if isList(para_child.tag):
                                allowed_switches = parseList(para_child)
                            rst.unordered_list(allowed_switches)

            if(return_val_description):
                rst.paragraph("**Returns:**")
                rst.paragraph(return_val_description)
################################################################################################

asic = argv[1]
COMPANY_LOGO_PATH = join(script_dir, "common/Habana_Intel_Blue.png")
branch_version = get_branch_version()
revision = '1.0'

## Generate cover.tmpl
with open(join(script_dir, "rst/cover.tmpl"), 'w') as f:
    f.write('.. role:: normal\n\n.. cssclass:: title\n\n| SynapseAI ')
    f.write(branch_version)
    f.write('\n| ')
    f.write(asic.title())
    f.write(' TPC Intrinsics\n\n.. cssclass:: subtitle\n\nRevision ')
    f.write(revision)
    f.write('\n\n.. image:: ')
    f.write(COMPANY_LOGO_PATH)
    f.write('\n   :height: 2cm\n\n.. cssclass:: center\n\nConfidential and Proprietary Information')
    f.write('\n')

rst = RstWriter(True)   # Instantiates RstWriter which contains utility functions for inserting text in rst format
rst.table_of_contents()
tree = ET.parse(join(cwd_dir, "xml/tpc-intrinsics_8h.xml"))
root = tree.getroot()

content_dict = {}

section_xml_dict = get_section_wise_content()
parse_xml(section_xml_dict)

rst.create_rst_file(join(cwd_dir, "rst/index.rst"))

