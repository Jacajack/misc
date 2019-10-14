#include "url_request.hpp"
#include <algorithm>
#include <stdexcept>

// The write callback function
size_t url_request::write_callback( void *contents, size_t size, size_t nmemb, void *userp )
{
	size_t realsize = size * nmemb;
	url_request *rq = static_cast<url_request*>( userp );
	std::uint8_t *b = static_cast<std::uint8_t*>( contents );
	std::uint8_t *e = b + realsize;
	std::copy( b, e, std::back_inserter( rq->m_data ) );
	
	return realsize;
}

url_request::url_request( const std::string &url, long timeout_ms ) :
	m_curl( curl_easy_init( ), [](CURL *c){ curl_easy_cleanup( c ); } )
{
	// Handle failed init
	if ( !m_curl )
		throw std::runtime_error( "curl_easy_init() error!" );
	
	// Set options
	curl_easy_setopt( m_curl.get( ), CURLOPT_URL, url.c_str( ) );
	curl_easy_setopt( m_curl.get( ), CURLOPT_USERAGENT, "libcurl-agent/1.0" );
	curl_easy_setopt( m_curl.get( ), CURLOPT_TIMEOUT_MS, timeout_ms );
	curl_easy_setopt( m_curl.get( ), CURLOPT_WRITEFUNCTION, url_request::write_callback );
	curl_easy_setopt( m_curl.get( ), CURLOPT_WRITEDATA, static_cast<void*>( this ) );
	
	// Perform the request
	m_result = curl_easy_perform( m_curl.get( ) );
}

bool url_request::get_success( ) const
{
	return m_result == CURLE_OK;
}

const std::vector<std::uint8_t> &url_request::get_data( ) const
{
	return m_data;
}

std::string url_request::get_string( ) const
{
	return std::string( m_data.begin( ), m_data.end( ) );
}
