#include "FeedbackManager.h"

FeedbackManager Feedback;

FeedbackManager::FeedbackManager()
    : _audioEnabled(true), _vibroEnabled(true), _initialized(false) {}

void FeedbackManager::init() {
    if (_initialized) return;

    // Configurazione PWM LEDC per Buzzer e Vibrazione (ESP32 Core 3.x API)
    ledcAttach(BUZZER_PIN, BUZZER_CHANNEL_FREQ, PWM_RESOLUTION);
    ledcAttach(VIBRO_PIN, VIBRO_CHANNEL_FREQ, PWM_RESOLUTION);

    // Azzeramento uscite
    ledcWrite(BUZZER_PIN, 0);
    ledcWrite(VIBRO_PIN, 0);
    ledcWriteTone(BUZZER_PIN, 0);

    _initialized = true;
}

void FeedbackManager::setAudioEnabled(bool enabled) {
    _audioEnabled = enabled;
    if (!enabled) {
        ledcWrite(BUZZER_PIN, 0);
        ledcWriteTone(BUZZER_PIN, 0);
    }
}

void FeedbackManager::setVibroEnabled(bool enabled) {
    _vibroEnabled = enabled;
    if (!enabled) {
        ledcWrite(VIBRO_PIN, 0);
    }
}

bool FeedbackManager::isAudioEnabled() const {
    return _audioEnabled;
}

bool FeedbackManager::isVibroEnabled() const {
    return _vibroEnabled;
}

void FeedbackManager::playNote(int freq, int duration, int attack, int release) {
    if (!_initialized) init();

    if (_audioEnabled && freq > 0) {
        ledcWriteTone(BUZZER_PIN, freq);
    }

    const int steps = 15;
    int attackStepDelay = attack / steps;
    if (attackStepDelay < 1) attackStepDelay = 1;

    // Rampa di salita (Attack): buzzer 0->255, vibrazione MIN->MAX per vincere l'inerzia
    for (int step = 0; step <= steps; step++) {
        int audioLevel = (255 * step) / steps;
        int vibroLevel = MIN_VIBRO_PWM + ((MAX_VIBRO_PWM - MIN_VIBRO_PWM) * step) / steps;

        if (_audioEnabled) ledcWrite(BUZZER_PIN, audioLevel);
        if (_vibroEnabled) ledcWrite(VIBRO_PIN, vibroLevel);
        delay(attackStepDelay);
    }

    // Sustain: picco mantenuto stabile
    int sustainTime = duration - attack - release;
    if (sustainTime > 0) {
        if (_vibroEnabled) ledcWrite(VIBRO_PIN, MAX_VIBRO_PWM);
        delay(sustainTime);
    }

    // Rampa di discesa (Release)
    int releaseStepDelay = release / steps;
    if (releaseStepDelay < 1) releaseStepDelay = 1;

    for (int step = steps; step >= 0; step--) {
        int audioLevel = (255 * step) / steps;
        int vibroLevel = MIN_VIBRO_PWM + ((MAX_VIBRO_PWM - MIN_VIBRO_PWM) * step) / steps;

        if (_audioEnabled) ledcWrite(BUZZER_PIN, audioLevel);
        if (_vibroEnabled) ledcWrite(VIBRO_PIN, vibroLevel);
        delay(releaseStepDelay);
    }

    // Spegnimento netto a fine ciclo
    ledcWrite(BUZZER_PIN, 0);
    ledcWrite(VIBRO_PIN, 0);
    ledcWriteTone(BUZZER_PIN, 0);
}

void FeedbackManager::playMonumentSequence() {
    for (int i = 0; i < NUM_MONUMENT_NOTES; i++) {
        playNote(MONUMENT_NOTES[i], NOTE_DURATION_MS, ATTACK_MS, RELEASE_MS);
        delay(40);
    }
    stopAll();
}

void FeedbackManager::playBootJingle() {
    int bootNotes[] = { 523, 659, 784, 1046 }; // Do5, Mi5, Sol5, Do6
    for (int i = 0; i < 4; i++) {
        playNote(bootNotes[i], 35, 10, 15);
        delay(20);
    }
    stopAll();
}

void FeedbackManager::playClick() {
    if (_audioEnabled) {
        ledcWriteTone(BUZZER_PIN, 1800);
        ledcWrite(BUZZER_PIN, 120);
    }
    if (_vibroEnabled) {
        ledcWrite(VIBRO_PIN, 180);
    }
    delay(12);
    stopAll();
}

void FeedbackManager::playSuccess() {
    playNote(1046, 60, 15, 20);
    delay(30);
    playNote(1318, 90, 15, 30);
    stopAll();
}

void FeedbackManager::playWarning() {
    playNote(440, 80, 20, 30);
    delay(40);
    playNote(330, 120, 20, 40);
    stopAll();
}

void FeedbackManager::stopAll() {
    ledcWrite(BUZZER_PIN, 0);
    ledcWrite(VIBRO_PIN, 0);
    ledcWriteTone(BUZZER_PIN, 0);
}
