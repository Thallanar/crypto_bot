# CryptoBot

CryptoBot é um bot de código aberto para a realizações de transações com a API da Binance. Uma forma rápida e simples de realizar Day Trading!

# Como compilar:

-> Como muitas bibliotecas do C++ não estão instaladas por padrão, em um sistema Linux, sugiro que leiam todas as dependências que estão sendo exigidas no `makefile` do projeto.

-> Depois de Resolvidas todas as dependências de compilação, use o seguinte comando:

```sh
make
./crypto_bot
```

**Obs.:** Caso a compilação não seja bem sucedida, sugiro que faça uma compilação limpa:

```sh
make clean && make
```

-> Após compilado, existem algumas situações que precisam ser citadas:

1- [INFO]: O bot depende do arquivo `.env`, onde deverão está contidas sua `API_KEY` e `SECRET_PASSWORD`. Sem elas o bot aparecerá um erro e fechará;

2- [BUG]: Possivelmente um bug ocorre quando a aplicação `websocket` perde contato com a API da Binance. O bot para de realiazar transações e crasha, porém continua rodando sem informação alguma. Para que possa voltar ao funcionamento, o processo deverá ser intorrempido (`ctrl+c`). Basta rodar o bot mais uma vez e tudo será normalizado;

3- [BUG]: Também existe um bug onde o terminal não encerra a tarefa do bot quando a tecla `q` é acionada. Caso ocorra, basta interromper com o comando `ctrl+c`.

**PS.:** Caso encontrem algum bug ou tenha alguma sugestão de alterações, fique a vontade de enviar e-mail para `thallanargomes@gmail.com`.