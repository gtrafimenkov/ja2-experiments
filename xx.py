#!/usr/bin/env python3
#
# Helper script to run different useful commands on the repository.

import os
import platform
import subprocess
import sys
import shutil

USAGE = """Helper script to run one or more predefiend commands.

Usage:
  python xx.py COMMAND [COMMAND...]

Commands:
  build-debug           - build debug version
  build-release         - build release version
  build-release-ru      - build release version for Russian game resources
  build-release-fr      - build release version for French game resources
  build-release-gr      - build release version for German game resources
  format-modified       - format modified files using clang-format
  format-all            - format all sources
  copy-dlls             - copy dlls necessary to run the game
  copy-dlls-ru          - copy dlls necessary to run the game
  copy-dlls-fr          - copy dlls necessary to run the game
  copy-dlls-gr          - copy dlls necessary to run the game
  copy-data             - find and copy game data to the debug build localtion
  copy-data-ru          - find and copy game data to Russian version
  copy-data-fr          - find and copy game data to French version
  copy-data-gr          - find and copy game data to German version
  clean                 - cleanup repository from all unwanted files
  run                   - run release build
  run-ru                - run Russian release build
  run-fr                - run French release build
  run-gr                - run German release build
  test                  - run unit test

Examples:
  python xx.py build copy-data run
"""


def find_files(dir, extensions):
    found = []
    for root, _, files in os.walk(dir):
        for file in files:
            ext = os.path.splitext(file)[1]
            if ext in extensions:
                found.append(os.path.join(root, file))
    return found


def get_modified_files():
    result = subprocess.run(
        ["git", "status", "--porcelain"], capture_output=True, text=True
    )
    modified_files = []
    for line in result.stdout.splitlines():
        flags, file_path = line.split()[:2]
        if "M" in flags:
            modified_files.append(file_path)
    return modified_files


def filter_source_files(files):
    source_extensions = [".h", ".c", ".cpp", ".cc"]
    return [file for file in files if os.path.splitext(file)[1] in source_extensions]


def format_files(files):
    if len(files) > 0:
        print(f"Formatting {len(files)} files ...", file=sys.stderr)
        subprocess.run(["clang-format", "-i", "--style=file"] + files)
    else:
        print("No files to format", file=sys.stderr)


def copy_dlls(dest_dir):
    """
    Copy dlls necessary to run the game.
    """
    if platform.system() == "Windows":
        shutil.copy("tools/original-dlls/Smackw32.dll", dest_dir)
        shutil.copy("tools/original-dlls/mss32.dll", dest_dir)

    if platform.system() == "Windows" and platform.release() == "10":
        shutil.copy("tools/dxwrapper/ddraw.dll", dest_dir)
        shutil.copy("tools/dxwrapper/dsound.dll", dest_dir)
        shutil.copy("tools/dxwrapper/dxwrapper.dll", dest_dir)
        shutil.copy("tools/dxwrapper/dxwrapper.ini", dest_dir)


def find_ja2_data_files(lang):
    try_dirs = []
    if lang == "ENGLISH":
        try_dirs = [
            "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Jagged Alliance 2 Gold\\Data",
            "C:\\GOG Games\\Jagged Alliance 2\\Data\\",
        ]
    elif lang == "RUSSIAN":
        try_dirs = [
            "../ja2-collection/russian-buka/Data",
            # "../ja2-collection/russian-gold/Data",
        ]
    elif lang == "FRENCH":
        try_dirs = [
            "../ja2-collection/french/Data",
        ]
    elif lang == "GERMAN":
        try_dirs = [
            "../ja2-collection/german/Data",
        ]
    for d in try_dirs:
        if os.path.isdir(d):
            return d
    return None


def copy_ja2_data_files(dest_dir, lang):
    if os.path.isdir(dest_dir):
        print("Data already copied", file=sys.stderr)
        return True

    source = find_ja2_data_files(lang)
    if source is None:
        print(f"Cannot find data files for JA2 {lang}", file=sys.stderr)
        return False

    shutil.copytree(source, dest_dir)
    return True


# def get_debug_build_location():
#     if platform.system() == "Windows":
#         return "build/Debug"
#     return "build"


# def get_debug_build_exe():
#     if platform.system() == "Windows":
#         return "build/Debug/ja2vcp.exe"
#     return "build/ja2vcp"


def get_release_build_location(lang):
    if platform.system() == "Windows":
        return os.path.join(get_build_dir(True, lang), "bin-win32/RelWithDebInfo")
    else:
        raise Exception("No game build for linux yet")


def get_release_build_exe(lang):
    if platform.system() == "Windows":
        return os.path.join(
            get_build_dir(True, lang), "bin-win32/RelWithDebInfo/ja2v.exe"
        )
    else:
        raise Exception("No game build for linux yet")


def get_release_test_exe(lang):
    if platform.system() == "Windows":
        return os.path.join(
            get_build_dir(True, lang), "unittester/RelWithDebInfo/unittester.exe"
        )
    else:
        return os.path.join(get_build_dir(True, lang), "unittester/unittester")


def get_build_dir(release, lang):
    suffix = ""
    if lang == "RUSSIAN":
        suffix = "-ru"
    elif lang == "FRENCH":
        suffix = "-fr"
    elif lang == "GERMAN":
        suffix = "-gr"
    if release:
        return "build-release" + suffix
    else:
        return "build-debug" + suffix


def get_configure_build_command(release, lang):
    lang_param = f"-DLANGUAGE={lang}"
    build_type = "Release" if release else "Debug"
    if platform.system() == "Windows":
        return [
            "cmake",
            "-G",
            "Visual Studio 17 2022",
            "-A",
            "Win32",
            "-B",
            get_build_dir(release, lang),
            lang_param,
        ]
    else:
        return [
            "cmake",
            "-B",
            get_build_dir(release, lang),
            f"-DCMAKE_BUILD_TYPE={build_type}",
            lang_param,
        ]


def get_build_command(release, lang):
    if platform.system() == "Windows":
        return [
            "cmake",
            "--build",
            get_build_dir(release, lang),
            "--parallel",
            "--config",
            "RelWithDebInfo" if release else "Debug",
        ]
    else:
        return ["cmake", "--build", get_build_dir(release, lang), "--parallel"]


def run_command(command):
    if command in ["build-debug"]:
        is_release = False
        lang = "ENGLISH"
        subprocess.run(get_configure_build_command(is_release, lang), check=True)
        subprocess.run(get_build_command(is_release, lang), check=True)

    elif command in ["build-release"]:
        is_release = True
        lang = "ENGLISH"
        subprocess.run(get_configure_build_command(is_release, lang), check=True)
        subprocess.run(get_build_command(is_release, lang), check=True)

    elif command in ["build-release-ru"]:
        is_release = True
        lang = "RUSSIAN"
        subprocess.run(get_configure_build_command(is_release, lang), check=True)
        subprocess.run(get_build_command(is_release, lang), check=True)

    elif command in ["build-release-fr"]:
        is_release = True
        lang = "FRENCH"
        subprocess.run(get_configure_build_command(is_release, lang), check=True)
        subprocess.run(get_build_command(is_release, lang), check=True)

    elif command in ["build-release-gr"]:
        is_release = True
        lang = "GERMAN"
        subprocess.run(get_configure_build_command(is_release, lang), check=True)
        subprocess.run(get_build_command(is_release, lang), check=True)

    elif command == "clean":
        subprocess.run(["git", "clean", "-fdx"], check=True)

    elif command == "copy-dlls":
        copy_dlls(get_release_build_location(lang="ENGLISH"))

    elif command == "copy-dlls-ru":
        copy_dlls(get_release_build_location(lang="RUSSIAN"))

    elif command == "copy-dlls-fr":
        copy_dlls(get_release_build_location(lang="FRENCH"))

    elif command == "copy-dlls-gr":
        copy_dlls(get_release_build_location(lang="GERMAN"))

    elif command == "copy-data":
        dest_dir = os.path.join(get_release_build_location(lang="ENGLISH"), "data")
        if not copy_ja2_data_files(dest_dir, lang="ENGLISH"):
            sys.exit(10)

    elif command == "copy-data-ru":
        dest_dir = os.path.join(get_release_build_location(lang="RUSSIAN"), "data")
        if not copy_ja2_data_files(dest_dir, lang="RUSSIAN"):
            sys.exit(10)

    elif command == "copy-data-fr":
        dest_dir = os.path.join(get_release_build_location(lang="FRENCH"), "data")
        if not copy_ja2_data_files(dest_dir, lang="FRENCH"):
            sys.exit(10)

    elif command == "copy-data-gr":
        dest_dir = os.path.join(get_release_build_location(lang="GERMAN"), "data")
        if not copy_ja2_data_files(dest_dir, lang="GERMAN"):
            sys.exit(10)

    elif command == "format-modified":
        modified_files = get_modified_files()
        source_files = filter_source_files(modified_files)
        format_files(source_files)

    elif command == "format-all":
        source_files = find_files("ja2lib", [".h", ".c", ".cc"])
        source_files += find_files("bin-linux", [".h", ".c", ".cc"])
        source_files += find_files("bin-win32", [".h", ".c", ".cc"])
        source_files += find_files("platform-dummy", [".h", ".c", ".cc"])
        source_files += find_files("platform-dummy", [".h", ".c", ".cc"])
        source_files += find_files("platform-win32", [".h", ".c", ".cc"])
        source_files += find_files("unittester", [".h", ".c", ".cc"])
        format_files(source_files)

    elif command == "run":
        subprocess.run([get_release_build_exe(lang="ENGLISH")])

    elif command == "run-ru":
        subprocess.run([get_release_build_exe(lang="RUSSIAN")])

    elif command == "run-fr":
        subprocess.run([get_release_build_exe(lang="FRENCH")])

    elif command == "run-gr":
        subprocess.run([get_release_build_exe(lang="GERMAN")])

    elif command == "test":
        subprocess.run([get_release_test_exe(lang="ENGLISH")], check=True)

    else:
        print(f"Unknown command {command}", file=sys.stderr)
        sys.exit(1)


def main():
    args = sys.argv[1:]
    if len(args) == 0:
        print(USAGE, file=sys.stderr)
        sys.exit(1)

    commands = args
    for command in commands:
        run_command(command)


if __name__ == "__main__":
    main()
