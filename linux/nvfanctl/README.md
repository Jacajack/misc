# nvfanctl

This custom fan driver eliminates revving problems of my GIGABYTE RTX 2070 SUPER by smoothly transitioning fan speeds according to my own fan curve. By default, the fan is kept running at 15% speed (`FAN_MIN_SPEED`), achieved by throttling-down from higher speed (`FAN_SPIN_UP_SPEED`). The fan is never turned off, because any sudden spin-up results in audible and annoying revving sound. This is because, for some reason, the regulator they decided to employ favors lower spin-up time over RPM stability. I really hope this issue gets fixed soon...

Any fan instability is detected by measuring RPM changes. If the change over last `RPM_SAMPLES` samples exceeds `RPM_RESPIN_THRES`, the fan is spun up to `FAN_SPIN_UP_SPEED` and throttled down again.

`GPU_ID` environment variable determines controlled GPU ID. If not set, `GPU_ID` is assumed to be 0.

The new version utilizes libXNVCtrl to control the GPU instead of calling `nvidia-smi` and `nvidia-settings`.
