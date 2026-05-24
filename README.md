# 🛰️ DeepCore OS - Firmware ESP32

Este repositório contém o código-fonte desenvolvido em C++/Arduino para o microcontrolador ESP32, responsável pelo monitoramento de temperatura e automação de resfriamento.

## 🚀 Funcionalidades
- **Leitura em Tempo Real:** Sensor DS18B20 para alta precisão.
- **Controle Dinâmico:** - Estágio 1 (28°C): Acionamento de Ventoinha 12V.
  - Estágio 2 (34°C): Acionamento de Bomba d'água 110V.
- **Telemetria:** Envio de dados via protocolo MQTT (Broker EMQX).
- **Interface Física:** Display OLED 0.96" para monitoramento local.

## 🛠️ Tecnologias
- C++ / Arduino IDE
- Bibliotecas: `PubSubClient`, `DallasTemperature`, `Adafruit_SSD1306`.
