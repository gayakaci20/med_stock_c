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
```

### Configuration

1. Cloner le repository
2. Copier `src/config.c.example` vers `src/config.c`
3. Ajouter votre clé API Gemini dans `src/config.c`
4. Compiler avec `make`

## Utilisation

Lancer l'application :
```bash
./medical_stock_manager
```

## Licence

MIT License
```

This README includes:
1. Project description
2. Main features
3. Technologies used
4. Installation instructions
5. Configuration steps
6. Usage information
7. License information

Would you like me to create the config.c.example file as well?
