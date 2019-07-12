# bod

A utility to detect power brown-outs.

Example:
`bod logfile.log > stdout.log`

Useful command for viewing logs:
`paste <(cut -f 1 -d ' ' < bodlog.txt | xargs -L 1 -i date -d @{}) <(cut -d ' ' -f 3 < bodlog.txt)`
