# This script takes all commits between REBASE_POINT and the head
# and applies them to a new branch "pub-<current-branch-name>",
# modifying the commit message along the way.  The current branch
# name (without a numeric suffix, if present) is added to the
# commit messages.

import subprocess
import sys
import re

REBASE_POINT = "origin/main"


def remove_numeric_suffix(text):
    return re.sub(r"-\d+$", "", text)


def run_command(command, check=True, capture_output=True):
    print(f".. running command {command}", file=sys.stderr)
    result = subprocess.run(
        command, shell=True, text=True, capture_output=capture_output
    )
    if check and result.returncode != 0:
        print(f"Error: {result.stderr.strip()}")
        sys.exit(1)
    return result.stdout.strip()


def main():
    # Check if there are uncommitted changes
    status = run_command("git status --porcelain", check=False)
    if status:
        print("Error: You have uncommitted changes. Commit or stash them first.")
        sys.exit(1)

    # Get current branch name
    current_branch = run_command("git rev-parse --abbrev-ref HEAD")
    if current_branch in ["main", "dev"]:
        print(
            f"Error: You are on a protected branch ({current_branch}). Switch to another branch."
        )
        sys.exit(1)
    if current_branch.startswith("pub-"):
        print(
            f"Error: You are on a publication branch ({current_branch}). Switch to another branch."
        )
        sys.exit(1)

    current_branch_wo_suffix = remove_numeric_suffix(current_branch)

    # Get list of commits between REBASE_POINT and current branch
    commits = run_command(
        f"git log --reverse --format=%H {REBASE_POINT}..{current_branch}"
    )
    commit_list = commits.splitlines()

    # Define the new branch name
    pub_branch = f"pub-{current_branch}"

    # Delete existing pub- branch if it exists
    existing_branches = run_command("git branch --list").splitlines()
    if pub_branch in [b.strip() for b in existing_branches]:
        run_command(f"git branch -D {pub_branch}")

    # Create new pub- branch from REBASE_POINT
    run_command(f"git checkout -b {pub_branch} {REBASE_POINT}")

    # Apply commits
    for commit in commit_list:
        commit_message = run_command(f"git log --format=%B -n 1 {commit}").strip()
        if commit_message.startswith("("):
            # not modifying
            modified_message = commit_message
        else:
            modified_message = f"({current_branch_wo_suffix}) {commit_message}"
        run_command(["git", "cherry-pick", "--allow-empty-message", commit])
        run_command(["git", "commit", "--amend", "-m", modified_message])

    run_command(f"git checkout {current_branch}")
    print(
        f"Successfully created {pub_branch} with applied commits from {current_branch}."
    )


if __name__ == "__main__":
    main()
