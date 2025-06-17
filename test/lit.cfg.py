import os
import lit.formats

config.name = "c2pulse-tests"
config.test_format = lit.formats.ShTest()
config.suffixes = [".c"]
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = config.test_source_root

# Get the paths from environment variables or provide default fallback
c2pulse_path = os.environ.get("C2PULSE_PATH", os.path.abspath("../../CtoPulse/build/c2pulse"))
filecheck_path = os.environ.get("FILECHECK", os.path.abspath("../../external_tools/llvm-project/build/bin/FileCheck"))

# Substitutions
config.substitutions.append(("%c2pulse", c2pulse_path))
config.substitutions.append(("%{FILECHECK}", filecheck_path))
