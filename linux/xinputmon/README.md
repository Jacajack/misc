# xinputmon

A utility to run provided command each time X11 input device configuration changes (i.e. a keyboard or mouse is connected/disconnected).
Basically a lighter alternative for `inputplug`. Useful for running `xkbcomp` and similar stuff.

Command is executed when at least `COMMAND_DELAY` seconds pass after the last event.

Example:
`xinputmon echo "input config changed!"`

