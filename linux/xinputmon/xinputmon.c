#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput2.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/wait.h>

#define COMMAND_DELAY 2

// --------------- child code

struct timeval cmd_time = {.tv_sec = 0, .tv_usec = 0};

void run_command(char *argv[])
{
	if (fork() == 0)
	{
		if (execvp(argv[0], &argv[0]))
		{
			perror("Failed to run command");
			exit(1);
		}
	}
	
	wait(NULL);
}

void child_usr1(int signum)
{
	signal(SIGUSR1, child_usr1);
	gettimeofday(&cmd_time, NULL);
	cmd_time.tv_sec += COMMAND_DELAY;
}

int child_main(int argc, char *argv[])
{
	signal(SIGUSR1, child_usr1);
	
	while (1)
	{
		struct timeval cur_time = {};
		gettimeofday(&cur_time, NULL);
		
		if (cmd_time.tv_sec != 0 && cmd_time.tv_sec < cur_time.tv_sec && cmd_time.tv_usec < cur_time.tv_usec)
		{
			run_command(&argv[1]);
			cmd_time = (struct timeval){.tv_sec = 0, .tv_usec = 0};
		}
		
		sleep(1);
	}
}

// --------------- parent code

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr,
			"Usage: %s <COMMAND> [ARGS]\n"
			"The command will be ran on every detected X11 input change.\n",
			argv[0]
		);
		return 1;
	}
	
	// Fork out to child process
	pid_t child_pid = fork();
	if (child_pid == 0)
		return child_main(argc, argv);
	
	// Start X11 session
	Display *display = XOpenDisplay(NULL);
	Window window = DefaultRootWindow(display);
	
	// Check if XInput2 is present
	int xinput2_major_opcode;
	int dummy;
	bool xinput2_preset = XQueryExtension(display, INAME, &xinput2_major_opcode, &dummy, &dummy);
	assert(xinput2_preset);
	
	// We're only interested in hierarchy changes
	int device_mask = XI_HierarchyChangedMask;
	
	// We're listening to all input devices
	XIEventMask event_mask = {
		.mask = (unsigned char*) &device_mask,
		.mask_len = sizeof(device_mask),
		.deviceid = XIAllDevices,
	};
	
	XISelectEvents(display, window, &event_mask, 1);
	
	while (1)
	{
		XEvent ev; 
		XNextEvent(display, &ev);
		const XIHierarchyEvent *hier_ev = (XIHierarchyEvent*)&ev;
		
		// Notify our child if the input device hierarchy has changed
		if (ev.type == GenericEvent && hier_ev->extension == xinput2_major_opcode && hier_ev->evtype == XI_HierarchyChanged)
			kill(child_pid, SIGUSR1);
	}
}
