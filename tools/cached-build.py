#!/usr/bin/env python3
#
# This is a script for caching build of the game.  Built binaries are saved for
# later, when a user wants to run the same binary again, it will be taken from
# the cache.
#
# It is especially useful during git bisect for quickly testing different
# versions of the game.
#
# Usage
# -----
#
# Building the current commit:
#   python tools/cached-build.py build $(git rev-parse HEAD)
#
# Building again, which will be instantaneous:
#   python tools/cached-build.py build $(git rev-parse HEAD)
#
# Building any commit:
#   python tools/cached-build.py build COMMIT_HASH
#
# Commit COMMIT_HASH will be checked out to another working tree
# ../<repo_name>-cached-build, the program will be built and the resulting exe
# will be stored in ../<repo_name>-build-cache.
#
# Using with git bisect
# ---------------------
#
# See docs/git-bisect.md

import os
import os.path
import subprocess
import sys
import shutil

USAGE = """Helper script for cached build.

Usage:
  python tools/cached-build.py COMMAND [ARGS...]

Commands:
  build     COMMIT_HASH   - cached build
  build-run COMMIT_HASH   - cached build + run the resulting exe

Examples:
  python tools/cached-build.py build-run $(git rev-parse HEAD)
"""


def fatal_error(message, exit_code=10):
    print(message, file=sys.stderr)
    sys.exit(exit_code)


def get_branches_map():
    """
    Get list of branches in the git repo.
    Return mapping branch_name -> branch_status.
    """
    branches = {}
    result = subprocess.run(
        ["git", "branch"],
        stdout=subprocess.PIPE,
        check=True,
        text=True,
    )
    for line in result.stdout.rstrip().split("\n"):
        status = line[0]
        branch_name = line[2:]
        branches[branch_name] = status

    return branches


def has_worktree_for_branch(branch_name):
    """
    Check if git worktree exists for the given branch.
    """
    result = subprocess.run(
        ["git", "worktree", "list", "--porcelain"],
        stdout=subprocess.PIPE,
        check=True,
        text=True,
    )
    return f"branch refs/heads/{branch_name}\n" in result.stdout


def log(message):
    print(message, file=sys.stderr)


def get_build_cache_dir(commit_hash):
    curdir_name = os.path.basename(os.getcwd())
    buildcache_dir = os.path.join("..", curdir_name + "-build-cache")
    cache_dir = os.path.join(buildcache_dir, commit_hash[0:2], commit_hash)
    return cache_dir


def build_to_cache_and_run(commit_hash, build_branch_name, built_exe_path):
    curdir_name = os.path.basename(os.getcwd())
    built_exe_name = os.path.basename(built_exe_path)
    cache_dir = get_build_cache_dir(commit_hash)
    cached_exe = os.path.join(cache_dir, built_exe_name)
    build_location = os.path.join("..", curdir_name + f"-{build_branch_name}")
    if (not os.path.isdir(build_location)) or (not os.path.exists(cached_exe)):
        log(f"==> building exe for {commit_hash}")
        build_to_cache(commit_hash, build_branch_name, built_exe_path)
    exe_dest_dir = os.path.join(build_location, os.path.dirname(built_exe_path))
    log(f"==> copying {cached_exe} to {exe_dest_dir}")
    os.makedirs(exe_dest_dir, exist_ok=True)
    copied_exe_path = os.path.join(exe_dest_dir, built_exe_name)
    shutil.copy2(cached_exe, copied_exe_path)
    log("==> launching")
    print(copied_exe_path)
    subprocess.run([copied_exe_path])


def get_build_location(build_branch_name):
    curdir_name = os.path.basename(os.getcwd())
    return os.path.join("..", curdir_name + f"-{build_branch_name}")


def build_to_cache(commit_hash, build_branch_name, built_exe_path):
    """
    Build exe for commit `commit_hash`.
    Use branch `build_branch_name` in a separate git workspace for building the commit.
    Put the built exe into `<current_dir>-build-cache` directory.
    `built_exe_path` is the relative path for finding built exe file.
    """
    build_location = get_build_location(build_branch_name)
    cache_dir = get_build_cache_dir(commit_hash)
    cached_exe = os.path.join(cache_dir, os.path.basename(built_exe_path))
    if os.path.exists(cached_exe):
        log(f"==> built file exists: {cached_exe}")
        return
    branches = get_branches_map()
    if build_branch_name not in branches:
        log(f"==> creating branch {build_branch_name}")
        subprocess.run(
            ["git", "branch", build_branch_name],
            check=True,
        )
    if not has_worktree_for_branch(build_branch_name):
        log(f"==> adding work tree for branch {build_branch_name} at {build_location}")
        subprocess.run(
            [
                "git",
                "worktree",
                "add",
                os.path.abspath(build_location),
                build_branch_name,
            ],
            check=True,
        )
    log(f"==> switching to commit {commit_hash} at worktree {build_location}")
    subprocess.run(
        ["git", "reset", "--hard", commit_hash], check=True, cwd=build_location
    )
    log(f"==> running the build at worktree {build_branch_name}")
    subprocess.run(["python", "xx.py", "build-release"], check=True, cwd=build_location)
    log(f"==> copying the built exe to the build cache {cache_dir}")
    os.makedirs(cache_dir, exist_ok=True)
    shutil.copy2(os.path.join(build_location, built_exe_path), cache_dir)


def main():
    if len(sys.argv) < 3:
        print(USAGE, file=sys.stderr)
        sys.exit(10)

    command = sys.argv[1]
    commit_hash = sys.argv[2]
    built_exe_path = "build-release/bin-win32/RelWithDebInfo/ja2v.exe"

    build_branch_name = "cached-build"

    if command == "build":
        build_to_cache(commit_hash, build_branch_name, built_exe_path)
    elif command == "build-run":
        build_location = get_build_location(build_branch_name)
        subprocess.run(
            ["python", "xx.py", "copy-dlls", "copy-data"],
            check=True,
            cwd=build_location,
        )
        build_to_cache_and_run(commit_hash, build_branch_name, built_exe_path)
    else:
        fatal_error(f"Unknown command {command}", 1)


if __name__ == "__main__":
    main()
