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

#include <openssl/hmac.h>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <json/json.h>
#include <cpr/cpr.h>
#include <asio/ssl.hpp>
#include <nlohmann/json.hpp>

#include "api/api.h"
#include "OrderManager.h"
#include "QuantitativeAnalyses.h"

#pragma once

class TradeManager 
{
    private:
        API& api;
        QuantitativeAnalyses qa;
        OrderManager& order_mgr;
        std::string symbol;
    
    public:
        TradeManager(API& api_ref, OrderManager& om, const std::string& sym) : api(api_ref), qa(api_ref), order_mgr(om), symbol(sym) {}
    
        void update_price(double price);
    
        void evaluate_signals();
};