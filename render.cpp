// render.cpp

#include <Bela.h>
#include <stdio.h>
#include <vector>
#include "const.h"

const int SOURCE = 0; 
// 0 Ableton Live          - PPQN 24 
// 1 Arturia Mini Brute 2S - Clock  

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

bool setup(BelaContext *context, void *userData)
{
    pinMode(context, 0, ledPin, OUTPUT);  // Configurer la pin de la LED en sortie
    pinMode(context, 0, outputPin, OUTPUT);
    switch (SOURCE) {
    	case 0:
    		// Ableton Live - 24PPQN
    		pulsesPerBeat = ABLETON_LIVE_24PPQN;
    		break;
    	default:
    		// Arturia Mini Brute 2S
    		pulsesPerBeat = ARTURIA_MINI_BRUTE_2S;
    		break;
    }
    // Ajuster la taille du tableau dynamique en fonction de pulsesPerBeat
    pulseDurations.resize(pulsesPerBeat);
    return true;
}

int cycleCounter = 0;
float bpmSum = 0;
int numCyclesToAverage = 4;
float bpmAdjustmentFactor = 120.0 / 125.26;  // Ajustement basé sur l'observation

void render(BelaContext *context, void *userData)
{
    // Obtenir le temps en millisecondes à partir des échantillons écoulés
    unsigned long currentTime = context->audioFramesElapsed / (context->audioSampleRate / 1000);

    for(unsigned int n = 0; n < context->audioFrames; n++) {
        float analogValue = analogRead(context, n/2, 0);

        if (analogValue > threshold && !previousState) {
            pulseCounter++;
            pulseDurations[pulseCounter - 1] = currentTime;

            if (pulseCounter >= pulsesPerBeat) {
                pulseCounter = 0;

                unsigned long totalDuration = pulseDurations[pulsesPerBeat - 1] - pulseDurations[0];
                float averagePulseDuration = totalDuration / (float)pulsesPerBeat;

                // Calcul du BPM avec la correction
                //float bpm = (60.0 * 1000.0) / (averagePulseDuration * pulsesPerBeat);
				float bpm = (60.0 * 1000.0) / (averagePulseDuration * pulsesPerBeat) * bpmAdjustmentFactor;
                bpmSum += bpm;
                cycleCounter++;

                if (cycleCounter >= numCyclesToAverage) {
                    float averageBPM = bpmSum / (float)numCyclesToAverage;
                    rt_printf("Average BPM: %.2f\n", averageBPM);

                    cycleCounter = 0;
                    bpmSum = 0;
                }
            }
            
            // Générer un signal 24 fois plus lent sur le mini-jack
            outputPulseCounter++;
            if (outputPulseCounter >= 24 && !signalActive) {
                // Passer la sortie à HIGH
                digitalWrite(context, n, outputPin, HIGH);
                signalStartTime = currentTime;  // Enregistrer l'heure d'activation du signal
                signalActive = true;  // Le signal est actif
                outputPulseCounter = 0;  // Réinitialiser le compteur
            }
        }

        previousState = (analogValue > threshold);

        if (ledState == 1 && currentTime - ledLastChangeTime > ledOnDuration) {
            digitalWrite(context, n, ledPin, LOW);
            ledState = 0;
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