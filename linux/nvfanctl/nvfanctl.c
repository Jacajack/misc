#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <NVCtrl/NVCtrlLib.h>

// Speeds
#define FAN_SPIN_UP_SPEED 50
#define FAN_MIN_SPEED 15

// Modes
#define FAN_AUTO   0
#define FAN_MANUAL 1

// RPM sampling
#define RPM_SAMPLES       6
#define RPM_RESPIN_THRES  2000

// Controlled GPU id
static int gpu_id = 0;

// X11 stuff
static Display *display = NULL;
static int screen = 0;

// Loop breaker
int alive = 1;

// Nice and easy nanosleep wrapper
void delay(float t)
{
	struct timespec ts = {.tv_sec = floorf(t), .tv_nsec = fmodf(t, 1) * 1e9};
	nanosleep(&ts, NULL);
}

// Returns temperature of a GPU
int get_temp(int id)
{
	int temp = -1;
	XNVCTRLQueryTargetAttribute(display, NV_CTRL_TARGET_TYPE_GPU, id, 0, NV_CTRL_GPU_CORE_TEMPERATURE, &temp);
	return temp;
}

// Returns fan RPM
int get_rpm(int id)
{
	int rpm = -1;
	XNVCTRLQueryTargetAttribute(display, NV_CTRL_TARGET_TYPE_COOLER, id, 0, NV_CTRL_THERMAL_COOLER_SPEED, &rpm);
	return rpm;
}

// Set fan speed
void fan_speed(int id, int percent)
{
	XNVCTRLSetTargetAttribute(display, NV_CTRL_TARGET_TYPE_COOLER, id, 0, NV_CTRL_THERMAL_COOLER_LEVEL, percent);
}

// Spins the fan up, so it can be throttled down
int fan_spin_up(int id)
{
	fan_speed(id, FAN_SPIN_UP_SPEED);
	delay(2.f);
	return FAN_SPIN_UP_SPEED;
}

// Changes fan control mode (auto/manual)
void fan_ctl(int id, int mode)
{
	XNVCTRLSetTargetAttribute(display, NV_CTRL_TARGET_TYPE_GPU, id, 0, NV_CTRL_GPU_COOLER_MANUAL_CONTROL, mode);
}

// SIGINT handler
void sigint_handler(int signum)
{
	alive = 0;
}

// User-defined fan control logic (must return new fan speed)
// Current speed is passed as an argument in order to allow hysteresis implementation
static int fan_control_logic(int id, int temp, int speed)
{
	// return 3 * (temp - 50) + 15;                              // Virgin linear
	return temp < 20 ? 0 : expf(0.02f * powf(temp - 6.f, 1.3f)); // Chad exponential
}

int main(int argc, char **argv)
{
	// Handle SIGINT nicely
	signal(SIGINT, sigint_handler);
	
	// GPU ID
	const char *gpu_id_str = getenv("GPUID");
	if (gpu_id_str) sscanf(gpu_id_str, "%d", &gpu_id);
	else fprintf(stderr, "assuming GPUID = 0...\n");
	
	// Start X11 session
	display = XOpenDisplay(NULL);
	screen = DefaultScreen(display);
	
	// Start manual fan control
	fan_ctl(gpu_id, FAN_MANUAL);
	
	// Spin up fan to FAN_SPIN_UP_SPEED so it can be throttled down smoothly
	int speed = fan_spin_up(gpu_id);

	// The control loop
	while (alive)
	{
		int temp = get_temp(gpu_id);
		int rpm = get_rpm(gpu_id);
		int target_speed = fan_control_logic(gpu_id, temp, speed);
		
		// Calculate RPM change
		static int last_rpm = 0;
		int rpm_delta = rpm - last_rpm;
		last_rpm = rpm;
		
		// Last deltas and their sum
		static int rpm_deltas[RPM_SAMPLES] = {0};
		int rpm_change_sum = 0;
		memmove(&rpm_deltas[1], rpm_deltas, sizeof(rpm_deltas[0]) * (RPM_SAMPLES - 1));
		rpm_deltas[0] = rpm_delta;
		for (int i = 0; i < RPM_SAMPLES; i++)
			rpm_change_sum += abs(rpm_deltas[i]);
		
		// Respin the fan if it's unstable
		if (rpm_change_sum >= RPM_RESPIN_THRES)
		{
			speed = fan_spin_up(gpu_id);
			memset(rpm_deltas, 0, RPM_SAMPLES * sizeof(rpm_deltas[0]));
			continue;
		}
		
		// Clamp target speed if below FAN_MIN_SPEED
		// The fan is never turned off, because it makes a loud noise when spinning up
		if (target_speed < FAN_MIN_SPEED) target_speed = FAN_MIN_SPEED;
		
		// Smoothly change fan speed
		if (target_speed < speed) speed--;
		else if (target_speed > speed) speed++;
		fan_speed(gpu_id, speed);
		
		// Sleep
		delay(0.5f);
	}
	
	// Restore auto control
	fan_ctl(gpu_id, FAN_AUTO);
	XFlush(display);
	XCloseDisplay(display);
	return 0;
}
