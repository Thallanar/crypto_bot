#include "../../header/api/api.h"

API::API()
{
    load_env(".env");
}

std::map<std::string, std::string> API::load_env(const std::string& filename)
{
    std::ifstream file(filename);
    std::string line;
    std::map<std::string, std::string> env;

    if (!file.is_open()) 
    {
        throw std::runtime_error("Erro ao abrir o arquivo .env");
    }

    while (std::getline(file, line)) 
    {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream ss(line);
        std::string key, value;

        if (std::getline(ss, key, '=') && std::getline(ss, value)) 
        {
            if (key == "API_KEY") 
            {
                this->api_key = value;
            } else if (key == "SECRET_KEY") {
                this->secret_key = value;
            }
        }
    }

    return env;
}

std::string API::get_timestamp() 
{
    using namespace std::chrono;
    return std::to_string(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

std::string API::timestamp_to_string(time_t timestamp) 
{
    std::stringstream ss;
    std::tm *tm_info = std::localtime(&timestamp);
    ss << std::put_time(tm_info, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Função para inicializar SSL/TLS
websocketpp::lib::shared_ptr<boost::asio::ssl::context> API::on_tls_init(std::weak_ptr<void>) 
{
    auto ctx = websocketpp::lib::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::tlsv12);
    try 
    {
        ctx->set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2 |
                         boost::asio::ssl::context::no_sslv3 |
                         boost::asio::ssl::context::single_dh_use);
    } catch (std::exception &e) {
        std::cerr << "Erro ao inicializar TLS: " << e.what() << std::endl;
    }
    return ctx;
}


double API::extract_price(const std::string& response_text) 
{
    auto json_response = nlohmann::json::parse(response_text);
    if (!json_response["fills"].empty()) {
        return std::stod(json_response["fills"][0]["price"].get<std::string>());
    }
    return 0.0;
}

std::string API::hmac_sha256(const std::string &key, const std::string &data) 
{
    unsigned char* digest;
    digest = HMAC(EVP_sha256(), key.c_str(), key.length(), (unsigned char*)data.c_str(), data.length(), NULL, NULL);
    
    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return ss.str();
}

// Callback para mensagens WebSocket (atualizações de preço)
void API::on_message(websocketpp::connection_hdl hdl, client::message_ptr msg) 
{
    Json::Value jsonData;
    Json::CharReaderBuilder reader;
    std::string errs;
    std::istringstream s(msg->get_payload());

    if (Json::parseFromStream(reader, s, &jsonData, &errs)) 
    {
        double price = std::stod(jsonData["p"].asString());  // "p" é o preço na Binance

        // Adiciona o preço ao buffer e mantém o tamanho fixo
        price_buffer.push_back(price);
        if (price_buffer.size() > MAX_BUFFER_SIZE) 
        {
            price_buffer.pop_front();
        }
    }
}

// Função para conectar ao WebSocket da Binance
void API::connect_websocket(const std::string symbol) 
{
    client c;
    std::string lower_str = symbol;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), [](unsigned char c){ return std::tolower(c); });
    std::string uri = "wss://stream.binance.com:9443/ws/" + lower_str + "@trade";

    try 
    {
        c.init_asio();
        c.set_tls_init_handler([this](websocketpp::connection_hdl hdl){ return this->on_tls_init(hdl ); });
        c.set_message_handler([this](websocketpp::connection_hdl hdl, client::message_ptr msg){ return this->on_message(hdl, msg); });
        
        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        
        if (ec) 
        {
            std::cerr << "Erro ao conectar ao WebSocket: " << ec.message() << std::endl;
            return;
        }

        c.connect(con);
        c.run();  // Mantém a conexão ativa
    } catch (websocketpp::exception const &e) {
        std::cerr << "Exceção WebSocket: " << e.what() << std::endl;
    }
}

void API::run_websocket(const std::string symbol)
{
    std::thread websocket_thread([this, symbol](){ connect_websocket(symbol); });
    websocket_thread.detach();  // Deixa a thread rodando independentemente
}
        
cpr::Response API::send_signed_request(const std::string& endpoint, const std::string& params, const std::string& method) 
{
    std::string query = params + "&timestamp=" + get_timestamp();
    std::string signature = hmac_sha256(this->secret_key, query);

    query += "&signature=" + signature;
    
    std::string url = BASE_URL + endpoint + "?" + query;
    
    cpr::Header headers = { {"X-MBX-APIKEY", this->api_key} };
    if (method == "GET") 
    {
        return cpr::Get(cpr::Url{url}, headers);
    } else if (method == "POST") {
        return cpr::Post(cpr::Url{url}, headers);
    }

    return {};
}

// Função para buscar o preço atual da moeda escolhida via API REST
double API::fetch_price(const std::string symbol) 
{
    std::vector<Trade> trades;
    cpr::Response r = cpr::Get(cpr::Url{BASE_URL + "/ticker/price?symbol=" + symbol});
    
    if (r.status_code == 200) 
    {
        Json::Value jsonData;
        Json::CharReaderBuilder reader;
        std::string errs;
        std::istringstream s(r.text);
        
        if (Json::parseFromStream(reader, s, &jsonData, &errs)) 
        {
            return std::stod(jsonData["price"].asString());
            
        } else {
            std::cerr << "Erro ao analisar JSON: " << errs << std::endl;
            return -1.0;
        }
    } else {
        std::cerr << "Erro ao buscar dados da Binance! Código: " << r.status_code << std::endl;
        return -1.0;
    }
}

double API::get_balance(const std::string asset) 
{
    cpr::Response r = send_signed_request("/account", "", "GET");

    if (r.status_code != 200) 
    {
        std::cerr << "Erro ao buscar saldo: " << r.status_code << std::endl;
        return 0.0;
    }

    try 
    {
        auto json = nlohmann::json::parse(r.text);

        for (const auto& bal : json["balances"]) 
        {
            if (bal["asset"] == asset) 
            {
                return std::stod(bal["free"].get<std::string>());
            }
        }

        std::cerr << "Ativo " << asset << " não encontrado na conta." << std::endl;
    } 
    catch (const std::exception& e) 
    {
        std::cerr << "Erro ao analisar JSON de saldo: " << e.what() << std::endl;
    }

    return 0.0;
}
