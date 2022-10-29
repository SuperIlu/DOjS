
# ini
A *tiny* ANSI C library for loading .ini config files

## Usage
The files **[ini.c](src/ini.c?raw=1)** and **[ini.h](src/ini.h?raw=1)** should
be dropped into an existing project.

The library has support for sections, comment lines and quoted string values
(with escapes). Unquoted values and keys are trimmed of whitespace when loaded.

```ini
; last modified 1 April 2001 by John Doe
[owner]
name = John Doe
organization = Acme Widgets Inc.

[database]
; use IP address in case network name resolution is not working
server = 192.0.2.62     
port = 143
file = "payroll.dat"
```

An ini file can be loaded into memory by using the `ini_load()` function.
`NULL` is returned if the file cannot be loaded.
```c
ini_t *config = ini_load("config.ini");
```

The library provides two functions for retrieving values: the first is
`ini_get()`. Given a section and a key the corresponding value is returned if
it exists. If the `section` argument is `NULL` then all sections are searched.
```c
const char *name = ini_get(config, "owner", "name");
if (name) {
  printf("name: %s\n", name);
}
```

The second, `ini_sget()`, takes the same arguments as `ini_get()` with the
addition of a scanf format string and a pointer for where to store the value.
```c
const char *server = "default";
int port = 80;

ini_sget(config, "database", "server", NULL, &server);
ini_sget(config, "database", "port", "%d", &port);

printf("server: %s:%d\n", server, port);
```

The `ini_free()` function is used to free the memory used by the `ini_t*`
object when we are done with it. Calling this function invalidates all string
pointers returned by the library.
```c
ini_free(config);
```


## License
This library is free software; you can redistribute it and/or modify it under
the terms of the MIT license. See [LICENSE](LICENSE) for details.
