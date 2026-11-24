#!/usr/bin/env python3
"""Downloads picobench and applies picobench.patch to it.

    python ext/picobench/update.py           # the latest tag
    python ext/picobench/update.py v2.11.0   # a given tag

The patch adds the median of the samples to the report of picobench.

If the patch doesn't apply to a new version, update it by hand:
    python ext/picobench/update.py <tag> --no-patch
    git add ext/picobench/picobench.hpp
    (make the changes of the old patch in picobench.hpp)
    git diff ext/picobench/picobench.hpp > ext/picobench/picobench.patch
"""

import json
import pathlib
import re
import subprocess
import sys
import urllib.request

REPO = "iboB/picobench"
HERE = pathlib.Path(__file__).resolve().parent
ROOT = HERE.parent.parent
PATCH = HERE / "picobench.patch"

# The file in this folder and its path in the picobench repository.
FILES = {
    "picobench.hpp": "include/picobench/picobench.hpp",
    "LICENSE": "LICENSE.txt",
}


def download(url):
    with urllib.request.urlopen(url) as response:
        return response.read()


def latest_tag():
    tags = json.loads(download(f"https://api.github.com/repos/{REPO}/tags"))
    # The API doesn't sort tags by version, so compare the numbers in them.
    return max((tag["name"] for tag in tags), key=lambda name: [int(n) for n in re.findall(r"\d+", name)])


def main():
    args = [arg for arg in sys.argv[1:] if arg != "--no-patch"]
    tag = args[0] if args else latest_tag()

    for name, path in FILES.items():
        (HERE / name).write_bytes(download(f"https://raw.githubusercontent.com/{REPO}/{tag}/{path}"))
    print(f"picobench {tag} is downloaded")

    if "--no-patch" in sys.argv:
        return 0

    result = subprocess.run(["git", "apply", PATCH.relative_to(ROOT).as_posix()], cwd=ROOT)
    if result.returncode != 0:
        print("The patch doesn't apply to this version. Update it by hand, see the top of this file.")
        return 1

    print("picobench.patch is applied")
    return 0


if __name__ == "__main__":
    sys.exit(main())
