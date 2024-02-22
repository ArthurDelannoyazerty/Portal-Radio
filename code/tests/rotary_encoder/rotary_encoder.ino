#define sw_pin  5
#define dt_pin  4
#define clk_pin 3

int compteur = 0;

int etatPrecedentLigneSW;
int etatPrecedentLigneCLK;

void setup() {
    Serial.begin(9600);

    // pinMode(sw_pin, INPUT);         // à remplacer par : 
    pinMode(sw_pin, INPUT_PULLUP);

    pinMode(dt_pin, INPUT);
    pinMode(clk_pin, INPUT);

    etatPrecedentLigneSW  = digitalRead(sw_pin);
    etatPrecedentLigneCLK = digitalRead(clk_pin);

    Serial.println(compteur);

    delay(200);
}

void loop() {
    int etatActuelDeLaLigneCLK = digitalRead(clk_pin);
    int etatActuelDeLaLigneSW  = digitalRead(sw_pin);
    int etatActuelDeLaLigneDT  = digitalRead(dt_pin);

    if(etatActuelDeLaLigneSW != etatPrecedentLigneSW) {
        if(etatActuelDeLaLigneSW == LOW)  Serial.println(F("Bouton SW appuyé"));
        else                              Serial.println(F("Bouton SW relâché"));
        delay(10);
    }
    if(etatActuelDeLaLigneCLK != etatPrecedentLigneCLK) {
      etatPrecedentLigneCLK = etatActuelDeLaLigneCLK;
      if(etatActuelDeLaLigneCLK == LOW) {
        if(etatActuelDeLaLigneCLK != etatActuelDeLaLigneDT) {
            compteur--;
            Serial.println(compteur);
        }
        else {
            compteur++;
            Serial.println(compteur);
        }
        delay(7);
      }
    }
}