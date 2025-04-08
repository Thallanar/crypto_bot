# Nome do executável final
TARGET = crypto_bot

# Diretórios
SRC_DIR = src
HDR_DIR = header
OBJ_DIR = build

# Compilador e flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -I./header -I/usr/include/jsoncpp -I/usr/local/include/cpr -I/usr/local/include/asio
LDFLAGS = -L/usr/local/lib -lcpr -ljsoncpp -lssl -lcrypto -lcurl -lpthread -lboost_system -lboost_thread

# Busca recursivamente todos os .cpp no src
SRCS = $(shell find $(SRC_DIR) -name '*.cpp')
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Regra padrão
all: $(TARGET)

# Linkagem final
$(TARGET): $(OBJS)
	@echo "Linkando $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compilação de cada .cpp em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpeza dos objetos e executável
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Executar
run: all
	./$(TARGET)
