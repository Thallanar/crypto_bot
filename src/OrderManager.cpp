#include "../header/OrderManager.h"
#include "../header/TradeManager.h"
#include "../header/BotInterface.h"
#include "../header/FileManager.h"

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



bool OrderManager::place_order(const std::string& symbol, const std::string& side, double quantity) 
{
    std::string params = "symbol=" + symbol + "&side=" + side + "&type=MARKET&quantity=" + std::to_string(quantity);
    cpr::Response r = api.send_signed_request("/order", params, "POST");

    double price = api.fetch_price(symbol);
    double cost = price * quantity;

    if (r.status_code == 200) 
    {
        auto json = nlohmann::json::parse(r.text);
        double executed_price = std::stod(json["fills"][0]["price"].get<std::string>());

        if (side == "BUY") 
        {
            if (active_trades_count >= max_active_trades)
            {
                BotInterface::notify_event("⚠️ Limite de compras simultâneas atingido (" + std::to_string(max_active_trades) + "). Aguardando liquidação...", "ERROR");
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

            std::stringstream log;
            log << "Quantidade: " << quantity << ", Preço: $" << price;
            file_mgr->trade_log(symbol, "BUY", log.str());

            BotInterface::notify_event("[🟢 COMPRA EXECUTADA] " + std::to_string(quantity) + " " + symbol + " a " + std::to_string(executed_price), "BUY");

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

            std::stringstream log;
            log << "[SELL] Quantidade: " << quantity << ", Preço: $" << price << ", Lucro: $" << profit;
            file_mgr->trade_log(symbol, "SELL", log.str());

            BotInterface::notify_event ("[🔴 VENDA EXECUTADA] " + std::to_string(quantity) + " " + symbol + " a " + std::to_string(executed_price), "SELL");

        } else {
            has_active_trade = false;
        }
    } else {
        BotInterface::notify_event("Erro ao executar ordem: " + std::to_string(r.status_code), "ERROR");
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
        BotInterface::notify_event( "🛑 Stop Loss acionado. Vendendo para limitar perdas!", "SELL");
        place_order(symbol, "SELL", coin_balance);
    } else if (current_price >= last_buy_price * (1.0 + TAKE_PROFIT_PERCENT)) {
        BotInterface::notify_event( "🎯 Take Profit alcançado. Vendendo para garantir lucros!", "SELL");
        place_order(symbol, "SELL", coin_balance);
    }
}

void OrderManager::user_input_loop(const std::string& symbol)
{
    while (is_running())
    {
        char ch = getch_non_blocking();
        if (ch == 'c')
        {
            BotInterface::notify_event("\n🟢 Compra manual executada!\n", "BUY");
            place_order(symbol, "BUY", 10); // pode ajustar o amount
        }
        else if (ch == 'v')
        {
            BotInterface::notify_event("\n🔴 Venda manual executada!\n", "SELL");
            place_order(symbol, "SELL", 10);
        }
        else if (ch == 'q')
        {
            BotInterface::notify_event("\n⏹️ Encerrando...\n", "SIGNAL");
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