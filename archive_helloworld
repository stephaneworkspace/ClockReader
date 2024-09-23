#include <Bela.h>
#include <stdio.h>

// Seuil de détection
float threshold = 0.5;
// Pin de la LED sur Bela Mini (par exemple D0)
int ledPin = 0;

bool setup(BelaContext *context, void *userData)
{
    pinMode(context, 0, ledPin, OUTPUT);  // Configurer le pin de la LED en sortie
    return true;
}

void render(BelaContext *context, void *userData)
{
    for(unsigned int n = 0; n < context->audioFrames; n++) {
        // Lecture du signal sur l'entrée analogique A0
        float analogValue = analogRead(context, n/2, 0);  // A0 est l'index 0
        
        // Si le signal dépasse le seuil, allume la LED
        if (analogValue > threshold) {
            digitalWrite(context, n, ledPin, HIGH);  // Allume la LED
        } else {
            digitalWrite(context, n, ledPin, LOW);   // Éteint la LED
        }
    }
}

void cleanup(BelaContext *context, void *userData)
{
    // Rien à nettoyer
}