import sys
import re

if len(sys.argv) < 2:
    print("Usage:\n  {} <template> <undef file>".format(sys.argv[0]))
    exit(1)

fd = re.compile(r"unresolved: `_(\w*)'")

had_errors = False

with open(sys.argv[1], "r") as template:
    with open(sys.argv[2], "r") as undeffile:

        # create a set with available exports
        exports = []
        for l in template:
            l = l.strip()
            if not l.startswith("//") and not l.startswith("#") and len(l) > 0:
                exports.append(l)
        available = set(exports)

        # check if all symbols exist
        for l in undeffile:
            res = fd.search(l)
            if res:
                func = res.group(1)
                if not func.startswith("js_"):
                    if func not in exports:
                        had_errors = True
                        print("WARNING: DXE export '{}' missing!".format(func))

# fail if there were missing symbols
if had_errors:
    print("\n\n!!! DXE export check FAILED!\n\n")
    exit(1)
else:
    print("DXE export check OK!")
    exit(0)
