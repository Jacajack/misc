# diskmercy

A utility to restrain abusive disk writes (like constant stream logging). It's very similar to `sponge`.

`diskmercy` allows user to specify RAM buffer size (in bytes) and automatic buffer flushing interval (in seconds). Additionally, data write to the disk can be triggered by sending SIGALRM to the diskmercy process.

See `diskmercy --help` for more information.

Example:
`some-program | diskmercy out.txt`
