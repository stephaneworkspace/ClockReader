// render.cpp

#include <Bela.h>
#include <stdio.h>
#include <vector>
#include "const.h"

const int SOURCE = 0; 
// 0 Ableton Live          - PPQN 96 
// 1 Arturia Mini Brute 2S - Clock  
// 2 ABLETON_VCV_RACK_16TH - VCV 16th

float threshold = THRESHOLD;  // Ajuster le seuil pour capter correctement le signal
int ledPin = LED_PIN;
const int outputPin = 1;  // Utilise la pin D1 pour la sortie jack
int pulseCounter = 0;  // Compteur pour diviser la clock
int outputPulseCounter = 0;  // Compteur pour la sortie divisée
int pulsesPerBeat = 24;  // Nombre d'impulsions par battement
bool previousState = false;  // Suivre l'état précédent pour détecter le front montant
unsigned int ledState = 0;  // Etat de la LED (allumée ou éteinte)
unsigned long ledLastChangeTime = 0;  // Dernière fois que la LED a changé d'état
unsigned long ledOnDuration = 100;  // Durée pendant laquelle la LED reste allumée (en ms)
unsigned long signalStartTime = 0;  // Enregistrer le temps où le signal a été mis à HIGH
bool signalActive = false;  // Suivre l'état du signal
// Déclare un vecteur dynamique pour stocker les impulsions
std::vector<float> pulseDurations;

float averageBPM = 120.0;  // Valeur initiale du BPM
float smoothingFactor = 0.1;  // Facteur de lissage pour le filtre passe-bas
unsigned long lastPhraseTime = 0;  // Temps du dernier début de phrase

bool setup(BelaContext *context, void *userData)
{
    pinMode(context, 0, ledPin, OUTPUT);  // Configurer la pin de la LED en sortie
    pinMode(context, 0, outputPin, OUTPUT);
    switch (SOURCE) {
    	case 0:
    		// Ableton Live - 96PPQN
    		pulsesPerBeat = ABLETON_LIVE_96PPQN;
    		ledOnDuration *= 4;
    		break;
    	case 2:
    		pulsesPerBeat = ABLETON_VCV_RACK_16TH;
    	default:
    		// Arturia Mini Brute 2S
    		pulsesPerBeat = ARTURIA_MINI_BRUTE_2S;
    		break;
    }
    // Ajuster la taille du tableau dynamique en fonction de pulsesPerBeat
    pulseDurations.resize(pulsesPerBeat);
    return true;
}

// Fonction pour filtrer la durée moyenne à l'aide d'un filtre passe-bas exponentiel
float lowPassFilter(float currentValue, float previousValue, float alpha) {
    return alpha * currentValue + (1.0 - alpha) * previousValue;
}

void render(BelaContext *context, void *userData)
{
    unsigned long currentTime = context->audioFramesElapsed / (context->audioSampleRate / 1000);

    for(unsigned int n = 0; n < context->audioFrames; n++) {
        float analogValue = analogRead(context, n/2, 0);

        // Détection des impulsions normales
        if (analogValue > threshold && !previousState) {
            pulseCounter++;
            pulseDurations[pulseCounter - 1] = currentTime;

            if (pulseCounter >= pulsesPerBeat) {
                pulseCounter = 0;
                
                                // Allumer la LED pour indiquer un battement réel
                ledState = 1;
                ledLastChangeTime = currentTime;  // Mémoriser l'heure d'allumage de la LED
                digitalWrite(context, n, ledPin, HIGH);

                // Calculer la durée totale des impulsions normales uniquement
                unsigned long totalDuration = pulseDurations[pulsesPerBeat - 1] - pulseDurations[0];
                float averagePulseDuration = totalDuration / (float)pulsesPerBeat;

                // Calcul du BPM avec la correction
                float bpm = (60.0 * 1000.0) / (averagePulseDuration * pulsesPerBeat);
                
                // Appliquer un filtre passe-bas pour lisser les variations du BPM
                averageBPM = lowPassFilter(bpm, averageBPM, smoothingFactor);

                rt_printf("Average BPM: %.2f\n", averageBPM);
            }
            
            // Générer un signal 24 fois plus lent sur le mini-jack
            outputPulseCounter++;
            if (outputPulseCounter >= pulsesPerBeat) {
                digitalWrite(context, n, outputPin, HIGH);
                signalStartTime = currentTime;
                signalActive = true;
                outputPulseCounter = 0;
            }
        }

        previousState = (analogValue > threshold);
        
        unsigned int durationInSamples = DURATION_RELEASE * (context->audioSampleRate / 1000);

		if (signalActive && (currentTime - signalStartTime >= durationInSamples)) {
		    digitalWrite(context, n, outputPin, LOW);
		    signalActive = false;
		}

        if (ledState == 1 && currentTime - ledLastChangeTime > ledOnDuration) {
            digitalWrite(context, n, ledPin, LOW);
            ledState = 0;
        }
    }
}

void cleanup(BelaContext *context, void *userData)
{
    // Rien à nettoyer
}