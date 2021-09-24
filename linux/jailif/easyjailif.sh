#!/bin/sh
#PWD="$(pwd)"
#cd "$(dirname "$0")"

NAMESPACE="$2"
IFACE="$1"

source "$(dirname $0)/jailif_common.sh"

function usage() {
	msg_info "Usage: $(basename "$0") <interface> [namespace]"
}

if [ -z "$NAMESPACE" ]; then
	NAMESPACE="jail_$(tr -dc A-Za-z0-9 < /dev/urandom | head -c 8)"
	msg_warn "Generated a random namespace ($NAMESPACE)"
fi

if [ -z "$IFACE" ]; then
	msg_err "Missing interface name"
	usage
	exit 1
fi

RCFILE="$(mktemp)"
cat "$(dirname "$0")/jailif_shellrc.sh" >> "$RCFILE"
msg_info "Preparing env dump..."
export -p >> "$RCFILE"

# cat "$RCFILE"

sudo "$(dirname "$0")/jailif.sh" "$IFACE" "$NAMESPACE" "$USER" bash --init-file "$RCFILE"
rm "$RCFILE"
