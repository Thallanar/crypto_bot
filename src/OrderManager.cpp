#include "../header/OrderManager.h"

void OrderManager::place_order(const std::string& symbol, const std::string& side, double quantity) 
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
            has_active_trade = true;

            double quote_spent = quantity * executed_price;

            average_buy_price = ((average_buy_price * buy_count) + executed_price) / (buy_count + 1);
            coin_balance += quantity;
            total_invested += quote_spent;
            
            last_buy_price = executed_price;
            buy_count++;

            std::cout << "[🟢 COMPRA EXECUTADA] " << quantity << " " << symbol << " a " << executed_price 
              << " | Total de " << coin_balance << " moedas acumuladas | Investimento total: $" 
              << total_invested << std::endl;
        } else if (side == "SELL") {
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