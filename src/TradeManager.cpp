#include "../header/TradeManager.h"
    
void TradeManager::update_price(double price) 
{
    api.price_buffer.push_back(price);
    if (api.price_buffer.size() > api.MAX_BUFFER_SIZE) 
    {
        api.price_buffer.erase(api.price_buffer.begin());
    }
}

AnalysisSnapshot TradeManager::get_analysis_snapshot() 
{
    AnalysisSnapshot snapshot;

    if (api.price_buffer.empty()) return snapshot;

    double upper_band, lower_band;
    double macd_signal;

    snapshot.available_usdt = api.get_balance("USDT");

    snapshot.sma20 = qa.calculate_sma(20);
    snapshot.sma50 = qa.calculate_sma(50);
    snapshot.rsi14 = qa.calculate_rsi(14);
    snapshot.macd_line = qa.calculate_macd(macd_signal);
    snapshot.macd_signal = macd_signal;
    snapshot.last_price = api.price_buffer.back();
    
    qa.calculate_bollinger_bands(upper_band, lower_band);
    snapshot.upper_band = upper_band;
    snapshot.lower_band = lower_band;

    snapshot.is_ready = true;
    return snapshot;
}
    
void TradeManager::evaluate_signals() 
{
    if (api.price_buffer.size() < 50) return;

    double upper_band, lower_band;
    double macd_signal;

    double available_money = api.get_balance("USDT");

    double sma20 = qa.calculate_sma(20);
    double sma50 = qa.calculate_sma(50);
    double rsi14 = qa.calculate_rsi(14);
    double macd_line = qa.calculate_macd(macd_signal);
    double last_price = api.price_buffer.back();

    qa.calculate_bollinger_bands(upper_band, lower_band);

    //---------------------------------------------------------------------------
    //------------------------->  Debug de Compra <------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------    
    //-Obs.: DESCOMENTAR APENAS EM CASOS ONDE HOUVER ALGUM PROBLEMA NAS COMPRAS!-
    // std::cout << "\n[DEBUG - Compra]\n";
    // std::cout << "has_active_trade: " << order_mgr.has_active_trade << "\n";
    // std::cout << "sma20 > sma50: " << (sma20 > sma50) << " (" << sma20 << " > " << sma50 << ")\n";
    // std::cout << "rsi14 < 30: " << (rsi14 < 30) << " (" << rsi14 << ")\n";
    // std::cout << "price <= lower_band: " << (last_price <= lower_band) << " (" << last_price << " <= " << lower_band << ")\n";
    // std::cout << "macd_line > macd_signal: " << (macd_line > macd_signal) << " (" << macd_line << " > " << macd_signal << ")\n";
              

    if (order_mgr.active_trades_count < 5 && sma20 > sma50 && rsi14 < 30 && last_price <= lower_band && macd_line > macd_signal && last_price <= order_mgr.average_buy_price) 
    {
        if (available_money == 0)
        {
            std::cout << "Não há dinheiro suficiente para a transação!" << std::endl;
            return;
        }
        
        std::cout << "📈 SINAL FORTE DE COMPRA 🚀 Executando ordem..." << std::endl;
        order_mgr.place_order(symbol, "BUY", 10.0);

    } else if (sma20 < sma50 && rsi14 > 70 && last_price >= upper_band && macd_line < macd_signal) {
        if (last_price <= order_mgr.average_buy_price) 
        {
            std::cout << "⚠️ Venda ignorada: preço abaixo do preço médio de compra!" << std::endl;
            return;
        } else if (!order_mgr.has_active_trade) {
            std::cout << "📉 SINAL FORTE DE VENDA ❌ Não há posição aberta para vender!" << std::endl;
            return;
        } else {
            std::cout << "📉 SINAL FORTE DE VENDA ❌ Executando ordem..." << std::endl;
            order_mgr.place_order(symbol, "SELL", 10.0);
        }
    }
}