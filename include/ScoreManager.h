#ifndef SCOREMANAGER_H
#define SCOREMANAGER_H

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <SFML/System/Time.hpp>

class ScoreManager {
public:
    static const std::string SCORE_FILE;

    // Charger les scores depuis le fichier
    static std::vector<float> loadScores() {
        std::vector<float> scores;
        std::ifstream file(SCORE_FILE);
        float time;
        while (file >> time) {
            scores.push_back(time);
        }
        // Trier (plus petit temps = meilleur)
        std::sort(scores.begin(), scores.end());
        return scores;
    }

    // Sauvegarder un nouveau temps s'il est dans le top 5
    static void saveTime(float newTime) {
        std::vector<float> scores = loadScores();
        scores.push_back(newTime);
        std::sort(scores.begin(), scores.end());
        
        // Garder le top 5
        if (scores.size() > 5) {
            scores.resize(5);
        }

        std::ofstream file(SCORE_FILE);
        for (float time : scores) {
            file << time << "\n";
        }
    }
};

// Définition du chemin (à mettre dans un .cpp ou ici pour la simplicité inline)
inline const std::string ScoreManager::SCORE_FILE = "scores.dat";

#endif