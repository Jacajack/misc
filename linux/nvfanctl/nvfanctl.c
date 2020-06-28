#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>

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

// Nice and easy nanosleep wrapper
void delay(float t)
{
	struct timespec ts = {.tv_sec = floorf(t), .tv_nsec = fmodf(t, 1) * 1e9};
	nanosleep(&ts, NULL);
}

// Returns temperature of a GPU
int get_temp(int id)
{
	char buf[1024];
	snprintf(buf, sizeof buf, "nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader --id=%d", id);
	
	FILE *fp = popen(buf, "r");
	if (fp == NULL)
	{
		perror("nvidia-smi call error");
		raise(SIGINT);
	}
	
	int temp = -1;
	fscanf(fp, "%d", &temp);
	pclose(fp);
	return temp;
}

// Returns fan RPM
int get_rpm(int id)
{
	char buf[1024];
	snprintf(buf, sizeof buf, "nvidia-settings -q \"[fan-%d]/GPUCurrentFanSpeedRPM\" -t", id);
	
	FILE *fp = popen(buf, "r");
	if (fp == NULL)
	{
		perror("nvidia-settings call error");
		raise(SIGINT);
	}
	
	int rpm = -1;
	fscanf(fp, "%d", &rpm);
	pclose(fp);
	return rpm;
}

// Set fan speed
void fan_speed(int id, int percent)
{
	char buf[1024];
	snprintf(buf, sizeof buf, "nvidia-settings -a \"[fan-%d]/GPUTargetFanSpeed=%d\" 2>&1 >/dev/null", id, percent);
	FILE *fp = popen(buf, "r");
	if (fp == NULL)
	{
		perror("nvidia-settings call error");
		raise(SIGINT);
	}
	pclose(fp);
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
	char buf[1024];
	snprintf(buf, sizeof buf, "nvidia-settings -a \"[gpu-%d]/GPUFanControlState=%d\" 2>&1 >/dev/null", id, mode);
	FILE *fp = popen(buf, "r");
	if (fp == NULL)
	{
		perror("nvidia-settings call error");
		raise(SIGINT);
	}
	pclose(fp);
}

// Called on exit - returns default fan controls
void quit(int ec)
{
	fan_ctl(gpu_id, FAN_AUTO);
	exit(ec);
}

// SIGINT handler
void sigint_handler(int signum)
{
	quit(EXIT_FAILURE);
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
	
	// Start manual fan control
	fan_ctl(gpu_id, FAN_MANUAL);
	
	// Spin up fan to FAN_SPIN_UP_SPEED so it can be throttled down smoothly
	int speed = fan_spin_up(gpu_id);

	// The control loop
	while (1)
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
	quit(EXIT_SUCCESS);
	return 0;
}
