#!/bin/bash

# --- Netzwerk- und Systempfade ---
SERVER_BIND="127.0.0.1"
HOST="127.0.0.1"
PORT="8080"
MODEL_ID="Qwen35-9B-Q4_K_M"
LLAMA_SERVER="/home/tato/Dokumente/KiHelper-Mini/llama.cpp-source/llama.cpp/llama.cpp-master/build_withCuda/bin/llama-server"
INI_PATH="/home/tato/GitHub/KiHelperMiniDir/testFiles/Qwen_Qwen3.5-9B-Q4_K_M/preset.ini"
LOG_PATH="/home/tato/GitHub/KiHelperMiniDir/testFiles/Qwen_Qwen3.5-9B-Q4_K_M/server.log"

# 1. Radikale Prüfung und Beendigung ALLER Instanzen (Haupt- und Kind-Prozesse)
if pgrep -x "llama-server" > /dev/null; then
    echo "⚠️ Es laufen noch aktive oder verwaiste llama-server Prozesse im System."
    read -p "Möchtest du ALLE laufenden Server als Root beenden und neu starten? [j/y/N]: " antwort
    
    antwort=$(echo "$antwort" | tr '[:upper:]' '[:lower:]')
    
    if [[ "$antwort" == "j" || "$antwort" == "y" ]]; then
        echo "🛑 Beende ALLE Instanzen radikal via sudo killall..."
        sudo killall -9 llama-server
        echo "⏳ Warte 3 Sekunden, damit der VRAM der RTX 4060 sauber geleert wird..."
        sleep 3 
    else
        echo "❌ Abgebrochen. Bestehende Prozesse wurden nicht verändert."
        exit 0
    fi
fi

echo "🚀 Starte llama-server im Router-Modus mit integriertem Hardware-Tuning..."
echo "Server:   $LLAMA_SERVER"
echo "IP/Host:  $HOST"
echo "PORT:     $PORT"
echo "Ini:      $INI_PATH"
echo "Log:      $LOG_PATH"
echo

# 2. Server starten
taskset -c 0-11 "$LLAMA_SERVER" \
  --host "$SERVER_BIND" \
  --port "$PORT" \
  --models-preset "$INI_PATH" \
  --jinja \
  --models-max 1 \
  --parallel 1 \
  > "$LOG_PATH" 2>&1 &

# 3. Warten, bis das Modell im Router aktiv registriert ist
echo "⏳ Warte auf Server-Initialisierung und Modell-Registrierung..."
MODEL_REGISTERED=0
for i in {1..15}; do
    if curl -s "http://${HOST}:${PORT}/v1/models" | grep -Eq "\"id\"[[:space:]]*:[[:space:]]*\"${MODEL_ID}\""; then
        echo "📡 Router hat die INI-Datei erfolgreich unter http://${HOST}:${PORT} eingelesen!"
        MODEL_REGISTERED=1
        break
    fi
    sleep 1
done

if [[ "$MODEL_REGISTERED" -ne 1 ]]; then
    echo "❌ Modell ${MODEL_ID} wurde nicht registriert. Prüfe ${LOG_PATH}."
    exit 1
fi

echo "🧠 Erzwinge sofortiges Laden des Modells (Pre-Loading / Warm-up)..."

# 4. Das Buildin Modell via Netzwerk-IP in den VRAM laden
echo " -> Lade Buildin modell..."
if ! curl --fail --silent --show-error -o /dev/null -X POST "http://${HOST}:${PORT}/v1/chat/completions" \
  -H "Content-Type: application/json" \
    -d "{\"model\": \"${MODEL_ID}\", \"messages\": [{\"role\": \"user\", \"content\": \"warmup\"}], \"max_tokens\": 1}"; then
        echo "❌ Modell-Warm-up fehlgeschlagen. Prüfe ${LOG_PATH}."
        exit 1
fi

echo "✅ Modell ist vollständig geladen und mit maximaler Performance einsatzbereit!"
