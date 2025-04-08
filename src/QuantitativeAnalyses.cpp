#include "../header/QuantitativeAnalyses.h"

double QuantitativeAnalyses::calculate_sma(int period) 
{
    if (api.price_buffer.size() < period) return 0;  // Não há dados suficientes

    double sum = std::accumulate(api.price_buffer.end() - period, api.price_buffer.end(), 0.0);
    return sum / period;
}

        
double QuantitativeAnalyses::calculate_ema(int period) 
{
    if (api.price_buffer.size() < period) return 0;

    double multiplier = 2.0 / (period + 1);
    double ema = api.price_buffer[api.price_buffer.size() - period];  // Começa com o primeiro valor da SMA

    for (int i = api.price_buffer.size() - period + 1; i < api.price_buffer.size(); i++) 
    {
        ema = (api.price_buffer[i] - ema) * multiplier + ema;
    }

    return ema;
}
        
double QuantitativeAnalyses::calculate_rsi(int period) 
{
    if (api.price_buffer.size() < period + 1) return 0;

    double gain = 0, loss = 0;
    
    for (size_t i = api.price_buffer.size() - period; i < api.price_buffer.size() - 1; i++) 
    {
        double change = api.price_buffer[i + 1] - api.price_buffer[i];
        if (change > 0) gain += change;
        else loss -= change;  // Transformando em valor positivo
    }

    if (loss == 0) return 100;  // Evita divisão por zero

    double rs = gain / loss;
    return 100 - (100 / (1 + rs));
}

//
// Calcula Bandas de Bollinger (BB)
// Sendo:
//   Compra: BB e o RSI < 30.
//   Venda: BB e RSI > 70.
//
void QuantitativeAnalyses::calculate_bollinger_bands(double& upper_band, double& lower_band) 
{
    int period = 20;
    if (api.price_buffer.size() < period) return;

    double sma = calculate_sma(period);

    // Calcula o desvio padrão
    double sum_squared_diff = 0;
    for (size_t i = api.price_buffer.size() - period; i < api.price_buffer.size(); i++) 
    {
        sum_squared_diff += pow(api.price_buffer[i] - sma, 2);
    }
    double stddev = sqrt(sum_squared_diff / period);

    upper_band = sma + (2 * stddev);
    lower_band = sma - (2 * stddev);
}
        
//
//  Calcula a MACD (Moving Average Convergence Divergence)
//  Sendo:
//      Compra: MACD > Linha de Sinal
//      Venda: MACD < Linha de Sinal
//
double QuantitativeAnalyses::calculate_macd(double& signal_macd)
{
    if (api.price_buffer.size() < 26) return 0.0;

    double macd_line = calculate_ema(12) - calculate_ema(26);

    // Armazena a linha MACD no histórico
    macd_history.push_back(macd_line);
    if (macd_history.size() > MAX_MACD_HISTORY) 
    {
        macd_history.pop_front();
    }

    // Calcula o sinal da MACD com base no histórico
    if (macd_history.size() >= 9) 
    {
        signal_macd = calculate_ema_custom(macd_history, 9);
    } else {
        signal_macd = 0.0;
    }

    return macd_line;
}

double QuantitativeAnalyses::calculate_ema_custom(const std::deque<double>& data, int period)
{
    if (data.size() < period) return 0.0;

    double multiplier = 2.0 / (period + 1);
    double ema = std::accumulate(data.end() - period, data.end(), 0.0) / period; // SMA inicial

    for (auto it = data.end() - period + 1; it != data.end(); ++it) 
    {
        ema = ((*it - ema) * multiplier) + ema;
    }

    return ema;
}