# Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All rights reserved.
from optparse import OptionParser
import os
import subprocess

# Option help messages.
HELP_MSG_AMDISA_TEST_EXE_PATH = 'AMDISA unit test executable path'
HELP_XML_FILE_PATH = 'AMDISA XML file path'
HELP_XML_DIR_PATH_OUTPUT = 'AMDISA XML files directory path'

# Error messages.
ERR_MSG_EXE_DOES_NOT_EXIST = 'AMDISA tests executable does not exists: {}'
ERR_MSG_XML_FILE_DOES_NOT_EXIST = 'XML file path does not exists: {}'
ERR_MSG_XMLS_DIR_NOT_EXIST = 'XMLs directory path does not exists: {}'

# Executable command
AMDISA_TEST_CMD = '{} -I {}'

def parse_args():
    parser = OptionParser()
    parser.add_option("-e", "--test-path", dest="amdisa_test_exe", help=HELP_MSG_AMDISA_TEST_EXE_PATH)
    parser.add_option("-f", "--xml-file", dest="xml_file_path", help=HELP_XML_FILE_PATH)
    parser.add_option("-d", "--dir", dest="xmls_dir_path", help=HELP_XML_DIR_PATH_OUTPUT)
    (options, args) = parser.parse_args()
    
    is_parse_successful = True
    exec_path = "" 
    xml_file = ""
    xml_files_dir = ""
    
    if not options.amdisa_test_exe is None:
        if os.path.exists(options.amdisa_test_exe):
            exec_path = options.amdisa_test_exe
        else:
            print(ERR_MSG_EXE_DOES_NOT_EXIST.format(options.amdisa_test_exe))
            is_parse_successful = False
            
    if not options.xml_file_path is None:
        if os.path.exists(options.xml_file_path):
            xml_file = options.xml_file_path
        else:
            print(ERR_MSG_XML_FILE_DOES_NOT_EXIST.format(options.xml_file_path))
            is_parse_successful = False
            
    if not options.xmls_dir_path is None:
        if os.path.exists(options.xmls_dir_path):
            xml_files_dir = options.xmls_dir_path
        else:
            print(ERR_MSG_XMLS_DIR_NOT_EXIST.format(options.xmls_dir_path))
            is_parse_successful = False
    
    if not is_parse_successful:
        parser.print_help()
        
    return is_parse_successful, exec_path, xml_file, xml_files_dir
    
def test_xml_file(exec_path, xml_file):
    file_name = os.path.basename(xml_file).split('/')[-1]
    if file_name.endswith(".xml"):
        cli_command = AMDISA_TEST_CMD.format(exec_path, xml_file)
        subprocess.run(cli_command)
    
def test_xml_files_dir(exec_path, xml_files_dir):
    for xml_file in os.listdir(xml_files_dir):
        test_xml_file(exec_path, os.path.join(xml_files_dir, xml_file))
    
def main():    
    is_parse_successful, exec_path, xml_file, xml_files_dir = parse_args()   
    if is_parse_successful:
        if xml_file:
            test_xml_file(exec_path, xml_file)
        if xml_files_dir:
            test_xml_files_dir(exec_path, xml_files_dir)
            
if __name__ == "__main__":
    main()