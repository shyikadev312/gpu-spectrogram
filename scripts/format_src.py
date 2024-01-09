import argparse
import os
import subprocess
from shutil import which
from pathlib import Path

majorVersion = 14
codeFormatToolsNames = ["clang-format-{0}".format(majorVersion), "clang-format"]
requiredVersion = "version {0}.".format(majorVersion)


def isCorrectVersion(toolName):
    process = subprocess.Popen([toolName, "--version"],
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE,
                               universal_newlines=True)
    stdout, stderr = process.communicate()
    exitCode = process.wait()

    if not exitCode == 0:
        return False

    isRequiredVersion = requiredVersion in stdout
    return isRequiredVersion


def isTool(name):
    """Check whether `name` is on PATH and marked as executable."""
    # from whichcraft import which
    from shutil import which
    return which(name) is not None


def getSourceFiles(srcDir):
    sourceFiles = []
    allowedExtensions = [".cpp", ".h"]

    for root, dirs, files in os.walk(srcDir):
        for file in files:
            if any(file.endswith(extension) for extension in allowedExtensions):
                sourceFiles.append(os.path.join(root, file))

    return sourceFiles


def getFormatToolPath():
    for alternativeName in codeFormatToolsNames:
        if isTool(alternativeName) and isCorrectVersion(alternativeName):
            return alternativeName
    return None


def getSrcDir():
    scriptDir = Path(os.path.dirname(os.path.realpath(__file__)))
    projectRootDir = scriptDir.parent
    srcDir = projectRootDir.joinpath("src")
    return srcDir


def main():
    parser = argparse.ArgumentParser(
        description='''This script either formats all C++ source code files under /src directory, or checks their formatting.''',
        epilog="""I am epilog.""")
    parser.add_argument("--dry-run", help="If specified, clang-format tool will be run without actually changing source code files - only code format validation will be performed. ",
                    action="store_true")
    args = parser.parse_args()

    formatToolPath = getFormatToolPath()

    if formatToolPath is None:
        print("Error: {0} not installed.".format(codeFormatToolsNames[0]))
        exit(1)

    srcDir = getSrcDir()
    filesToFormat = getSourceFiles(srcDir)

    isSuccessFormat = True

    if args.dry_run:
        formatArguments = ["--dry-run", "--Werror"]
        print("Checking code format of all *.h and *.cpp files in ./src directory...\n")
    else:
        formatArguments = ["-i"]
        print("Formatting all *.h and *.cpp files in ./src directory...\n")

    for file in filesToFormat:
        # print(file)

        process = subprocess.Popen([formatToolPath, file, "-style=file"] + formatArguments,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE,
                                   universal_newlines=True)
        stdout, stderr = process.communicate()
        exitCode = process.wait()

        if exitCode != 0:
            isSuccessFormat = False
            print(stderr)

    if args.dry_run:
        print("\nCode format validation was performed for C++ source code files.")
        if isSuccessFormat:
            print("Success: all files conform code format.")
        else:
            print("Problem: Some files need code format to be applied.")
            exit(2)
    else:
        if isSuccessFormat:
            print("Formatting finished successfully!")
        else:
            print("Formatting finished with errors.")
            exit(3)


main()

