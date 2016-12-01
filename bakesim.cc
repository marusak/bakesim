////////////////////////////////////////////////////////////////////////////
// Model model2.cc                SIMLIB/C++
//
// Simple test model of queuing system 
//

#include "simlib.h"
#include <iostream>
#include <ctime>

#define MINIMALNE_MIESANIE      60*30
#define MAXIMALNE_MIESANIE      60*35
#define PRIPRAVA_MIXU           60*3
#define OD_MIXERU_K_MIXERU      60*2
#define DOBA_VYBERANIA_CESTA    60*5
#define POCET_CHLEBOV_Z_CESTA_M 88
#define POCET_CHLEBOV_Z_CESTA_H 91
#define SIMULACIA               60*60*12
#define DOBA_DELENIA            4
#define DOBA_ROLOVANIA          8
#define MINIMALNE_PREDKYSNUTIE  60*10
#define MAXIMALNE_PREDKYSNUTIE  60*12
#define MINIMALNE_KYSNUTIE      60*20
#define MAXIMALNE_KYSNUTIE      60*25
#define DOBA_PECENIA            60*35
#define DOBA_CHLADENIE          60*60*3

//-n pocet chlebov za noc
//-t cas z aktory upiect
//Statistika delicky
//Obcas sa chlieb pokazi + statistika pokazenych chelbov
//Statistika za aky cas dopecene
//

Store  Mixer("Mixer", 4);
Store  Rolovacka("Rolovacka", 3);
Store  PredKysnutie("PredKysnutie", 300);
Store  Kysnutie("Kysnutie", 450);
Store  Osatka("Osatka", 1000);
Store  Pec("Pec", 600);
Store  Chladenie("Chladenie", 1500);

Histogram ChliebCas("Doba pecenia chleba", 3800, 60, 10);

class Chlieb : public Process {
    double Prichod;

    void Behavior(){
        Prichod = Time;

        Enter(Rolovacka);
        Wait(DOBA_ROLOVANIA);
        Leave(Rolovacka);

        Enter(PredKysnutie);
        Wait(Uniform(MINIMALNE_PREDKYSNUTIE, MAXIMALNE_PREDKYSNUTIE));
        Leave(PredKysnutie);

        Enter(Osatka);

        Enter(Kysnutie);
        Wait(Uniform(MINIMALNE_KYSNUTIE, MAXIMALNE_KYSNUTIE));
        Leave(Kysnutie);

        Enter(Pec);
        Wait(DOBA_PECENIA);
        Leave(Pec);

        Leave(Osatka);

        ChliebCas(Time-Prichod);
        Enter(Chladenie);
        Wait(Exponential(DOBA_CHLADENIE));
        Leave(Chladenie);

    }
};


class Delicka : public Event {
    int free_capacity = 250;
    int in = 0;
    int working = 0;
    void Behavior() {
        if (in){
            double in_time = Time;
            working = 1;
            in--;
            free_capacity++;
            Activate(in_time+DOBA_DELENIA);
            (new Chlieb)->Activate();
        }
        else
            working = 0;
    }
    public:
        int Free_capacity(){
            return free_capacity;
        }

        void Insert_new(int n){
            free_capacity -= n;
            in += n;
            if (!working)
                Activate();
        }
};

Facility Miesac("Chlap co miesa");

int maximum_ciest = 18;
Delicka *d = new Delicka();//cleanup

class NoveMiesanie : public Process {
    void Behavior() {
        Enter(Mixer);//Zober volny mixer
        Seize(Miesac);//Zaber miesaca
        maximum_ciest--;
        Wait(Exponential(PRIPRAVA_MIXU));//Naloz suroviny a zapni mixer
        Release(Miesac);
        if (maximum_ciest > 0){
            (new NoveMiesanie)->Activate(Time+Exponential(OD_MIXERU_K_MIXERU));
        }
        Wait(Exponential(DOBA_VYBERANIA_CESTA));//Vyloz do delicky
        Seize(Miesac, HIGHEST_PRIORITY);
        d->Insert_new(Uniform(POCET_CHLEBOV_Z_CESTA_M, POCET_CHLEBOV_Z_CESTA_H));
        Leave(Mixer);
        Release(Miesac);
    }
};

int main() {
    RandomSeed(time(NULL));
    SetOutput("output.out");
    Init(0,SIMULACIA);
    (new NoveMiesanie)->Activate();
    Run();
    Mixer.Output();
    //Delicka.Output();
    Rolovacka.Output();
    PredKysnutie.Output();
    Kysnutie.Output();
    Osatka.Output();
    Pec.Output();
    Chladenie.Output();
    ChliebCas.Output();
    return 0;
}
