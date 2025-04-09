#pragma once

struct AnalysisSnapshot {
    double sma20 = 0.0;
    double sma50 = 0.0;
    double rsi14 = 0.0;
    double macd_line = 0.0;
    double macd_signal = 0.0;
    double upper_band = 0.0;
    double lower_band = 0.0;
    double last_price = 0.0;
    double available_usdt = 0.0;
    bool is_ready = false; // se os dados são válidos
};