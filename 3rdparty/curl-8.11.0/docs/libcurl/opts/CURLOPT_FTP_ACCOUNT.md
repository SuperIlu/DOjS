---
c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Title: CURLOPT_FTP_ACCOUNT
Section: 3
Source: libcurl
Protocol:
  - FTP
See-also:
  - CURLOPT_PASSWORD (3)
  - CURLOPT_USERNAME (3)
Added-in: 7.13.0
---

# NAME

CURLOPT_FTP_ACCOUNT - account info for FTP

# SYNOPSIS

~~~c
#include <curl/curl.h>

CURLcode curl_easy_setopt(CURL *handle, CURLOPT_FTP_ACCOUNT, char *account);
~~~

# DESCRIPTION

Pass a pointer to a null-terminated string (or NULL to disable). When an FTP
server asks for "account data" after username and password has been provided,
this data is sent off using the ACCT command.

The application does not have to keep the string around after setting this
option.

Using this option multiple times makes the last set string override the
previous ones. Set it to NULL to disable its use again.

# DEFAULT

NULL

# %PROTOCOLS%

# EXAMPLE

~~~c
int main(void)
{
  CURL *curl = curl_easy_init();
  if(curl) {
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_URL, "ftp://example.com/foo.bin");

    curl_easy_setopt(curl, CURLOPT_FTP_ACCOUNT, "human-resources");

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
  }
}
~~~

# %AVAILABILITY%

# RETURN VALUE

Returns CURLE_OK if the option is supported, CURLE_UNKNOWN_OPTION if not, or
CURLE_OUT_OF_MEMORY if there was insufficient heap space.
