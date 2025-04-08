#include <iostream>
#include <sstream>
#include <thread>
#include <deque>
#include <vector>
#include <numeric>
#include <cmath>
#include <fstream>
#include <chrono>
#include <ctime>
#include <map>
#include <string>

#include <openssl/hmac.h>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <json/json.h>
#include <cpr/cpr.h>
#include <asio/ssl.hpp>
#include <nlohmann/json.hpp>

#include "../../constructor/trade.h"

#pragma once

class API 
{
    private:
        typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
        const std::string BASE_URL = "https://testnet.binance.vision/api/v3";
    
        std::map<std::string, std::string> load_env(const std::string& filename);
    
    public:
        API();
           
        std::string api_key;
        std::string secret_key;
        
        std::deque<double> price_buffer;
        const size_t MAX_BUFFER_SIZE = 50;          // Tamanho do buffer para os cálculos  

        std::string get_timestamp(); 
        
        std::string timestamp_to_string(time_t timestamp); 
        
        websocketpp::lib::shared_ptr<boost::asio::ssl::context> on_tls_init(std::weak_ptr<void>); 

        double extract_price(const std::string& response_text);

        std::string hmac_sha256(const std::string &key, const std::string &data);

        void on_message(websocketpp::connection_hdl hdl, client::message_ptr msg);

        void connect_websocket(const std::string symbol);

        void run_websocket(const std::string symbol);
        
        cpr::Response send_signed_request(const std::string& endpoint, const std::string& params, const std::string& method);

        // Função para buscar o preço atual da moeda escolhida via API REST
        double fetch_price(const std::string symbol);

        double get_balance(const std::string asset);
};