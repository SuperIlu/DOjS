#!/usr/bin/env python
# Utility script to dump font widths via fpdf

from fpdf import FPDF
import sys

pdf = FPDF()

fonts = [("Helvetica", "", "helvetica"),
		 ("Helvetica", "B", "helvetica_bold"),
		 ("Helvetica", "BI", "helvetica_bold_oblique"),
		 ("Helvetica", "I", "helvetica_oblique"),
		 ("Symbol", "", "symbol"),
		 ("Times", "", "times"),
		 ("Times", "B", "times_bold"),
		 ("Times", "BI", "times_bold_italic"),
		 ("Times", "I", "times_italic"),
		 ("ZapfDingbats", "", "zapfdingbats"),
		 ("Courier", "", "courier"),
		 # All the courier fonts are the same width
		 #("Courier", "B", "courier_bold"),
		 #("Courier", "BI", "courier_bold_oblique"),
		 #("Courier", "I", "courier_oblique"),
		 ]

for f in fonts:
	pdf.set_font(f[0], f[1], 14)

	sys.stdout.write("static const uint16_t %s_widths[256] = {\n" % (f[2]))
	for i in range(0, 256):
		if i % 14 == 0:
			sys.stdout.write("    ")
		width = int(pdf.get_string_width(chr(i)) * 2.83465 * 72)
		sys.stdout.write("%d, " % width)
		if i % 14 == 13:
			sys.stdout.write("\n")
	sys.stdout.write("\n};\n\n")
