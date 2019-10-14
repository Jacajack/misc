#ifndef URL_REQUEST_HPP
#define URL_REQUEST_HPP

#include <vector>
#include <memory>
#include <string>
#include <curl/curl.h>

// URL request RAII wrapper class
class url_request
{
	public:
		url_request( const std::string &url, long timeout_ms = 5000 );
		
		url_request( const url_request &src ) = delete;
		url_request &operator=( const url_request &rhs ) = delete;
		
		url_request( url_request &&src ) = default;
		url_request &operator=( url_request &&rhs ) = default;
		
		bool get_success( ) const;
		const std::vector<std::uint8_t> &get_data( ) const;
		std::string get_string( ) const;
		
	private:
		static size_t write_callback( void *contents, size_t size, size_t nmemb, void *userp );
	
		std::unique_ptr<CURL, void(*)(CURL*)> m_curl;
		std::vector<std::uint8_t> m_data;
		CURLcode m_result;
};

#endif
