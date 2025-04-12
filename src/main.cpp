#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>

#include "../header/api/api.h"
#include "../header/OrderManager.h"
#include "../header/QuantitativeAnalyses.h"
#include "../header/TradeManager.h"
#include "../header/BotInterface.h"
#include "../header/FileManager.h"

OrderManager* global_order_mgr = nullptr; 

void signal_handler(int signal) 
{
    if (signal == SIGINT && global_order_mgr) 
    {
        std::cout << "\n\n🔌 Interrupção detectada (CTRL+C). Finalizando...\n";
        global_order_mgr->stop();
        global_order_mgr->print_report();
        exit(0);  // encerra o programa
    }
}

int main() 
{
    std::string symbol;

    std::system("clear");

    std::cout << "==================================================\n";
    std::cout << "      BOT DE TRADING - SELEÇÃO DE CRIPTOMOEDA\n";
    std::cout << "==================================================\n";
    std::cout << "        Alguns exemplos de símbolos válidos:\n";
    std::cout << "\n  - BTCUSDT (Bitcoin)\n";
    std::cout << "  - ETHUSDT (Ethereum)\n";
    std::cout << "  - ADAUSDT (Cardano)\n";
    std::cout << "  - BNBUSDT (Binance Coin)\n";
    std::cout << "  - XRPUSDT (Ripple)\n";
    std::cout << "\nDigite o símbolo da criptomoeda que deseja operar: ";
    std::getline(std::cin, symbol);

    symbol.erase(0, symbol.find_first_not_of(" \t\n\r"));
    symbol.erase(symbol.find_last_not_of(" \t\n\r") + 1);

    API api;
    FileManager file_manager(symbol);
    OrderManager order_mgr(api);
    TradeManager trade_mgr(api, order_mgr, symbol);
    BotInterface bot_interface(api, order_mgr, symbol);

    order_mgr.trade_mgr = &trade_mgr;
    order_mgr.file_mgr = &file_manager;
    bot_interface.trade_mgr = &trade_mgr;
    
    std::thread websocket_thread([&](){ api.run_websocket(symbol); });
    std::thread painel_thread(&BotInterface::display_status_loop, &bot_interface, symbol);
    std::thread input_thread(&OrderManager::user_input_loop, &order_mgr, symbol);
    
    std::this_thread::sleep_for(std::chrono::seconds(1)); 
    
    global_order_mgr = &order_mgr;   // linka para uso no handler
    signal(SIGINT, signal_handler);  // registra a função

    double current_price = api.fetch_price(symbol);      // Busca o preço inicial
    
    std::cout << "Conectando ao WebSocket da Binance..." << std::endl;
    std::cout << "Moeda a ser trocada: " << symbol << std::endl;

    // Loop principal para manter a execução
    while (order_mgr.is_running()) 
    {
        current_price = api.fetch_price(symbol);
        trade_mgr.update_price(current_price);
        order_mgr.check_risk_management(current_price, symbol);
        trade_mgr.evaluate_signals();
        sleep(2);
    }

    order_mgr.print_report();

    websocket_thread.join();
    painel_thread.join();
    input_thread.join();

    return 0;
}
