---
c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: ca-native
Help: Load CA certs from the OS
Protocols: TLS
Category: tls
Added: 8.2.0
Multi: boolean
See-also:
  - cacert
  - capath
  - dump-ca-embed
  - insecure
Example:
  - --ca-native $URL
---

# `--ca-native`

Use the CA store from the native operating system to verify the peer. By
default, curl otherwise uses a CA store provided in a single file or
directory, but when using this option it interfaces the operating system's own
vault.

This option works for curl on Windows when built to use OpenSSL, wolfSSL
(added in 8.3.0) or GnuTLS (added in 8.5.0). When curl on Windows is built to
use Schannel, this feature is implied and curl then only uses the native CA
store.
