         cRSID by Hermit (Mihaly Horvath) - Year 2022
         --------------------------------------------

This is a serious integer-only update (rewrite) to my cSID and cSID-light
commandline SID-players, aiming for an ongoing inclusion into RockBox,
as per a request by Ninja earlier at CSDb. He will do the integration.

The name now contains the 'R' because now a RealSID-like environment-mode
(CIA, VIC, IRQ, NMI) is supported more-or-less ('RealSIDmode' in source).
There are still many RSID tunes that are not played properly, mainly
the tricky ones which use DC04 or DD0C as jump-addresses or RTI placeholders,
like for example 'Hi Fi Sky', and other modern digi tunes.
I don't feel like debugging these complex tunes now...
PSID-only playback has improved a lot since cSID-light-1.1,
and now many PlaySID-digis are supported as well if that still matters.

The CPU and ADSR went through serious timing improvements,
it's now can be called cycle-exact, and hardrestarts and delaybugs are now
simulated well, no missed notes, whatever.

This release also contains shared and static library forms for better
inclusion in other SID-playback projects (like FlexSID for example).
I completely eliminated global variables and definitions, except a
'cRSID_c64' instance which is for faster access of struct members.
(Emulated C64 memory accesses are made to this for faster operation.)

More info (the API for the library) is seen in the 'libcRSID.h' file.

It has been tried already on a SanDisk RockBox device and tunes not
containing digis are already played on it fine. Tunes with 2SID/3SID or
digis however are too much for the player (38MHz..192MHz ARM) at this form.
So maybe the code will be optimized by eliminating overheads of structs,
pointers, optimizing function-calls, inlining, and by other possible means.

Hopefully in the near future the outdated TinySID engine will be replaced
in the RockBox source-tree...

License is still WTF: Do what the fuck you want with this code, but
                      it would be nice mentioning me as the original author.
