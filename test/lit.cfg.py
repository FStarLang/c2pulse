import os
import lit.formats

config.name = "c2pulse-tests"
config.test_format = lit.formats.ShTest()
config.suffixes = [".c"]
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = config.test_source_root

llvm_bin_path = os.path.join(config.test_source_root, '..', 'external', 'llvm-project', 'build', 'bin')

# Get the paths from environment variables or provide default fallback
c2pulse_path = os.environ.get("C2PULSE_PATH", os.path.join(llvm_bin_path, 'c2pulse'))
filecheck_path = os.environ.get("FILECHECK", os.path.join(llvm_bin_path, "FileCheck"))
fstar_run_script = os.environ.get("FSTAR_RUN_SCRIPT", os.path.join(config.test_source_root, '..', 'run_fstar.sh'))

# Substitutions
config.substitutions.append(("%c2pulse", c2pulse_path))
config.substitutions.append(("%{FILECHECK}", filecheck_path))
config.substitutions.append(("%run_fstar.sh", fstar_run_script))

# Add environment variables if needed (optional)
config.environment["FSTAR_Z3VERSION"] = "4.13.3"
