I will share my experiences in the file, but there will be no source code changes in the current version.
I use current llama trunk with CUDA.

1. How do I start Codestral-22B-v0.1-Q5_K_M.gguf with llama-server locally?
   - modelCodestral.ini:
     [codestral-agent]
      model = /home/UserName/LocalKiModels/Codestral-22B-v0.1-Q5_K_M.gguf
      ngl = 12
      c = 16384
      ctx-size = 16384
      t = 6
      tb = 6
  - codestral.jinja:
    {% for message in messages %}{% if message['role'] == 'user' %}[INST] {{ message['content'] }} [/INST]{% elif message['role'] == 'system' %}[INST] {{ message['content'] }} [/INST]{% else %}{{ message['content'] }}{% endif %}{% endfor %}
  - runSrvCodestral.sh:
      \#!/bin/bash  
      \# ==========================================
      \# Hardware: i7-13620H & RTX 4060 (WindowMaker)
      \# ==========================================
      
      \# Pfade definieren
      LLAMA_SERVER="/home/UserName/KiHelperMini-alpha/llama.cpp-source/llama.cpp/llama.cpp-master/build_withCuda/bin/llama-server"
      INI_PATH="/home/UserName/KiHelperMini-alpha/LocalModelRunner/Codestral/modelCodestral.ini"
      LOG_PATH="/home/UserName/KiHelperMini-alpha/LocalModelRunner/Codestral/serverCodestral.log"
      TEMPLATE_PATH="/home/UserName/KiHelperMini-alpha/LocalModelRunner/Codestral/codestral.jinja"
      
      \# 1. Radikale Prüfung und Beendigung ALLER Instanzen (Haupt- und Kind-Prozesse)
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
      echo "Ini:      $INI_PATH"
      echo "Template: $TEMPLATE_PATH"
      echo "Log:      $LOG_PATH"
      echo
      
      \# 2. Server im Hintergrund starten (Optimiert mit Template-File-Zuweisung)
      \# nice -n -20 / ionice: Maximale System-Priorität
      \# --cpu-mask 0xFFF: Kettet die Last exklusiv an die P-Kerne 0-11
      \# --ubatch-size 1024: Verarbeitet größere Prompt-Blöcke gleichzeitig
      \# --chat-template-file: Lädt das Jinja-Template direkt aus der Datei
      sudo nice -n -20 ionice -c 1 -n 0 "$LLAMA_SERVER" \
        --host 169.254.170.194 \
        --port 8080 \
        --models-preset "$INI_PATH" \
        --models-max 1 \
        --parallel 1 \
        --cpu-mask 0xFFF \
        --ubatch-size 1024 \
        --chat-template-file "$TEMPLATE_PATH" \
        > "$LOG_PATH" 2>&1 &
      
      \# 3. Warten, bis das Modell im Router aktiv registriert ist
      echo "⏳ Warte auf Server-Initialisierung und Modell-Registrierung..."
      for i in {1..15}; do
          if curl -s http://localhost:8080/v1/models | grep -q '"id": "codestral-agent"'; then
              echo "📡 Router hat die INI-Datei erfolgreich eingelesen!"
              break
          fi
          sleep 1
      done
      
      echo "🧠 Erzwinge sofortiges Laden des Modells (Pre-Loading / Warm-up)..."
      
      \# 4. Das Codestral-Modell via localhost in den VRAM/RAM laden
      echo " -> Lade codestral-agent (Code, 22B)..."
      curl -s -o /dev/null -X POST http://localhost:8080/v1/chat/completions \
        -H "Content-Type: application/json" \
        -d '{"model": "codestral-agent", "messages": [{"role": "user", "content": "warmup"}], "max_tokens": 1}'
      
      echo "✅ Modell ist vollständig geladen und mit maximaler Performance einsatzbereit!"

    2. Very fast for me: Qwen2.5-Coder-7B-Instruct-Q6_K.gguf
      
