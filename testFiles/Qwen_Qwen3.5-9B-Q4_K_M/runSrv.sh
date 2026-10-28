#!/bin/bash

# --- Netzwerk- und Systempfade ---
SERVER_BIND="0.0.0.0"    
HOST_LOCAL="127.0.0.1"
PORT="8080"
LLAMA_SERVER="/home/tato/Dokumente/KiHelper-Mini/llama.cpp-source/llama.cpp/llama.cpp-master/build_withCuda/bin/llama-server"
INI_PATH="/home/tato/Dokumente/KiHelper-Mini/Qwen3.5-9B-Q4_K_M/preset.ini"
LOG_PATH="/home/tato/Dokumente/KiHelper-Mini/Qwen3.5-9B-Q4_K_M/server.log"

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
echo "IP/Host:  $SERVER_BIND (Lauscht global)"
echo "PORT:     $PORT"
echo "Ini:      $INI_PATH"
echo "Log:      $LOG_PATH"
echo

# 2. Server starten
"$LLAMA_SERVER" \
  --host "$SERVER_BIND" \
  --port "$PORT" \
  --models-preset "$INI_PATH" \
  > "$LOG_PATH" 2>&1 &

SERVER_PID=$!

# 3. Warten, bis der HTTP-Server im Router-Modus antwortet
echo "⏳ Warte auf Server-Schnittstelle (Router-Modus)..."
SERVER_READY=0
for i in {1..15}; do
    # Ein simpler Ping auf die API reicht im On-Demand-Modus aus
    if curl -s -o /dev/null -w "%{http_code}" "http://${HOST_LOCAL}:${PORT}/v1/models" | grep -Eq "200|404|401"; then
        echo "📡 Router-Schnittstelle ist erfolgreich unter http://${SERVER_BIND}:${PORT} aktiv!"
        echo "🌐 Modelle werden bei der ersten Anfrage automatisch in den VRAM geladen."
        SERVER_READY=1
        break
    fi
    sleep 1
done

if [[ "$SERVER_READY" -eq 0 ]]; then
    echo "❌ Server konnte nicht rechtzeitig initialisiert werden."
    echo "Prüfe das Log unter: $LOG_PATH"
    kill "$SERVER_PID" 2>/dev/null
    exit 1
fi

echo "✅ Server läuft erfolgreich im Hintergrund (PID: $SERVER_PID)."
exit 0
