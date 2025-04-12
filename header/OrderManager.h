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
#include <atomic>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <openssl/hmac.h>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <json/json.h>
#include <cpr/cpr.h>
#include <asio/ssl.hpp>
#include <nlohmann/json.hpp>

#include "../constructor/AnalysisSnapshot.h"

#include "api/api.h"

#pragma once

class TradeManager;
class FileManager;

class OrderManager 
{
    private:
        API& api;
        
        bool running = true;

        char getch_non_blocking();
        
    public:
        OrderManager(API& api_ref) : api(api_ref) {};

        TradeManager* trade_mgr = nullptr;
        FileManager* file_mgr = nullptr;

        int max_active_trades = 5;
        int active_trades_count = 0;
        double last_buy_price = 0.0;
        double average_buy_price = 0.0;
        double total_invested = 0.0;
        double total_profit = 0.0;
        int coin_balance = 0;
        int buy_count = 0;


        bool has_active_trade = false;
        const double STOP_LOSS_PERCENT = 0.02;
        const double TAKE_PROFIT_PERCENT = 0.05;

        bool place_order(const std::string& symbol, const std::string& side, double quantity);
    
        void check_risk_management(double current_price, const std::string& symbol);

        void user_input_loop(const std::string& symbol);

        void print_report() const;

        void stop() { running = false; };

        bool is_running() const { return running; };
};