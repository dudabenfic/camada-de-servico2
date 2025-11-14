from fastapi import FastAPI, Request
from influxdb_client import InfluxDBClient, Point, WritePrecision
import os
from datetime import datetime

app = FastAPI()

url = os.getenv("INFLUXDB_URL", "http://influxdb:8086")
token = os.getenv("INFLUXDB_TOKEN", "DKkcVOJlopsZ_xLccqhj2wYpf-IhuVMx2AMeFupymE1XySZlP9Nz0F2zRvX0zpjiVsf5AiXGUeuZSauNV2HC3w==")
org = os.getenv("INFLUXDB_ORG", "meu_org")
bucket = os.getenv("INFLUXDB_BUCKET", "meu_bucket")

client = InfluxDBClient(url=url, token=token, org=org)
write_api = client.write_api()

@app.post("/dados")
async def receber_dados(data: dict):
    try:
        ts = datetime.fromtimestamp(data["timestamp"] / 1000)

        point = (
            Point("sensores")
            .tag("origem", "esp32")
            .field("nivel", float(data["nivel"]))
            .field("temperatura", float(data["temperatura"]))
            .field("umidade", float(data["umidade"]))
            .field("luminosidade", int(data["luminosidade"]))
            .field("presenca", int(data["presenca"]))
            .time(datetime.utcnow(), write_precision=WritePrecision.MS) 
        )

        write_api.write(bucket=bucket, org=org, record=point)
        return {"status": "success", "dados": data}
    except Exception as e:
        return {"status": "error", "detalhe": str(e)}

