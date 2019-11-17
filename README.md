# Literate HTTP Parser

This is an HTTP parser implemented using a kind of [literate
programming](https://en.wikipedia.org/wiki/Literate_programming).

See `http_parser.c`, which is literally a copy/paste of the RFC, with parsing rule inlined:

```c
/*    The version of an HTTP message is indicated by an HTTP-Version field */
/*    in the first line of the message. */

/*        HTTP-Version   = "HTTP" "/" 1*DIGIT "." 1*DIGIT */

RULE(HTTP_VERSION,
     ONE(STRING("HTTP/"))
     AT_LEAST_ONE(RANGE('0', '9'))
     ONE(STRING("."))
     AT_LEAST_ONE(RANGE('0', '9')))
```

The code uses C macros to try to keep the code as literate as possible.

I wrote this around 2007, and IIRC there's a memory leak that I never
found, so please don't use it. This is also probably not a fast
parser.

## Requirements

Building it depends uthash (`apt install uthash-dev` on Debian).
