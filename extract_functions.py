import sys
import re

if len(sys.argv) < 3:
    print("Usage:\n  {} <template> <infile> <outfile>".format(sys.argv[0]))
    exit(1)

fd = re.compile(r"(js_\w*)\([^;]*;")

with open(sys.argv[1], "r") as template:
    with open(sys.argv[2], "r") as infile:
        with open(sys.argv[3], "w") as outfile:
            # create lists with includes and exports
            exports = []
            prototypes = []
            includes = []
            for l in template:
                l = l.strip()
                if l.startswith("#"):
                    includes.append(l)
                elif l.startswith("extern "):
                    prototypes.append(l)
                elif not l.startswith("//") and len(l) > 0:
                    exports.append(l)

            # write header
            outfile.write(
                """
// this file is generated, do not edit!
#include <dlfcn.h>
#include <sys/dxe.h>

#include <mujs.h>

"""
            )

            # write includes
            outfile.write("\n")
            for i in includes:
                outfile.write(i)
                outfile.write("\n")

            # write prototypes
            outfile.write("\n")
            for i in prototypes:
                outfile.write(i)
                outfile.write("\n")

            # start symbol table
            outfile.write("\n")
            outfile.write(
                """

DXE_EXPORT_TABLE_AUTO(symtab)
  // template functions
"""
            )

            # write all exports
            for e in exports:
                outfile.write("  DXE_EXPORT({})".format(e))
                outfile.write("\n")

            # write js_ functions from include
            outfile.write("\n  // MuJS functions\n")
            for l in infile:
                res = fd.search(l)
                if res:
                    outfile.write("  DXE_EXPORT({})".format(res.group(1)))
                    outfile.write("\n")

            # finalize export table
            outfile.write("DXE_EXPORT_END\n")
