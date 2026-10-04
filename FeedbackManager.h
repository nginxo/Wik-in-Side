#pragma once

#include <Arduino.h>
#include "config.h"

class FeedbackManager {
public:
    FeedbackManager();

    void init();
    void setAudioEnabled(bool enabled);
    void setVibroEnabled(bool enabled);
    bool isAudioEnabled() const;
    bool isVibroEnabled() const;

    // Esegue una singola nota con curva di attacco/rilascio e vibrazione combinata (da audiovibration.c)
    void playNote(int freq, int duration = NOTE_DURATION_MS, int attack = ATTACK_MS, int release = RELEASE_MS);

    // Suona l'intera sequenza di notifica monumento (da audiovibration.c)
    void playMonumentSequence();

    // Effetti sonori e tattili per la UI dell'OS
    void playBootJingle();
    void playClick();
    void playSuccess();
    void playWarning();
    void stopAll();

private:
    bool _audioEnabled;
    bool _vibroEnabled;
    bool _initialized;
};

extern FeedbackManager Feedback;
