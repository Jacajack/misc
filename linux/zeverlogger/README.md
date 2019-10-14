# zeverloggerd

Solar power logger daemon (for Zeversolar inverter unit).

<hr>

Usage:
`./zeverloggerd IP OUTPUT_FILE [INTERVAL=10] [DAEMONIZE=0]`
 - `IP` argument is the IP address of the inverter device.
 - `OUTPUT_FILE`is the output log file.
 - `INTERVAL` should be logging interval in seconds.
 - `DAEMONIZE` should be either 1 or 0, depending on whether the logger should run as daemon.
 
 Send `SIGUSR1` to force the daemon to make a URL request.
 Send `SIGUSR2` to force write to the output file.
 

_Note: This program is not super reliable. It works, but sometimes (rarely) it may break down. I'll have to make some improvements in the future._
