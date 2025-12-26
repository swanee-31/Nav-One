# Historique du Projet NavOne

Ce fichier sert de mémoire pour le développement du projet. Il permet de reprendre le contexte rapidement lors d'une nouvelle session.

## Session: 26 Décembre 2025

### Objectifs
- Mettre en place le système de build et de packaging pour la distribution.
- Générer un installeur Windows pour l'utilisateur final.

### Réalisations Techniques
- **Configuration CPack** : Ajout des directives CPack dans `CMakeLists.txt` pour supporter NSIS (Windows Installer) et ZIP.
- **Build Release** : Compilation réussie du projet en mode Release.
- **Packaging** : Génération des artefacts de distribution :
  - `NavOne-0.1.0-win64.exe` (Installeur)
  - `NavOne-0.1.0-win64.zip` (Portable)

### État Actuel
- Version 0.1.0 tagguée.
- Le pipeline de build produit des exécutables fonctionnels et installables.

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

## Session: 20 Décembre 2025 (Suite)

### Objectifs
- Implémenter la rediffusion (multiplexage) des données reçues vers des sorties (Série/UDP).
- Permettre le filtrage des sources pour chaque sortie.
- Ajouter une configuration de l'apparence (Thème, Taille de police).

### Réalisations Techniques

#### 1. Multiplexage et Sorties
- **Architecture Output** : Extension de `ServiceManager` pour gérer des `DataOutputConfig` en plus des sources.
- **Bidirectionnel** : Mise à jour de `SerialService` pour supporter l'écriture (`send()`).
- **UdpSender** : Création d'une classe dédiée à l'envoi de paquets UDP.
- **Routage** : Implémentation de la logique de diffusion dans `ServiceManager::broadcast`.
  - Support du mode "Tout envoyer" (`multiplexAll`).
  - Support du filtrage par ID de source (`sourceIds`).

#### 2. Interface Graphique
- **CommunicationSettingsWindow** :
  - Ajout d'onglets "Inputs" et "Outputs".
  - Interface de sélection des sources à multiplexer (Checkboxes dynamiques).
- **DisplaySettingsWindow** :
  - Nouvelle fenêtre pour configurer le thème (Dark/Light/Classic) et l'échelle de la police.
  - Intégration dans le menu principal.

#### 3. Configuration et Persistance
- Mise à jour de `DataSourceConfig.hpp` pour inclure les structures `DataOutputConfig` et `DisplayConfig`.
- Mise à jour de `ConfigManager` pour sauvegarder/charger :
  - Les sorties et leurs règles de multiplexage.
  - Les préférences d'affichage.

#### 4. Corrections
- **Linker Error** : Implémentation manquante des méthodes d'écriture dans `SerialService.cpp`.
- **Crash au démarrage** : Déplacement de l'initialisation de la configuration d'affichage (`applyConfig`) après la création du contexte ImGui.

### État Actuel
- L'application peut agir comme un multiplexeur NMEA (N entrées -> M sorties).
- L'utilisateur peut personnaliser l'apparence.
- La configuration est entièrement persistante.

---
*Pour reprendre le développement, demandez à l'IA de "Lire le fichier docs/HISTORY.md pour récupérer le contexte".*

## Session: 21 Décembre 2025

### Objectifs
- Refactoring complet du module Simulator (Pattern Decorator).
- Ajout de simulations environnementales (Eau, AIS).
- Création d'un système de plugins pour la visualisation.
- Packaging de l'application.

### Réalisations Techniques

#### 1. Refactoring du Simulateur (Pattern Decorator)
- **Architecture** : Découpage en `ISimulator`, `BaseSimulator` et `SimulatorDecorator`.
- **Modules** :
  - `GpsSimulator` : Génère GPRMC.
  - `WindSimulator` : Génère IIMWV.
  - `WaterSimulator` : Génère IIDBS, IIDPT, IIMTW, IIHDT, IIVHW (avec oscillation sinusoïdale).
  - `AisSimulator` : Génère des trames binaires AIVDM pour 3 cibles (Zigomar, Yamato, Titanic).
- **Contrôle** : Ajout de contrôles de fréquence (ms) pour chaque module dans `SimulatorWindow`.

#### 2. Système de Plugins
- **Architecture** : Chargement dynamique de DLLs via `PluginManager`.
- **Nouveaux Plugins** :
  - `WaterPlugin` : Visualisation graphique de l'historique de profondeur (`ImGui::PlotLines`) et données numériques.
  - `WindPlugin` : Jauge de vent (AWA/TWA).
  - `GpsPlugin` : Affichage des coordonnées.

#### 3. Améliorations Core
- **NmeaParser** : Ajout du support pour DPT, MTW, VHW, HDT.
- **NavOneApp** : Propagation des données environnementales (Température eau, Vitesse surface).
- **About Window** : Affichage de la version Git (via CMake).

#### 4. Packaging et Déploiement
- **CMake/CPack** : Configuration pour générer un installateur NSIS (.exe) et une archive ZIP.
- **Installation** : Règles `install()` pour copier l'exécutable et les plugins dans `bin/`.

#### 5. Corrections de Bugs
- **Crash ImGui** : Correction de l'appel à `ImGui::End()` manquant lors de la réduction des fenêtres (`SimulatorWindow`, Plugins, `CommunicationSettings`).
- **Compilation** : Correction des exports DLL (`createPlugin` vs `CreatePlugin`).
- **Logique** : Correction de la structure `AisTargetConfig` (champ `callsign` manquant).

### État Actuel
- L'application est packagée et distribuable.
- Le simulateur est complet (GPS, Vent, Eau, AIS).
- Les plugins permettent une visualisation modulaire.
- La stabilité a été renforcée (fix crashs UI).

---

