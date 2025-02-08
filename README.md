# Gestionnaire de Stock Médical

Une application de bureau développée en C avec GTK pour la gestion de stock de médicaments et de fournisseurs pharmaceutiques.

## Fonctionnalités

- Gestion complète des médicaments (ajout, modification, suppression)
- Gestion des fournisseurs pharmaceutiques
- Recherche simple et avancée
- Exportation des données en CSV
- Génération automatique de données avec IA
- Interface graphique intuitive
- Base de données SQLite

## Technologies Utilisées

- C
- GTK3
- SQLite3
- libcurl
- json-c
- API Gemini (Google AI)

## Installation

### Prérequis
```bash
# Ubuntu/Debian
sudo apt-get install libgtk-3-dev
sudo apt-get install libsqlite3-dev
sudo apt-get install libcurl4-openssl-dev
sudo apt-get install libjson-c-dev

# macOS
brew install gtk+3
brew install sqlite3
brew install curl
brew install json-c