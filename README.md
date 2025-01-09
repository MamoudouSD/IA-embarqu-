# IA-embarqué - Système de Reconnaissance de Gestes en Temps Réel

Ce dépôt contient l'implémentation d'un **système de reconnaissance de gestes en temps réel** conçu pour des systèmes embarqués. Le système utilise un **Raspberry Pi 4 Model B** et une **caméra HD (Emeet C960)** pour capturer et classifier des gestes. Il s'appuie sur **l'analyse Rate Monotonic (RMA)** pour gérer les priorités des tâches et garantir une performance robuste en temps réel.

## Fonctionnalités
- Capture et classification d'images en temps réel.
- Architecture multi-thread pour le traitement parallèle des tâches.
- Planification des tâches basée sur les priorités via RMA.
- Synchronisation des tâches avec des sémaphores.
- Déploiement de modèles de machine learning légers en Python avec TensorFlow Lite.

---

## Structure des fichiers
- **`main.cpp`**  
  Implémentation principale en C++ qui gère :
  - Initialisation de la caméra et capture d'images.
  - Multi-threading avec priorisation des tâches.
  - Intégration avec un pipeline Python pour le machine learning.
  - Journalisation des événements système pour le débogage.

- **`python.py`**  
  Script Python dédié à :
  - La classification des images à l’aide d’un modèle TensorFlow Lite pré-entraîné.
  - Le prétraitement des données et la génération des résultats.

- **`Model_train.ipynb`**  
  Notebook Jupyter contenant :
  - Le pipeline d’entraînement du modèle de reconnaissance de gestes.
  - Techniques d’augmentation des données et optimisation du modèle.
  - Conversion du modèle entraîné en TensorFlow Lite pour le déploiement embarqué.

---

## Prérequis
### Matériel
- **Raspberry Pi 4 Model B**
- **Caméra HD (Emeet C960)**

### Logiciel
- **C++17 ou plus récent** (avec prise en charge d'OpenCV)
- **Python 3.10**
- Bibliothèques Python requises :
  - TensorFlow Lite
  - OpenCV
  - NumPy
- **Ubuntu 22.04** (ou distribution Linux compatible)

---

## Instructions d’exécution
### Compilation (C++)
1. Compilez le fichier `main.cpp` avec la commande suivante :
   ```bash
   g++ -std=c++20 -pthread -o main main.cpp `pkg-config --cflags --libs opencv4`
