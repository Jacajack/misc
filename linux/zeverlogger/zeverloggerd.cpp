#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <ctime>
#include <sstream>
#include <mutex>
#include <thread>
#include <fstream>
#include <chrono>
#include <signal.h>
#include <unistd.h>
#include "url_request.hpp"

struct data_frame
{
	data_frame( const std::string &text, std::time_t t );
	
	std::time_t unix_time;
	std::string time_string;
	float power;
	float energy;
};

data_frame::data_frame( const std::string &text, std::time_t t ) :
	unix_time( t )
{
	std::istringstream ss( text );
	
	bool complete = false;
	int linecnt = 0;
	std::string line;
	while ( linecnt++, std::getline( ss, line ) )
	{
		std::istringstream ss( line );
		
		switch ( linecnt )
		{
			case 11:
				ss >> power;
				break;
				
			case 12:
				ss >> energy;
				break;
		
			case 14:
				complete = true;
				break;
		}
		
		if ( !ss )
		{
			complete = false;
			break;
		}
		
	}
	
	if ( !complete )
		throw std::runtime_error( "Data frame incomplete" );
}

std::ostream &operator<<( std::ostream &s, const data_frame &frame )
{
	s << frame.unix_time << " " << frame.energy << " " << frame.power;
	return s;
}







std::vector<data_frame> data;
void do_request( const std::string &ip )
{
	try
	{
		url_request rq( ip + "/home.cgi", 2000 );
		if ( !rq.get_success( ) )
			return;
		data.emplace_back( rq.get_string( ), std::time( nullptr ) );
	}
	catch ( ... )
	{
	}
}

std::mutex dump_mutex;
void dump_data( const std::string &path )
{
	std::lock_guard<std::mutex> lg( dump_mutex );
	std::ofstream f( path, std::ios::app );
	if ( !f )
		throw std::runtime_error( "could not open output file!" );
	
	for ( const auto &d : data )
		f << d << std::endl;
	data.clear( );
	data.shrink_to_fit( );
}

void signal_handler( int signum );
void signal_setup( )
{
	signal( SIGUSR1, signal_handler );
	signal( SIGUSR2, signal_handler );
	signal( SIGINT, signal_handler );
}


bool should_terminate = false;
bool should_request = false;
bool should_dump = false;
void signal_handler( int signum )
{
	switch ( signum )
	{
		case SIGINT:
			should_terminate = true;
			break;
			
		case SIGUSR1:
			should_request = true;
			break;
			
		case SIGUSR2:
			should_dump = true;
			break;
			
		default:
			break;
	}

	signal_setup( );
}

int main( int argc, char *argv[] )
{
	if ( argc < 3 )
	{
		std::cerr << "Usage: loggerd [IP] [OUTPUT_PATH] [INTERVAL=10] [DAEMONIZE=0]" << std::endl;
		return EXIT_FAILURE;
	}

	std::string ip( argv[1] );
	std::string output_path( argv[2] );
	int request_interval = argc > 3 ? std::atoi( argv[3] ) : 10;
	bool should_daemonize = argc > 4 ? std::atoi( argv[4] ) : 0;

	// Daemonization
	if ( should_daemonize )
	{
		if ( daemon( true, false ) == -1 )
			throw std::runtime_error( "daemon() call failed!" );
	}
	signal_setup( );

	
	// CURL init
	curl_global_init( CURL_GLOBAL_ALL );	
	
	while ( !should_terminate || !data.empty( ) )
	{	
		std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
		
		static std::time_t last_dump = 0;
		std::time_t t = std::time( nullptr );
		if ( should_request || ( t != last_dump && t % request_interval == 0 ) )
			do_request( ip ), should_request = false, last_dump = t;
		
		if ( should_dump || should_terminate )
			dump_data( output_path ), should_dump = false;
	}
	
	// CURL cleanup
	curl_global_cleanup( );

	return EXIT_SUCCESS;
}
