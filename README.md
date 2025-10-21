# camada-de-servico2
# Camada de Serviço 2  
Projeto de integração: ESP32 → Backend (FastAPI) → InfluxDB → Grafana  
Repositorio: https://github.com/dudabenfic/camada-de-servico2

---

## 1. Como montar o projeto  
### Pré-requisitos  
- Docker e Docker Compose instalados  
- Python 3.10+  
- Arduino IDE ou PlatformIO para programar o ESP32  
- (Opcional) Acesso à rede WiFi local para os ESP32s  

### Hardware / ESP32  
1. Conecte o ESP32 ao computador via USB.  
2. Instale sensores conforme o esquema:  
   - DHT11 no pino definido no código (`DHTPIN`)  
   - Sensor LDR no pino analógico definido  
   - Sensor PIR no pino definido  
   - (E qualquer outro sensor que você usare)  
3. No código do emissor ESP32, configure o SSID e senha da rede WiFi e o endereço do backend (endpoint) no receptor se for necessário.  
4. No receptor ESP32, conecte-o à mesma rede WiFi, e ele receberá os pacotes via ESP-NOW, e então enviará para o backend (HTTP POST).  
5. Compile e faça o upload do código para os ESP32s.

---

## 2. Como rodar o backend + banco de dados  
### 2.1 Subir os serviços com Docker  
Na raiz do repositório, supondo que contém o `docker-compose.yml`, execute:

```bash
docker-compose up -d
Isso iniciará:

InfluxDB (porta 8086)

Grafana (porta 3000)

2.2 Configurações iniciais
Acesse o painel do InfluxDB em https://bookish-garbanzo-v969qx4jvvw3694x-8086.app.github.dev/orgs/f5433c72c756eb55

Usuário: admin

Senha: 12345678

Organização (org): meu_org

Bucket: meu_bucket

Acesse o Grafana em https://bookish-garbanzo-v969qx4jvvw3694x-3000.app.github.dev/dashboards

Usuário: admin

Senha: 123456

2.3 Rodar o backend (FastAPI)
Entre na pasta do backend: c+

bash
Copy code
cd backend
python -m venv venv
source venv/bin/activate   # no Linux/macOS  
# venv\Scripts\activate     # no Windows
pip install -r requirements.txt
uvicorn main:app --host 0.0.0.0 --port 8000 --reload
O backend estará acessível em https://bookish-garbanzo-v969qx4jvvw3694x-8000.app.github.dev/
Você pode acessar a documentação interativa: http://localhost:8000/docs.

2.4 Testar API manualmente
Envie um exemplo via curl ou via Postman:

bash
Copy code
curl -X POST http://localhost:8000/dados \
  -H "Content-Type: application/json" \
  -d '{
        "nivel": 50,
        "temperatura": 25.3,
        "umidade": 45.7,
        "luminosidade": 1200,
        "presenca": 1,
        "timestamp": 1698000000000
      }'
Você deve receber algo como:

json
Copy code
{"status":"ok"}
Se sim — os dados foram gravados no bucket do InfluxDB.

3. Como consultar dados salvos
3.1 Usando InfluxDB direto
Acesse http://localhost:8086.

No menu lateral, vá em Data Explorer.

Selecione bucket: meu_bucket.

Crie uma query usando Flux, por exemplo:

flux
Copy code
from(bucket: "meu_bucket")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "leituras")
Você pode ver os resultados como tabela ou gráfico.

3.2 Usando Grafana
Acesse http://localhost:3000.

Adicione Data Source:

Tipo: InfluxDB

URL: http://influxdb:8086 (ou http://localhost:8086)

Token: meu_token_123

Org: meu_org

Bucket: meu_bucket

Crie um Dashboard → Add Panel.

Exemplo de query no painel:

flux
Copy code
from(bucket: "meu_bucket")
  |> range(start: -30m)
  |> filter(fn: (r) => r._measurement == "leituras")
  |> filter(fn: (r) => r._field == "temperatura")
Escolha o tipo de visualização (gráfico de linhas, gauge, etc.).

4. Endpoints disponíveis
POST /dados — Recebe os dados do ESP32 e grava no InfluxDB.

Outros endpoints podem ser definidos conforme necessidade (por exemplo GET /dados para consulta via API).

5. Tecnologias empregadas
ESP32 para aquisição de dados via sensores

Protocolos: WiFi, ESP-NOW

Backend em Python + FastAPI

Banco de dados de séries temporais: InfluxDB (versão 2.x)

Visualização: Grafana

Orquestração: Docker + Docker Compose

6. Autor
Desenvolvido por Duda Benfíc
