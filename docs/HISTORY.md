# Historique du Projet NavOne

Ce fichier sert de mémoire pour le développement du projet. Il permet de reprendre le contexte rapidement lors d'une nouvelle session.

## Session: 20 Décembre 2025

### Objectifs
- Modulariser l'application principale (`NavOneApp.cpp`) qui devenait trop volumineuse.
- Implémenter un Dashboard affichant les données de navigation et l'état des services.

### Réalisations Techniques

#### 1. Refactoring Architectural
- **Découpage de `NavOneApp`** : L'application a été divisée en plusieurs composants indépendants.
- **Création de `src/gui/windows/`** :
  - `NmeaMonitorWindow` : Gère l'affichage des logs NMEA bruts et l'auto-scroll.
  - `DashboardWindow` : Affiche les instruments (Cap, Vitesse, Vent) et le tableau des services actifs.
  - `CommunicationSettingsWindow` : Gère la configuration des ports Série et UDP.
- **Création de `src/app/services/`** :
  - `ServiceManager` : Centralise la gestion des sources de données (démarrage, arrêt, configuration).

#### 2. Gestion des Données
- **Configuration XML** : Migration vers `TinyXML2` pour la sauvegarde de la configuration (`config.xml`).
- **Interface `IService`** : Abstraction pour gérer uniformément les sources UDP et Série.
- **MessageBus** : Utilisation d'un bus de messages pour découpler la réception des données de l'affichage.

#### 3. Fonctionnalités Ajoutées
- **Simulateur** : Intégré dans `NavOneApp` (génère des trames NMEA fictives).
- **Monitoring** : Visualisation en temps réel de l'état des connexions (Running/Error/Disabled) dans le Dashboard.

### État Actuel
- Le projet compile avec CMake.
- L'architecture est modulaire et prête pour l'ajout de nouvelles fonctionnalités.
- Les fichiers sources sont organisés par responsabilité (`gui`, `app`, `network`, `core`).

### Structure des Fichiers Clés
```text
src/
├── app/
│   ├── services/
│   │   ├── ServiceManager.hpp/.cpp  # Gestionnaire des sources (Serial/UDP)
│   ├── NavOneApp.cpp                # Point d'entrée de l'application (Orchestrateur)
│   └── DataSourceConfig.hpp         # Structures de configuration partagées
├── gui/
│   ├── windows/
│   │   ├── DashboardWindow.cpp      # Affichage principal + État des services
│   │   ├── NmeaMonitorWindow.cpp    # Logs NMEA défilants
│   │   └── CommunicationSettingsWindow.cpp # Configuraion des ports
├── network/
│   ├── IService.hpp                 # Interface polymorphique pour les sources
│   ├── SerialService.cpp            # Implémentation ASIO Série
│   └── UdpService.cpp               # Implémentation ASIO UDP
└── utils/
    └── ConfigManager.cpp            # Persistance XML (TinyXML2)
```

### Instructions de Compilation
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Prochaines Étapes (Suggestions)
- Ajouter de nouveaux widgets graphiques (Jauges circulaires).
- Implémenter le décodage de nouvelles phrases NMEA (MWV, RMC, etc.).
- Ajouter un système de log sur fichier (en plus de l'écran).

---
*Pour reprendre le développement, demandez à l'IA de "Lire le fichier docs/HISTORY.md pour récupérer le contexte".*
