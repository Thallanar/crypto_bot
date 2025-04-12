#include <iostream>
#include <chrono>

#include "api.h"
#include "TradeManager.h"
#include "OrderManager.h"
#include "../constructor/AnalysisSnapshot.h"

#pragma once

class TradeManager;

class BotInterface
{
    private:
        API& api;
        OrderManager& order_manager;

        bool running = true;
        std::string symbol;

    public:
        BotInterface(API& api_ref, OrderManager& om_ref, std::string symbol_ref) : api(api_ref), order_manager(om_ref), symbol(symbol_ref) {};

        TradeManager* trade_mgr = nullptr;
        
        void display_status_loop(const std::string& symbol);
        
        static void notify_event(const std::string& message, const std::string& type);
};