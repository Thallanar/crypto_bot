#include <iostream>

#pragma once

struct Trade 
{
    std::string date;           // Data e hora da operação
    std::string type;           // "buy" ou "sell"
    double entry_price;         // Preço de entrada
    double exit_price;          // Preço de saída (para venda)
    double profit_percent;      // Percentual de lucro
    double rsi;                 // RSI no momento da operação
    double sma;                 // SMA no momento da operação
    double macd;                // MACD no momento da operação
    bool stop_loss_triggered;   // Indica se o stop loss foi atingido
    bool take_profit_triggered; // Indica se o take profit foi atingido
    int duration;               // Duração da operação em candles

    Trade
    (
        std::string type,
        std::string date,
        double entry_price,         
        double exit_price,           
        double profit_percent,      
        double rsi,                 
        double sma,                 
        double macd,
        bool stop_loss_triggered,      
        bool take_profit_triggered,   
        int duration     
    ) : type(type), date(date), entry_price(entry_price), exit_price(exit_price), profit_percent(profit_percent), rsi(rsi), sma(sma), macd(macd), stop_loss_triggered(stop_loss_triggered), take_profit_triggered(take_profit_triggered), duration(duration) {}
};