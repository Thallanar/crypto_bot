#include "../header/BotInterface.h"

void BotInterface::display_status_loop(const std::string& symbol)
{
    while (running)
    {
        AnalysisSnapshot snapshot = trade_mgr->get_analysis_snapshot();

        double current_price = api.fetch_price(symbol);

        // std::cout << "\033[2J\033[H"; // limpa tela + posiciona cursor no topo (sem flicker)

        std::system("clear");
        
        std::cout << "===================== BOT DE TRADING =====================\n";
        std::cout << "Investido: $" << order_manager.total_invested << "\tPosição: " << (order_manager.has_active_trade ? "Aberta" : "Nenhuma") << "\n";
        std::cout << "Moeda: " << symbol << "\n";
        std::cout << "Lucro total: $" << order_manager.total_profit << "\n";
        std::cout << "Preço atual: $" << current_price << "\n";
        std::cout << "Take Profit: $" << order_manager.last_buy_price * (1.0 + order_manager.TAKE_PROFIT_PERCENT) << "\n";
        std::cout << "Stop Loss:   $" << order_manager.last_buy_price * (1.0 - order_manager.STOP_LOSS_PERCENT) << "\n";
        std::cout << "----------------------------------------------------------\n";
        std::cout << "Moedas em carteira: " << order_manager.coin_balance << "\n";
        std::cout << "Média de compra: $" << order_manager.average_buy_price << "\n";
        std::cout << "----------------------------------------------------------\n";

        if (snapshot.is_ready)
        {
            std::cout << "[Saldo]: $" << snapshot.available_usdt
            << "\t[SMA(20)]: " << snapshot.sma20
            << "\t[SMA(50)]: " << snapshot.sma50
            << "\t[RSI(14)]: " << snapshot.rsi14
            << "\t[MACD]: " << snapshot.macd_line
            << "\t[MACD SIGNAL]: " << snapshot.macd_signal
            << "\t[Bandas de Bollinger]:" << snapshot.lower_band << " - " << snapshot.upper_band 
            << "\t[Preço Atual]: $" << snapshot.last_price << "\n";
        } else {
            std::cout << "Carregando dados do mercado... \n";
        }
        
        std::cout << "----------------------------------------------------------\n";
        std::cout << "[c] Comprar manualmente | [v] Vender manualmente | [q] Sair\n";
        std::cout << "==========================================================\n";

        std::cout << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
};

void BotInterface::notify_event(const std::string& message, const std::string& type = "INFO") 
{
    // Aqui você pode futuramente armazenar logs num buffer,
    // ou atualizar a área da interface que mostra eventos.
    std::string prefix;

    if (type == "BUY") prefix = "[🟢 COMPRA]";
    else if (type == "SELL") prefix = "[🔴 VENDA]";
    else if (type == "ERROR") prefix = "[❌ ERRO]";
    else if (type == "SIGNAL") prefix = "[📡 SINAL]";
    else prefix = "[INFO]";

    // Por enquanto, só retornamos uma string limpa (sem printar direto)
    std::cout << prefix << " " << message << std::endl;
};