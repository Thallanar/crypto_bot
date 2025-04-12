#include "../header/TradeManager.h"
#include "../header/BotInterface.h"
    
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
    // -Obs.: DESCOMENTAR APENAS EM CASOS ONDE HOUVER ALGUM PROBLEMA NAS COMPRAS!-
    // BotInterface::notify_event("[DEBUG - Compra]"
    //     + "has_active_trade: " + order_mgr.has_active_trade + "
    //     + "sma20 > sma50: " + (sma20 > sma50) + " (" + sma20 + " > " + sma50 + ")
    //     + "rsi14 < 30: " + (rsi14 < 30) + " (" + rsi14 + ")
    //     + "price <= lower_band: " + (last_price <= lower_band) + " (" + last_price + " <= " + lower_band + 
    //     + "macd_line > macd_signal: " + (macd_line > macd_signal) + " (" + macd_line + " > " + macd_signal")");
              

    if (order_mgr.active_trades_count < 5 && sma20 > sma50 && rsi14 < 30 && last_price <= lower_band && macd_line > macd_signal && last_price <= order_mgr.average_buy_price) 
    {
        if (available_money == 0)
        {
            BotInterface::notify_event("Não há dinheiro suficiente para a transação!", "ERROR");
            return;
        }
        
        BotInterface::notify_event("📈 SINAL FORTE DE COMPRA 🚀 Executando ordem...", "SIGNAL");
        order_mgr.place_order(symbol, "BUY", 10.0);

    } else if (sma20 < sma50 && rsi14 > 70 && last_price >= upper_band && macd_line < macd_signal) {
        if (last_price <= order_mgr.average_buy_price) 
        {
            BotInterface::notify_event("⚠️ Venda ignorada: preço abaixo do preço médio de compra!", "INFO");
            return;

        } else if (!order_mgr.has_active_trade) {
            BotInterface::notify_event( "📉 SINAL FORTE DE VENDA ❌ Não há posição aberta para vender!", "INFO");
            return;

        } else {
            BotInterface::notify_event( "📉 SINAL FORTE DE VENDA ❌ Executando ordem...", "SIGNAL");
            order_mgr.place_order(symbol, "SELL", 10.0);
        }
    }
}