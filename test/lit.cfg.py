# -*- Python -*-

# Configuration file for the 'lit' test runner.

import os
import sys
import re
import platform
import subprocess

import lit.util
import lit.formats
from lit.llvm import llvm_config

# Targets
config.targets = frozenset(config.targets_to_build.split())

# name: The name of this test suite.
config.name = 'topt'

# testFormat: The test format to use to interpret tests.
#
# For now we require '&&' between commands, until they get globally killed and
# the test runner updated.
execute_external = platform.system() != 'Windows'
config.test_format = lit.formats.ShTest(execute_external)

# suffixes: A list of file extensions to treat as test files. This is overriden
# by individual lit.local.cfg files in the test subdirectories.
config.suffixes = ['.ll']

# excludes: A list of directories to exclude from the testsuite. The 'Inputs'
# subdirectories contain auxiliary inputs for various tests in their parent
# directories.
config.excludes = ['Inputs', 'CMakeLists.txt', 'README.txt', 'LICENSE.txt']

# test_source_root: The root path where tests are located.
config.test_source_root = os.path.dirname(__file__)

# test_exec_root: The root path where tests should be run.
config.test_exec_root = os.path.join(config.topt_obj_root, 'test')

# Tweak the PATH to include the tools dir and the scripts dir.
base_paths = [config.llvm_tools_dir, config.environment['PATH']]
path = os.path.pathsep.join(base_paths)
config.environment['PATH'] = path

path = os.path.pathsep.join((config.llvm_libs_dir,
                             config.environment.get('LD_LIBRARY_PATH','')))
config.environment['LD_LIBRARY_PATH'] = path
