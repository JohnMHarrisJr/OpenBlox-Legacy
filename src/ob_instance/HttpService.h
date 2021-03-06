#ifndef OB_INSTANCE_HTTPSERVICE_H_
#define OB_INSTANCE_HTTPSERVICE_H_

#include "Instance.h"

#include "../ob_type/WebSocket.h"

namespace ob_instance{
	class HttpService: public Instance{
		public:
			struct response_body{
				size_t size;
				char* data;
			};

			HttpService();
			virtual ~HttpService();

			static size_t write_data(void* ptr, size_t size, size_t nmemb, struct response_body* data);

			std::string GetAsync(std::string url, bool nocache = false);
			std::string PostAsync(std::string url, std::string data, ob_enum::HttpContentType content_type = ob_enum::HttpContentType::ApplicationJson);

			std::string UrlEncode(std::string input);
			std::string UrlDecode(std::string input);

			ob_type::WebSocket* CreateWebSocket(const char* uri);

			std::string GenerateGUID(bool wrapInCurlyBraces = true);

			DECLARE_CLASS(HttpService);

			static void register_lua_methods(lua_State* L);
	};
}
#endif
