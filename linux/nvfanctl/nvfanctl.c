#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Controlled GPU id
static int gpu_id = 0;

// Returns temperature of a GPU
int get_temp(int id)
{
	char buf[1024];
	snprintf(buf, sizeof buf, "nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader --id=%d", id);
	
	FILE *fp = popen(buf, "r");
	if (fp == NULL)
		raise(SIGINT);
	
	int temp = -1;
	fscanf(fp, "%d", &temp);
	pclose(fp);
	return temp;
}

// Set fan speed
void fan_speed(int id, int percent)
{
	char buf[1024];
	snprintf(buf, sizeof buf, "nvidia-settings -a \"[fan-%d]/GPUTargetFanSpeed=%d\" 2>&1 >/dev/null", id, percent);
	system(buf);
}

// Changes fan control mode (auto/manual)
#define FAN_AUTO   0
#define FAN_MANUAL 1
void fan_ctl(int id, int mode)
{
	char buf[1024];
	snprintf(buf, sizeof buf, "nvidia-settings -a \"[gpu-%d]/GPUFanControlState=%d\" 2>&1 >/dev/null", id, mode);
	system(buf);
}

// Called on exit
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

// Clamped linear speed regulation
static int calc_fan_speed(int temp)
{
	// 60deg - 40%
	// 80deg - 60%
	// speed = (T - 60deg) / 20 deg * 20% + 40%
	int speed = (temp - 60) + 40;
	return speed < 40 ? 40 : speed; 
}

// User-defined fan control logic (must return new fan speed)
// In this case it implements simple hysteresis between T_LOW and T_HIGH temperature values
#define T_LOW 55
#define T_HIGH 65
static int fan_control_logic(int id, int temp, int speed)
{
	if (speed > 0) // Fan already running?
	{
		speed = calc_fan_speed(temp);
		
		// Turn off fan below 50deg
		if (temp < T_LOW) speed = 0;
		
	}
	else
	{
		// Enable fan above T_HIGH
		if (temp > T_HIGH) speed = calc_fan_speed(temp);
	}
	
	return speed;
}

int main(int argc, char **argv)
{
	signal(SIGINT, sigint_handler);
	
	// GPU ID
	const char *gpu_id_str = getenv("GPUID");
	if (gpu_id_str) sscanf(gpu_id_str, "%d", &gpu_id);
	else fprintf(stderr, "assuming GPUID=0...\n");
	
	// Start manual fan control
	fan_ctl(gpu_id, FAN_MANUAL);
	
	// The control loop
	while (1)
	{
		static int speed = 0;
		int temp = get_temp(gpu_id);
		speed = fan_control_logic(gpu_id, temp, speed);
		fan_speed(gpu_id, speed);
		sleep(1);
	}
	
	// Restore auto control
	quit(EXIT_SUCCESS);
	return 0;
}
