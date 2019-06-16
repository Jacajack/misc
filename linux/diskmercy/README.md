# diskmercy

A utility to restrain abusive disk writes (like constant stream logging). Works very similar to `sponge`.

Usage:
`diskmercy FILE [buffer size] [flush interval]`

`diskmercy` allows user to specify RAM buffer size (in bytes) and automatic buffer flushing interval (in seconds). If the flush interval is negative, no automatic buffer flushes are performed.

Additionally, data write to the disk can be triggered by sending SIGALRM to the diskmercy process.

Example:
`some-program | diskmercy out.txt 65536 3600`
