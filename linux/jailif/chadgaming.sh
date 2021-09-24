#!/bin/sh

NAMESPACE="gigachad_$(tr -dc A-Za-z0-9 < /dev/urandom | head -c 8)"
IFACE="$1"
GAME="$2"

source "$(dirname $0)/jailif_common.sh"

function usage() {
	msg_info "Usage: $(basename "$0") <interface> <game>"
}

if ! [ -x "$(command -v tmux)" ]; then
	msg_error "Please install tmux"
	exit 1
fi

if ! [ -x "$(command -v figlet)" ]; then
	msg_error "Please install figlet"
	exit 1
fi

if ! [ -x "$(command -v playonlinux)" ]; then
	msg_error "Please install playonlinux"
	exit 1
fi

if [ -z "$IFACE" ]; then
	msg_err "Missing interface name"
	usage
	exit 1
fi

if [ -z "$GAME" ]; then
	msg_err "Missing game name"
	usage
	exit 1
fi


RCFILE="$(mktemp)"
cat "$(dirname "$0")/jailif_shellrc.sh" >> "$RCFILE"
msg_info "Preparing env dump..."
export -p >> "$RCFILE"

# cat "$RCFILE"

BASHCMD="source \"$RCFILE\"; playonlinux --run \"$GAME\""
tmux new-session \
	'cat smallchad.txt; read -rsn1' \; \
	split-window -h \
	"sudo $(dirname "$0")/jailif.sh $IFACE $NAMESPACE $USER bash -c '$BASHCMD'; echo 'Press enter to continue...'; read"\;

rm "$RCFILE"
