# 🐜 Ant Bridge Simulation

Questo progetto simula il comportamento cooperativo di un gruppo di formiche in un ambiente 2D, ispirato a meccanismi di **swarm intelligence** e **comunicazione chimica e tattile**.

Le formiche sono suddivise in:
- **Droni**: esplorano l’ambiente alla ricerca del bersaglio e depositano feromoni.
- **Lavoratrici**: seguono il percorso segnato e formano "ponti viventi" per superare ostacoli.

## 🧠 Algoritmo

La logica si basa su principi di swarm intelligence, in cui semplici agenti locali (formiche) cooperano per risolvere un compito collettivo.
In questo caso l'obiettivo è di far passare più formiche lavoratrici possibili dal nido al bersaglio (cibo), minimizzando il numero di formiche che formano il ponte.

## ⚙️ Caratteristiche principali

- Simulazione su griglia di dimensione scalabile con terreno, buchi, ostacoli, nido e obiettivo.
- Rilevamento e conteggio dei buchi da parte dei droni per scegliere il percorso che ne contiene meno.
- Formiche lavoratrici che diventano statiche per formare ponti, per far passare le altre su di sè e superare i buchi.
- Gestione dello stato delle formiche: `MOVING`, `STILL`, `WARNING`.
- Uso di feromoni e mappa olfattiva per navigazione e ritorno.

## ▶️ Esecuzione

```bash
python ant_bridge.py
