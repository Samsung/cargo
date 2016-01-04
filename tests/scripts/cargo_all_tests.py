#!/usr/bin/env python

import cargo_launch_test
import sys

# insert other test binaries to this array
_testCmdTable = ["cargo-unit-tests"]

for test in _testCmdTable:
    cargo_launch_test.launchTest([test] + sys.argv[1:])
