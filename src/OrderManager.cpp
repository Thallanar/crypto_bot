#include "../header/OrderManager.h"
#include "../header/TradeManager.h"

char OrderManager::getch_non_blocking()
{
    struct termios oldt, newt;
    char ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // modo raw
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    return ch;
}

void OrderManager::display_status_loop(const std::string& symbol)
{
    while (running)
    {
        AnalysisSnapshot snapshot = trade_mgr->get_analysis_snapshot();

        double current_price = api.fetch_price(symbol);

        std::cout << "\033[2J\033[H"; // limpa tela + posiciona cursor no topo (sem flicker)

        // std::system("clear");
        
        std::cout << "===================== BOT DE TRADING =====================\n";
        std::cout << "Investido: $" << total_invested << "\tPosição: " << (has_active_trade ? "Aberta" : "Nenhuma") << "\n";
        std::cout << "Moeda: " << symbol << "\n";
        std::cout << "Lucro total: $" << total_profit << "\n";
        std::cout << "Preço atual: $" << current_price << "\n";
        std::cout << "Take Profit: $" << last_buy_price * (1.0 + TAKE_PROFIT_PERCENT) << "\n";
        std::cout << "Stop Loss:   $" << last_buy_price * (1.0 - STOP_LOSS_PERCENT) << "\n";
        std::cout << "----------------------------------------------------------\n";
        std::cout << "Moedas em carteira: " << coin_balance << "\n";
        std::cout << "Média de compra: $" << average_buy_price << "\n";
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
}

bool OrderManager::place_order(const std::string& symbol, const std::string& side, double quantity) 
{
    std::string params = "symbol=" + symbol + "&side=" + side + "&type=MARKET&quantity=" + std::to_string(quantity);
    cpr::Response r = api.send_signed_request("/order", params, "POST");

    if (r.status_code == 200) 
    {
        auto json = nlohmann::json::parse(r.text);
        double executed_price = std::stod(json["fills"][0]["price"].get<std::string>());

        std::cout << "Ordem executada: " << r.text << std::endl;
        if (side == "BUY") 
        {
            if (active_trades_count >= max_active_trades)
            {
                std::cout << "⚠️ Limite de compras simultâneas atingido (" << max_active_trades << "). Aguardando liquidação...\n";
                return false;
            }

            has_active_trade = true;

            double quote_spent = quantity * executed_price;

            coin_balance += quantity;
            total_invested += quote_spent;
            average_buy_price = total_invested / coin_balance;
            
            last_buy_price = executed_price;
            active_trades_count++;
            buy_count++;

            std::cout << "[🟢 COMPRA EXECUTADA] " << quantity << " " << symbol << " a " << executed_price 
              << " | Total de " << coin_balance << " moedas acumuladas | Investimento total: $" 
              << total_invested << std::endl;
        } else if (side == "SELL") {
            if (active_trades_count > 0)
            {
                active_trades_count--;
            }

            double profit = (executed_price - average_buy_price) * quantity;
            total_profit += profit;

            coin_balance -= quantity;
            total_invested -= average_buy_price * quantity;

            if (coin_balance <= 1e-6) 
            {
                has_active_trade = false;
                coin_balance = 0.0;
                total_invested = 0.0;
                average_buy_price = 0.0;
            }

            std::cout << "[🔴 VENDA EXECUTADA] " << quantity << " " << symbol << " a " << executed_price 
              << " | Lucro/prejuízo: $" << profit 
              << " | Total restante: " << coin_balance << " moedas" 
              << " | Investimento restante: $" << total_invested 
              << " | Lucro total: $" << total_profit << std::endl;
        } else {
            has_active_trade = false;
        }
    } else {
        std::cerr << "Erro ao executar ordem: " << r.status_code << " - " << r.text << std::endl;
    }

    return true;
}

    
void OrderManager::check_risk_management(double current_price, const std::string& symbol) 
{
    if (!has_active_trade) return;

    double stop_loss_price = last_buy_price * (1.0 - STOP_LOSS_PERCENT);
    double take_profit_price = last_buy_price * (1.0 + TAKE_PROFIT_PERCENT);

    if (current_price <= stop_loss_price) 
    {
        std::cout << "🛑 Stop Loss acionado. Vendendo para limitar perdas!" << std::endl;
        place_order(symbol, "SELL", coin_balance);
    } else if (current_price >= last_buy_price * (1.0 + TAKE_PROFIT_PERCENT)) {
        std::cout << "🎯 Take Profit alcançado. Vendendo para garantir lucros!" << std::endl;
        place_order(symbol, "SELL", coin_balance);
    }
}

void OrderManager::user_input_loop(const std::string& symbol)
{
    std::cout << "\n🎮 Controle Manual Ativado:\n";
    std::cout << "  [c] Comprar manualmente\n";
    std::cout << "  [v] Vender manualmente\n";
    std::cout << "  [q] Parar o bot\n";

    while (is_running())
    {
        char ch = getch_non_blocking();
        if (ch == 'c')
        {
            std::cout << "\n🟢 Compra manual executada!\n";
            place_order(symbol, "BUY", 10); // pode ajustar o amount
        }
        else if (ch == 'v')
        {
            std::cout << "\n🔴 Venda manual executada!\n";
            place_order(symbol, "SELL", 10);
        }
        else if (ch == 'q')
        {
            std::cout << "\n⏹️ Encerrando...\n";
            stop();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void OrderManager::print_report() const 
{
    std::cout << "\n------------------------------------------";
    std::cout << "\n📊 [RELATÓRIO FINAL] 📊\n";
    std::cout << "Total de compras: " << buy_count << std::endl;
    std::cout << "Moedas restantes: " << coin_balance << std::endl;
    std::cout << "Investimento restante: $" << total_invested << std::endl;
    std::cout << "Lucro/prejuízo total acumulado: $" << total_profit << std::endl;
    std::cout << "Média de compra atual: " << average_buy_price << std::endl;
    std::cout << "------------------------------------------\n";
}