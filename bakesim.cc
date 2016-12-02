////////////////////////////////////////////////////////////////////////////
// Model model2.cc                SIMLIB/C++
//
// Simple test model of queuing system 
//

#include "simlib.h"
#include <iostream>
#include <ctime>

#define MINIMALNE_MIESANIE          60*4
#define MAXIMALNE_MIESANIE          60*6
#define PRIPRAVA_MIXU_MIN           60*2
#define PRIPRAVA_MIXU_MAX           60*3
#define OD_MIXERU_K_MIXERU          60*2
#define MIN_OD_MIXERU_K_MIXERU      60*2
#define MINIMALNE_VYBERANIE_CESTA   60*4
#define MAXIMALNE_VYBERANIE_CESTA   60*6
#define POCET_CHLEBOV_Z_CESTA_M     88
#define POCET_CHLEBOV_Z_CESTA_H     91
#define SIMULACIA                   60*60*12
#define DOBA_DELENIA                4
#define DOBA_ROLOVANIA              8
#define MINIMALNE_PREDKYSNUTIE      60*10
#define MAXIMALNE_PREDKYSNUTIE      60*12
#define MINIMALNE_KYSNUTIE          60*20
#define MAXIMALNE_KYSNUTIE          60*25
#define DOBA_PECENIA                60*35
#define DOBA_CHLADENIE              60*60*3

Store  Mixer("Mixer", 4);
Store  Rolovacka("Rolovacka", 3);
Store  PredKysnutie("PredKysnutie", 300);
Store  Kysnutie("Kysnutie", 450);
Store  Osatka("Osatka", 1000);
Store  Pec("Pec", 600);
Store  Chladenie("Chladenie", 1500);

Histogram ChliebCas("Doba pecenia chleba", 3800, 60, 10);
Histogram CestoCas("Doba pripravy cesta", 600, 60, 10);

int dobre_chleby = 0;
int zle_chleby = 0;

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

        if (Random() <= 0.01)
            zle_chleby++;
        else
            dobre_chleby++;

    }
};

/*
 * Deli cesto na jednotlive bochniky.
 */
class Delicka : public Event {
    //Maximalna volna kapacita
    int free_capacity = 1250;
    //Prave bochnikov vo zasobniku cesta
    int in = 0;
    //Prebieha delenie
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
        // Ziskaj volnu kapacitu v zasobniku
        int Free_capacity(){
            return free_capacity;
        }

        /* Vlozenie cesta do delicky
         */
        void Insert_new(int n){
            free_capacity -= n;
            in += n;
            //Ak delicka nepracovala, zapni ju
            if (!working)
                Activate();
        }
};

/* Pracovnik obslushujuci mixery*/
Facility Miesac("Chlap co miesa");

/* Pocet ciest, ktore chceme vymiesat*/
int maximum_ciest = 30;

Delicka *d = new Delicka();//TODO cleanup

/*
 * Miesanie cesta.
 * Potrebuje mixer, pracovnika
 */
class NoveMiesanie : public Process {
    void Behavior() {
        int my_id = maximum_ciest;
        Enter(Mixer);//Zober volny mixer
        Seize(Miesac);//Zaber miesaca
        maximum_ciest--;
        double zaciatok = Time;
        Wait(Uniform(PRIPRAVA_MIXU_MIN, PRIPRAVA_MIXU_MAX));//Naloz suroviny a zapni mixer
        Release(Miesac);//Pocas miesania nie je pracovnik potrebny
        if (maximum_ciest > 0){
            //Ak este treba, pracovnik prechadza k dalsiemu mixeru
            (new NoveMiesanie)->Activate(Time+MIN_OD_MIXERU_K_MIXERU + Exponential(OD_MIXERU_K_MIXERU));
        }
        Wait(Uniform(MINIMALNE_MIESANIE, MAXIMALNE_MIESANIE));//Vymiesanie cesta
        //Prednostne vyberie cesto pred pripravou noveho
        Seize(Miesac, HIGHEST_PRIORITY);
        Wait(Uniform(MINIMALNE_VYBERANIE_CESTA, MAXIMALNE_VYBERANIE_CESTA));//Vylozenie cesta
        int vymiesane = Uniform(POCET_CHLEBOV_Z_CESTA_M, POCET_CHLEBOV_Z_CESTA_H);
        // Pokial nie je dost miesta, cakaj
        while ( d->Free_capacity() < vymiesane){
            Wait(15);
        }
        //Vylozenie do delicky
        d->Insert_new(vymiesane);
        Leave(Mixer);
        Release(Miesac);
        CestoCas(Time - zaciatok);
        std::cout<<"Cesto no. "<<my_id<<" time "<<Time-zaciatok<<std::endl;
    }
};

int main() {
    RandomSeed(time(NULL));
    SetOutput("output.out");
    Init(0,SIMULACIA);
    (new NoveMiesanie)->Activate();
    Run();
    std::cout<<"Spravne upecenych chlebov: "<<dobre_chleby<<std::endl;
    std::cout<<"Chleby nevyhovujuce:       "<<zle_chleby<<std::endl;
    Mixer.Output();
    Rolovacka.Output();
    PredKysnutie.Output();
    Kysnutie.Output();
    Osatka.Output();
    Pec.Output();
    Chladenie.Output();
    ChliebCas.Output();
    CestoCas.Output();
    return 0;
}
