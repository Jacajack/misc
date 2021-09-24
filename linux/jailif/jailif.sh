#!/bin/sh
NAMESPACE=$2
NAMESPACE_EXISTS=false
ASUSER=$3
IFACE=$1

source "$(dirname $0)/jailif_common.sh"

function usage() {
	msg_info "Usage: $(basename "$0") <interface> <namespace> <username> <command> [args]"
}

if [ -z "$NAMESPACE" ]; then
	msg_err "Missing namespace name"
	usage
	exit 1
fi

if [ -z "$IFACE" ]; then
	msg_err "Missing interface name"
	usage
	exit 1
fi

if [ -z "$ASUSER" ]; then
	msg_err "Missing username"
	usage
	exit 1
fi

if ip netns | grep "^$NAMESPACE" > /dev/null; then
	msg_warn "Namespace $NAMESPACE already exists"
	NAMESPACE_EXISTS=true
fi

if [ $EUID -ne 0 ]; then
	msg_err "This script must be run as root"
	msg_info "Maybe meant 'easyjailif'?"
	# msg_warn "Attempting to re-run with sudo"
	# sudo $0 $1 $2 $(id -un $3)
	exit 1
fi

IP=$(ip -f inet addr show $IFACE | grep -o 'inet [0-9\.\/]*' | cut -d ' ' -f 2)

if [ -z "$IP" ]; then
	msg_err "Failed to acquire $IFACE's IP address"
	exit 1
fi

msg_info "$IFACE's IP is $IP"

if [ $NAMESPACE_EXISTS = false ]; then
	msg_info "Creating namespace $NAMESPACE"
	ip netns add $NAMESPACE
fi

msg_info "Moving interface $IFACE to $NAMESPACE"
ip l set dev $IFACE netns $NAMESPACE

msg_info "Configuring the interface..."
ip netns exec $NAMESPACE ip l set $IFACE up
ip netns exec $NAMESPACE ip l set lo up
ip netns exec $NAMESPACE ip a add $IP dev $IFACE
ip netns exec $NAMESPACE route add -host 255.255.255.255 dev $IFACE

ARGV=("$@")
CMD="$COMMAND $(printf "'%s' " "${ARGV[@]:3}")"
msg_info "Running '$CMD' in the net namespace..."
ip netns exec $NAMESPACE su $ASUSER -c "$CMD"
msg_ok "Done!"

msg_info "Bringing the interface back to the default namespace"
ip netns exec $NAMESPACE ip l set dev $IFACE netns 1
ip l set $IFACE up
ip a add $IP dev $IFACE

if [ $NAMESPACE_EXISTS = false ]; then
	msg_info "Deleting the namespace $NAMESPACE..."
	ip netns del $NAMESPACE
else
	msg_warn "NOT deleting namespace $NAMESPACE"
fi

