# Using git bisect for bug hunting

To find a commit that introduced a bug, use `git bisect` command.
Also use `tools/cached-build.py` script to quickly build and the game
for any commit.

## Example of using git bisect with cached-build.py

```
git bisect start --no-checkout BAD_COMMIT GOOD_COMMIT
python tools/cached-build.py build-run $(git rev-parse BISECT_HEAD)
git bisect good
python tools/cached-build.py build-run $(git rev-parse BISECT_HEAD)
git bisect good
python tools/cached-build.py build-run $(git rev-parse BISECT_HEAD)
git bisect good
python tools/cached-build.py build-run $(git rev-parse BISECT_HEAD)
git bisect bad
python tools/cached-build.py build-run $(git rev-parse BISECT_HEAD)
git bisect good
python tools/cached-build.py build-run $(git rev-parse BISECT_HEAD)
git bisect good
python tools/cached-build.py build-run $(git rev-parse BISECT_HEAD)
git bisect bad
python tools/cached-build.py build-run $(git rev-parse BISECT_HEAD)
git bisect good
git branch crash-found $(git rev-parse BISECT_HEAD)
git push origin crash-found
git bisect reset
```
