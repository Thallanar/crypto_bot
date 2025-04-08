#include <iostream>
#include <csignal>
#include <atomic>

#include "../header/api/api.h"
#include "../header/OrderManager.h"
#include "../header/QuantitativeAnalyses.h"
#include "../header/TradeManager.h"

OrderManager* global_order_mgr = nullptr; 

void signal_handler(int signal) 
{
    if (signal == SIGINT && global_order_mgr) 
    {
        std::cout << "\n\n🔌 Interrupção detectada (CTRL+C). Finalizando...\n";
        global_order_mgr->print_report();
        exit(0);  // encerra o programa
    }
}

int main() 
{
    std::string symbol = "XRPUSDT";
    API api;
    OrderManager order_mgr(api);
    TradeManager trade_mgr(api, order_mgr, symbol);

    global_order_mgr = &order_mgr;   // linka para uso no handler
    signal(SIGINT, signal_handler);  // registra a função

    double current_price;

    api.fetch_price(symbol);      // Busca o preço inicial

    std::cout << "Conectando ao WebSocket da Binance..." << std::endl;
    std::cout << "Moeda a ser trocada: " << symbol << std::endl;

    api.run_websocket(symbol);    // Inicia o WebSocket em uma thread separada
    
    if (!order_mgr.has_active_trade) 
    {
        std::cout << "Iniciando compra..." << std::endl;
        order_mgr.place_order(symbol, "BUY", 10.0);
    }

    
    // Loop principal para manter a execução
    while (true) 
    {
        current_price = api.fetch_price(symbol);
        trade_mgr.update_price(current_price);
        order_mgr.check_risk_management(current_price, symbol);
        trade_mgr.evaluate_signals();
        sleep(2); // Espera 5 segundos antes de verificar novamente
    }

   return 0;
}
