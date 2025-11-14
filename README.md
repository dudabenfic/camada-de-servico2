camada-de-servico2
## Camada de Serviço 2

Projeto de integração: ESP32 → ESP-NOW → ESP32 → Backend (FastAPI) → InfluxDB
Repositório: https://github.com/dudabenfic/camada-de-servico2

## 1. Como montar o projeto
Pré-requisitos:
- Docker e Docker Compose instalados
- Python 3.10+ (opcional, caso queira rodar FastAPI sem Docker)
- Arduino IDE ou PlatformIO para programar os ESP32
- Rede WiFi local para o ESP32 gateway
- 2 ESP32 (sensor + gateway)

## 2. Hardware / ESP32
ESP32 Sensor (emissor via ESP-NOW)

Este ESP32 coleta dados dos sensores e envia via ESP-NOW.
Sensores utilizados:

- DHT11 — temperatura e umidade
- LDR — luminosidade
- PIR — presença
- HC-SR04 — nível do tanque
- LEDs indicadores

Passos para configuração:

- Conecte cada sensor aos pinos definidos no código:
PIN_DHT, PIN_LDR, PIN_PIR, PIN_TRIG, PIN_ECHO

Atualize o MAC Address do ESP32 gateway:
uint8_t mac_address_monitoramento[] = { ... };

Faça upload do código usando Arduino IDE ou PlatformIO.

# ESP32 Gateway (receptor via ESP-NOW)

Este ESP32:

Recebe dados do sensor via ESP-NOW

Exibe valores no display LED

Liga e desliga LEDs de status

Envia os dados ao backend via HTTP POST

--- Atenção: o ESP32 gateway DEVE enviar para o IPv4 local da máquina que está executando o FastAPI. --- 

Exemplo de URL compatível com ESP32:

http://192.168.0.10:8000/dados

Configuração no código:
const char* SERVER_URL = "http://SEU_IPV4_LOCAL:8000/dados";

Para descobrir o IP:

Windows: ipconfig
Linux/Mac: ifconfig

## 3. Como rodar o backend

O backend é formado por:

- FastAPI
- InfluxDB 2.0

Ambos sobem automaticamente via Docker.

Passo 1 — Iniciar os containers

No diretório do projeto:
docker-compose up -d

Isso inicializará:

-- Container influxdb (porta 8086) -- 

Configurações automáticas (do docker-compose):

User: admin
Password: 12345678
Org: meu_org
Bucket: meu_bucket

-- Container fastapi (porta 8000) -- 

Endpoint principal:
POST /dados

## 4. Como acessar o InfluxDB

Acesse no navegador:

http://localhost:8086

- Faça login com:
Usuário: admin
Senha: 12345678

- Vá para Data Explorer:
Bucket: meu_bucket
Measurement: sensores
Campos disponíveis:

{nivel,
temperatura,
umidade,
luminosidade,
presenca,
timestamp}

## 5. Como testar o backend manualmente (sem ESP)

Você pode enviar dados falsos para testar se o backend e o Influx estão funcionando.

Exemplo com curl:
curl -X POST http://192.168.0.10:8000/dados \
  -H "Content-Type: application/json" \
  -d '{"nivel":80,"temperatura":22.5,"umidade":60,"luminosidade":300,"presenca":1,"timestamp":1700000000000}'

Resposta esperada:
{
  "status": "success",
  "dados": {
    "nivel": 80,
    "temperatura": 22.5,
    "umidade": 60,
    "luminosidade": 300,
    "presenca": 1,
    "timestamp": 1700000000000
  }
}

## 6. Como consultar dados no InfluxDB

- Acesse o painel em http://localhost:8086
- Abra Data Explorer
- Escolha:
   {Bucket: meu_bucket
   Measurement: sensores}
- Selecione os campos que deseja visualizar
- Veja a saída em tabela ou gráfico
