# nvfanctl

Making my own GPU fan controller, beacuse some fucking moron at GIGABYTE couldn't.... This program eliminates revving problems of RTX 2070 SUPER by smoothly transitioning fan speeds according to my own fan curve. By default, the fan is kept running at 15% speed (`FAN_MIN_SPEED`), achieved by throttling-down from higher speed (`FAN_SPIN_UP_SPEED`). The fan is never turned off, because any violent spin-up result in audible and annoying revving sound. This is thanks to the **lovely** regulator they decided to employ, that favors spin-up time over stability (_why won't you reduce the proportional gain?? whyyyyy?????_). I really hope they fix this issue soon...

`GPU_ID` environment variable determines controlled GPU ID. If not set, `GPU_ID` is assumed to be 0.
