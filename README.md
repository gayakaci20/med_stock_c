# 🏥 Gestionnaire de Stock Médical  

Une application de bureau développée en **C** avec **GTK** pour la gestion de stock de médicaments et de fournisseurs pharmaceutiques.  

## ✨ Fonctionnalités  

✅ **Gestion des médicaments** (ajout, modification, suppression)  
✅ **Gestion des fournisseurs pharmaceutiques**  
✅ **Recherche** simple et avancée 🔍  
✅ **Exportation des données** en **CSV** 📁  
✅ **Génération automatique de données avec IA** 🤖  
✅ **Interface graphique intuitive** 🎨  
✅ **Base de données SQLite** 🗄️  

## 🔧 Technologies Utilisées  

🚀 **Langage** : C  
🖥️ **Interface** : GTK3  
📦 **Base de données** : SQLite3  
🌐 **Réseau** : libcurl  
📜 **JSON** : json-c  
🧠 **IA** : API Gemini (Google AI)  

## ⚙️ Installation  

### 📌 Prérequis  

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

### 🔧 Configuration  

1️⃣ **Cloner le repository** 🛠️  
2️⃣ **Copier** `src/config.c.example` **vers** `src/config.c` 📂  
3️⃣ **Ajouter votre clé API Gemini** dans `src/config.c` 🔑  
4️⃣ **Compiler avec** `make` ⚡  

## 🚀 Utilisation  

Lancer l'application :  
```bash
./medical_stock_manager
```
