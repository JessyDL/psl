import os
import subprocess

def is_tool(name):
    """Check whether `name` is on PATH and marked as executable."""

    # from whichcraft import which
    from shutil import which

    return which(name) is not None


def generate(build_directory = None, output_dir =None, filters = ["*/psl/tests/*", "*gtest*", "*build/*"], gcov_override = None):
    if build_directory is None:
        root = os.path.dirname(os.path.realpath(__file__))
        build_directory = os.path.abspath(os.path.join(root, ".."))
    if output_dir is None:
        output_dir =  f"{build_directory}/build/coverage"
    if not is_tool("lcov"):
        raise Exception("missing 'lcov', please install it and make it visible to the path")
    if not is_tool("genhtml"):
        raise Exception("missing 'genhtml', please install it and make it visible to the path")

    output_file = "codecoverage.info"

    if gcov_override is None:
        gcov_override = []
    else:
        gcov_override = ["--gcov-tool", gcov_override]
        
    subprocess.check_call(["lcov", "--directory", build_directory, "--capture", "--no-external", "--output-file", output_file, "-rc", "lcov_branch_coverage=1"] + gcov_override)
    subprocess.check_call(["lcov", "--remove", output_file, "-o", "--no-external", "--output-file", output_file] + filters)
    subprocess.check_call(["genhtml", output_file, "--output-directory", output_dir])
    os.remove(output_file)
