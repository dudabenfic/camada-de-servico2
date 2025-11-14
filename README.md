# camada-de-servico2
# Camada de Serviço 2  
Projeto de integração: ESP32 --> ESPNOW --> ESP32 --> Backend (FastAPI) --> InfluxDB 
Repositorio: https://github.com/dudabenfic/camada-de-servico2

---

## 1. Como montar o projeto  
### Pré-requisitos  
- Docker e Docker Compose instalados  
- Python 3.10+  
- Arduino IDE ou PlatformIO para programar o ESP32  
- Acesso à rede WiFi local para os ESP32s  

### Hardware / ESP32  
1. Conecte o ESP32 ao computador via USB.  
2. Instale sensores conforme o esquema:  
   - DHT11 no pino definido no código (`DHTPIN`)  
   - Sensor LDR no pino analógico definido  
   - Sensor PIR no pino definido  
   - (E qualquer outro sensor que você usar)  
3. No código do emissor ESP32, configure o SSID e senha da rede WiFi e o endereço do backend (endpoint) no receptor se for necessário.  
4. No receptor ESP32, conecte-o à mesma rede WiFi, e ele receberá os pacotes via ESP-NOW, e então enviará para o backend (HTTP POST).  
5. Compile e faça o upload do código para os ESP32s.
