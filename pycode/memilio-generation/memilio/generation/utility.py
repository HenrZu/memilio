#############################################################################
# Copyright (C) 2020-2023 German Aerospace Center (DLR-SC)
#
# Authors: Maximilian Betz
#
# Contact: Martin J. Kuehn <Martin.Kuehn@DLR.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################
"""
@file utility.py
@brief Additional functions used for the code generation.
"""
import os
import subprocess
from typing import Any, List, TextIO

import importlib_resources
from clang.cindex import Config, Cursor, Type


def try_set_libclang_path(path: str) -> None:
    """
    Try to set the file path for the libclang library. 
    If its already set, the returned Exception gets caught and discarded.
    If the given path string is empty or None, the path is determined with a call on the terminal.

    @param path Path to the library files of libclang. Can be an empty string.
    """
    # Check if path was set in config. If not, try to get it with cmd.
    if (not path or path == 'LIBCLANG_PATH-NOTFOUND'):
        clang_cmd = ["clang", '-print-file-name=']
        clang_cmd_result = subprocess.check_output(clang_cmd)
        path, dirname = os.path.split(clang_cmd_result)
        while ("llvm" not in str(dirname)):
            path, dirname = os.path.split(path)
            if str(path) == '':
                print("Error: define libclang path in config.json")
                raise (Exception)
        path = os.path.join(path.decode(), dirname.decode(),
                            "lib", "libclang.so")
    try:
        Config.set_library_file(path)
    except Exception as e:
        # Catch given exception from libclang. If the exact string output in libclang changes,
        # this needs to be adapted as well.
        if str(e) != "library file must be set before before using any other functionalities in libclang.":
            raise (e)


def try_get_compilation_database_path(skbuild_path_to_database: str) -> str:
    """
    Try to load the compile_commands.json ressource and retrieve the corresponding directory name.

    @param skbuild_path_to_database Value from config.json
    @return Path of directory
    """
    pkg = importlib_resources.files("memilio.generation")
    filename = skbuild_path_to_database.split('_skbuild')
    dirname = ""
    if (len(filename) > 1):
        with importlib_resources.as_file(
                pkg.joinpath("../../_skbuild" + filename[-1] +
                             "/compile_commands.json")) as path:
            dirname, _ = os.path.split(path)
    else:
        raise FileNotFoundError(
            'compile_commands.json could not be detected from skbuild path.')
    return dirname


def get_base_class(base_type: Type) -> List[Any]:
    """
    Retrieve the base class.
    Example for base_type: CompartmentalModel.

    @param Type of the current node.
    """
    result = [base_type.replace('> >', '>>')]
    for i in range(base_type.get_num_template_arguments()):
        result.append(get_base_class(base_type.get_template_argument_type(i)))
    return result


def get_base_class_string(base_type: Type) -> List[Any]:
    """
    Retrieve the spelling of the base class.
    Example for base_type.spelling: CompartmentalModel<mio::Populations<mio::AgeGroup, mio::InfectionState>, Parameters>.

    @param Type of the current node.
    """
    # fixes an issue in the generation of the abstract syntax tree
    # depending on the compiler version a whitespace is added between '>>'
    result = [base_type.spelling.replace('> >', '>>')]
    for i in range(base_type.get_num_template_arguments()):
        result.append(get_base_class_string(
            base_type.get_template_argument_type(i)))
    return result


def indent(spaces: int) -> str:
    """ 
    Indentation string for pretty-printing.

    @param spaces Number of spaces.
    """
    return '  '*spaces


def output_cursor_print(cursor: Cursor, spaces: int) -> None:
    """ 
    Low level cursor output to the terminal.

    @param cursor Represents the current node of the AST as an Cursor object from libclang.
    @param spaces Number of spaces.
    """
    spelling = ''
    displayname = ''

    if cursor.spelling:
        spelling = cursor.spelling
    if cursor.displayname:
        displayname = cursor.displayname
    kind = cursor.kind

    print(indent(spaces) + spelling, '<' + str(kind) + '>')
    print(indent(spaces+1) + '"' + displayname + '"')


def output_cursor_and_children_print(cursor: Cursor, spaces: int = 0) -> None:
    """ 
    Output this cursor and its children with minimal formatting to the terminal.

    @param cursor Represents the current node of the AST as an Cursor object from libclang.
    @param spaces [Default = 0] Number of spaces.
    """
    output_cursor_print(cursor, spaces)
    if cursor.kind.is_reference():
        print(indent(spaces) + 'reference to:')
        output_cursor_print(cursor.referenced, spaces+1)

    # Recurse for children of this cursor
    has_children = False
    for c in cursor.get_children():
        if not has_children:
            print(indent(spaces) + '{')
            has_children = True
        output_cursor_and_children_print(c, spaces+1)

    if has_children:
        print(indent(spaces) + '}')


def output_cursor_file(cursor: Cursor, f: TextIO, spaces: int) -> None:
    """ 
    Low level cursor output to a file.

    @param cursor Represents the current node of the AST as an Cursor object from libclang.
    @param f Open file object for output.
    @param spaces Number of spaces.
    """
    spelling = ''
    displayname = ''

    if cursor.spelling:
        spelling = cursor.spelling
    if cursor.displayname:
        displayname = cursor.displayname
    kind = cursor.kind

    f.write(indent(spaces) + spelling + ' <' + str(kind) + '> ')
    if cursor.location.file:
        f.write(cursor.location.file.name + '\n')
    f.write(indent(spaces+1) + '"' + displayname + '"\n')


def output_cursor_and_children_file(
        cursor: Cursor, f: TextIO, spaces: int = 0) -> None:
    """ 
    Output this cursor and its children with minimal formatting to a file.

    @param cursor Represents the current node of the AST as an Cursor object from libclang.
    @param f Open file object for output.
    @param spaces Number of spaces.
    """
    output_cursor_file(cursor, f, spaces)
    if cursor.kind.is_reference():
        f.write(indent(spaces) + 'reference to:\n')
        output_cursor_file(cursor.referenced, f, spaces+1)

    # Recurse for children of this cursor
    has_children = False
    for c in cursor.get_children():
        if not has_children:
            f.write(indent(spaces) + '{\n')
            has_children = True
        output_cursor_and_children_file(c, f, spaces+1)

    if has_children:
        f.write(indent(spaces) + '}\n')
