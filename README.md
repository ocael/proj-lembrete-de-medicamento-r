# 💊 Sistema de Lembrete de Medicamentos

Este repositório implementa um **sistema de lembrete de medicamentos**, utilizando um ESP32, display OLED, potenciômetro, buzzer e LEDs. O horário do alerta é configurado pelo potenciômetro, e os dados são publicados via MQTT.

---

## 📷 Protótipo

> *Simulação feita no [Wokwi](https://wokwi.com/).*

![Protótipo do circuito](imagens/prototipo.png)

---

## 🔧 Componentes Utilizados

* **Placa:** ESP32 DevKit C v4 (simulado no Wokwi)
* **Display:** OLED monocromático I2C 128x64
* **Entrada:** Potenciômetro (configuração do horário do alerta)
* **Atuadores:**
  * 1× LED verde (sinal de energia)
  * 1× LED vermelho (alerta)
  * 1× Buzzer piezoelétrico
* **Comunicação:** Wi-Fi + MQTT via `test.mosquitto.org:1883` + NTP para sincronização de horário

---

## ⚙️ Como Funciona

1. **Configuração do Horário**
   * O potenciômetro permite ajustar o horário do alerta de 00:00 a 23:59.

2. **Sincronização de Horário**
   * O ESP32 se conecta a um servidor NTP (`pool.ntp.org`) para obter o horário atual (GMT-3).

3. **Monitoramento**
   * O sistema compara continuamente o horário atual com o horário configurado pelo potenciômetro.

4. **Alerta**
   * Quando o horário atual coincide com o horário configurado:
     * O **LED vermelho** acende
     * O **buzzer emite som intermitente**
     * O **display mostra mensagem de alerta**
     * Uma mensagem é publicada no MQTT (`medicamento/alerta`)

5. **Exibição local**
   * O display OLED mostra:
     * Hora atual
     * Próximo horário do alerta
     * Estado do sistema (`Aguardando` ou `Alertando`)

6. **MQTT**
   * Publicação em tópicos:
     * `medicamento/horario` → Horário configurado no formato HH:MM
     * `medicamento/alerta` → Mensagem de alerta
   * Broker: `test.mosquitto.org` porta `1883` via TCP/IP
   * Cliente MQTT: PubSubClient para ESP32

---

## 📁 Estrutura de Arquivos

```plaintext
├── sketch.ino       # Código principal do projeto
├── diagram.json     # Diagrama do circuito no Wokwi
└── libraries.txt    # Bibliotecas necessárias
```

---

## 🚀 Simulação no Wokwi

1. Acesse [https://wokwi.com](https://wokwi.com)
2. Crie um novo projeto e faça upload de:
   * `sketch.ino`
   * `diagram.json`
   * `libraries.txt`
3. Clique em **Start Simulation**
4. Abra o **Serial Monitor** para observar os dados
5. Observe o **display OLED** e os LEDs para status de energia e alerta

---

## Interfaces e Protocolos

Este projeto utiliza comunicação via protocolo **MQTT** e sincronização de horário via **NTP**:

* **Broker MQTT:** `test.mosquitto.org`
* **Porta:** `1883`
* **Transporte:** TCP/IP
* **Client Library:** PubSubClient para ESP32
* **Servidor NTP:** `pool.ntp.org` (GMT-3)

### Publicações (ESP32 → Broker)

| Tópico                 | Descrição                                |
| ---------------------- | ---------------------------------------- |
| `medicamento/horario`  | Horário configurado no formato HH:MM     |
| `medicamento/alerta`   | Mensagem de alerta quando disparado      |

---

## 🔄 Possíveis Extensões

* Configuração remota do horário via MQTT
* Múltiplos horários de alertas programáveis
* Histórico de alertas disparados
* Botão de confirmação de tomada do medicamento
* Integração com dashboard Node-RED para visualização

---

## 📜 Licença

Este projeto está licenciado sob a **MIT License**. Veja o arquivo `LICENSE` para mais detalhes.
