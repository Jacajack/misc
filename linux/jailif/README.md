# jailif

A set of utilities for running programs in jail environments only able to access one chosen network interface.

`jailif` creates a new network namespace with access only to the specified network interface and runs a command inside of it.

`easyjailif` opens a `bash` shell inside a jail environment created by `jailif` and preserves all environment variables in that process.

`chadgaming` starts the specified PlayOnLinux game in an environment only connected to the specified ZeroTier network.

<hr>

Usage: `jalif <interface> <net-namespace> <username> <command> [args]`

Usage: `easyjail <interface> [net-namespace]`

Usage: `chadgaming <zerotier-network> <playonlinux-game>`


