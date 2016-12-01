////////////////////////////////////////////////////////////////////////////
// Model model2.cc                SIMLIB/C++
//
// Simple test model of queuing system 
//

#include "simlib.h"
#include <iostream>
#include <ctime>

#define MINIMALNE_MIESANIE      60*20
#define MAXIMALNE_MIESANIE      60*25
#define DOBA_VYBERANIA_CESTA    60*5
#define POCET_CHLEBOV_Z_CESTA_M 88
#define POCET_CHLEBOV_Z_CESTA_H 91
#define NOVE_MIESANIE           60*20
#define SIMULACIA               60*60*12
#define DOBA_DELENIA            4
#define DOBA_ROLOVANIA          8
#define MINIMALNE_PREDKYSNUTIE  60*10
#define MAXIMALNE_PREDKYSNUTIE  60*12
#define MINIMALNE_KYSNUTIE      60*20
#define MAXIMALNE_KYSNUTIE      60*25
#define DOBA_PECENIA            60*35
#define DOBA_CHLADENIE          60*60*3

//Facility  Delicka("Delicka");
//-n pocet chlebov za noc
//-t cas z aktory upiect
//Miesat za zaklade ineho miesania a nie po random dobe
//Statistika delicky
//Obcas sa chlieb pokazi + statistika pokazenych chelbov
//Statistika za aky cas dopecene
//

Store  Mixer("Mixer", 5);
Store  Rolovacka("Rolovacka", 4);
Store  PredKysnutie("PredKysnutie", 300);
Store  Kysnutie("Kysnutie", 450);
Store  Osatka("Osatka", 1000);
Store  Pec("Pec", 400);
Store  Chladenie("Chladenie", 1500);

Histogram ChliebCas("Doba pecenia chleba", 4000, 60, 10);

class Chlieb : public Process {
    double Prichod;

    void Behavior(){
        Prichod = Time;

        Enter(Rolovacka);
        Wait(DOBA_ROLOVANIA);//EXP?
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
            (new Chlieb)->Activate();
            Activate(in_time+DOBA_DELENIA);
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

class Cesto : public Process {
    double Prichod;
    Delicka *d = new Delicka();//cleanup

    void Behavior(){
        Prichod = Time;
        Enter(Mixer);//Zober volny mixer
        Wait(Uniform(MINIMALNE_MIESANIE, MAXIMALNE_MIESANIE));//Naloz suroviny a mixuj
        Wait(Exponential(DOBA_VYBERANIA_CESTA));//Vyloz do delicky
        d->Insert_new(Uniform(POCET_CHLEBOV_Z_CESTA_M, POCET_CHLEBOV_Z_CESTA_H));
        Leave(Mixer);
    }
};

class Generator : public Event {
    int maximum_ciest = 18;
    void Behavior() {
        (new Cesto)->Activate();
        if (--maximum_ciest)
            Activate(Time+Exponential(NOVE_MIESANIE));
    }
};

int main() {
    RandomSeed(time(NULL));
    SetOutput("output.out");
    Init(0,SIMULACIA);
    (new Generator)->Activate();
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
