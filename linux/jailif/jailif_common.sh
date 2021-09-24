function colortext() {
	echo -ne "\x1b[3$1m$2\x1b[0m"
}

function message() {
	echo -ne "[$(colortext $1 "$2")] $3\n"
}

function msg_info() {
	message 4 "INFO" "$@"
}

function msg_warn() {
	message 3 "WARN" "$@"
}

function msg_err() {
	message 1 "FAIL" "$@"
}

function msg_ok() {
	message 2 " OK " "$@"
}

