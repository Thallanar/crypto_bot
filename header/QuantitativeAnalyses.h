#include <iostream>
#include <sstream>
#include <thread>
#include <deque>
#include <vector>
#include <numeric>
#include <cmath>

#include <openssl/hmac.h>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <json/json.h>
#include <cpr/cpr.h>
#include <asio/ssl.hpp>
#include <nlohmann/json.hpp>

#include "api/api.h"

#pragma once

class QuantitativeAnalyses 
{
    private:
        API& api;
        
        std::deque<double>macd_history;
        const size_t MAX_MACD_HISTORY = 100;
        
    public:
        QuantitativeAnalyses(API& api_ref) : api(api_ref) {}

        double calculate_sma(int period);
        
        double calculate_ema(int period);
        
        double calculate_rsi(int period);

        void calculate_bollinger_bands(double& upper_band, double& lower_band);

        double calculate_macd(double& signal_macd);

        double calculate_ema_custom(const std::deque<double>& data, int period);
};
