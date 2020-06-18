#include <stdio.h>
#include <wiringPi.h>
#include <time.h>
#include <syslog.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <sys/time.h>

#define VCC_PIN 4
#define INP_PIN 5 

/*
	The brown-out data managed by the ISR
*/
static atomic_flag isr_lock = ATOMIC_FLAG_INIT;
struct timeval t_start;
struct timeval t_end;
struct timeval t_duration;
static bool brown_out_pending = false;
static const char *out_filename = NULL;

/*
	Called from the ISR
*/
void bodlog(void)
{
	double ms = t_duration.tv_sec * 1000.0 + t_duration.tv_usec / 1000.0;

	// Always log to stdout
	fprintf(stdout, "%lu %.2f ms\n", time(NULL), ms);

	// Log to file if name is specified
	if (out_filename != NULL)
	{
		FILE *f = fopen(out_filename, "a");
		if (f == NULL) perror("could not open log file!");
		fprintf(f, "%lu of %.2f ms\n", time(NULL), ms);
		if (fclose(f)) perror("could not close log file!");
	}

	// Log to syslog
	syslog(LOG_ALERT, "brown out detected: %.2f ms", ms);
}

/*
	Handles pin interrupts
*/
void isr_handler(void)
{
	int input = digitalRead(INP_PIN);

	// Wait for the 'mutex'
	while (atomic_flag_test_and_set(&isr_lock));

	// Check current pin state
	if (input)
	{
		// Rising edge - end of brown out - log
		if (brown_out_pending)
		{
			gettimeofday(&t_end, NULL);
			timersub(&t_end, &t_start, &t_duration);
			brown_out_pending = false;
			bodlog();
		}
	}
	else
	{
		// Falling edge - start of a brown out
		gettimeofday(&t_start, NULL);
		brown_out_pending = true;
	}

	// Free the lock
	atomic_flag_clear(&isr_lock);
}

/*
	Handles SIGINT nicely 
*/
static bool alive = true;
void sig_handler(int signum)
{
	alive = false;
}

int main(int argc, char **argv)
{
	wiringPiSetup();
	pinMode(VCC_PIN, OUTPUT);
	pinMode(INP_PIN, INPUT);
	digitalWrite(VCC_PIN, 1);

	// Logs init
	openlog("bod", LOG_NDELAY, LOG_USER);
	out_filename = argc > 1 ? argv[1] : NULL;

	// Start the ISR
	wiringPiISR(INP_PIN, INT_EDGE_BOTH, &isr_handler);

	while (alive)
	{
		// Sleep 50ms
		struct timespec ts = {
			.tv_sec = 0,
			.tv_nsec = 50e6
		};
		nanosleep(&ts, NULL);
	}

	// Close syslog
	closelog( );
}
