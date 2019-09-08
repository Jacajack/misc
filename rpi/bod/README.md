# bod

A utility to detect power brown-outs.

Example:
`bod logfile.log > stdout.log`

Useful command for viewing logs: <br>
`paste <(cut -f 1 -d ' ' < bodlog.txt | xargs -L 1 -i date -d @{}) <(cut -d ' ' -f 3 < bodlog.txt)`

For converting logs to CSV: <br>
`gawk '{printf( "\"%s\",\"%s\"\n", strftime("%Y.%m.%d %H:%M:%S",$1), $3 )}' bodlog.txt`

Schematic:<br>
<img width=60% src=./bod.jpeg></img>
