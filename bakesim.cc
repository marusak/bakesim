/*
 * Main source file for bakesim - Bakery simulation
 * bakesim.cc
 *
 * Implementation of all methods and main function.
 *
 * Authors: Matej Marusak   xmarus06@stud.fit.vutbr.cz
 *          Ondrej Budai    xbudai00@stud.fit.vutbr.cz
 *
 * 2.12.2016
 * Created as school project at VUT FIT Brno
 */

#include "simlib.h"
#include "bakesim.hh"
#include <iostream>
#include <ctime>

Facility Miesac("Chlap co miesa");

Store  Mixer("Mixer", 4);
Store  Rolovacka("Rolovacka", 3);
Store  PredKysnutie("PredKysnutie", 180);
Store  Kysnutie("Kysnutie", 360);
Store  Osatka("Osatka", 1000);
Store  Pec("Pec", 600);
Store  Chladenie("Chladenie", 2000);

Histogram ChliebCas("Doba pecenia chleba", 3800, 60, 10);
Histogram CestoCas("Doba pripravy cesta", 350, 60, 10);

int dobre_chleby = 0;
int zle_chleby = 0;
int all_mixed = 0;
int all_baked = 0;

/* Pocet ciest, ktore chceme vymiesat*/
int maximum_ciest = 30;

Delicka *d = new Delicka();//TODO cleanup

void Chlieb::Behavior(){
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

    if (Random() <= 0.01)
        zle_chleby++;
    else
        dobre_chleby++;

    if (all_mixed && !d->In() && PredKysnutie.Empty() && Kysnutie.Empty() && Pec.Empty()){
        std::cout<<"Vsetky chleby upecene v case: "<<Time<<"s"<<std::endl;
        all_baked = 1;
    }

    Enter(Chladenie);
    Wait(Exponential(DOBA_CHLADENIE));
    Leave(Chladenie);
    if (all_baked and Chladenie.Empty()){
        std::cout<<"Vsetky chleby vychladene. Koncim simulaciu v case: "<<Time<<"s"<<std::endl;
        Stop();
    }
}

void Delicka::Behavior() {
    if (in){
        double in_time = Time;
        working = 1;
        in--;
        free_capacity++;
        working_time += DOBA_DELENIA;
        Activate(in_time+DOBA_DELENIA);
        (new Chlieb)->Activate();
    }
    else
        working = 0;
}

int Delicka::Free_capacity(){
    return free_capacity;
}

int Delicka::In(){
    return in;
}

/* Vlozenie cesta do delicky
 */
void Delicka::Insert_new(int n){
    free_capacity -= n;
    in += n;
    insert_requests++;
    insert_volume += n;
    if (in > max_used)
        max_used = in;
    //Ak delicka nepracovala, zapni ju
    if (!working)
        Activate();
}

void Delicka::Output(){
      char s[100];
      Print("+----------------------------------------------------------+\n");
      Print("| FACILITY with storage %-34s |\n","Delicka");
      Print("+----------------------------------------------------------+\n");
      sprintf(s," Status = %s ", (working) ? "BUSY" : "not BUSY");
      Print("| %-56s |\n",s);
      sprintf(s," Time interval = %g - %g ",0.0, (double)Time);
      Print(  "| %-56s |\n", s);
      Print(  "|  Capacity of storage = %-27ld       |\n", free_capacity + in);
      Print(  "|  Maximal items in storage = %-22ld       |\n", max_used);
      Print(  "|  Number of input requests = %-22ld       |\n", insert_requests);
      Print(  "|  Average input volume = %-26g       |\n", (double)insert_volume/insert_requests);
      Print(  "|  Average utilization = %-27g       |\n", working_time/Time);
      Print("+----------------------------------------------------------+\n\n");
}


void NoveMiesanie::Behavior() {
    Enter(Mixer);//Zober volny mixer
    Seize(Miesac);//Zaber miesaca
    int my_id = maximum_ciest--;
    double zaciatok = Time;
    Wait(Uniform(PRIPRAVA_MIXU_MIN, PRIPRAVA_MIXU_MAX));//Naloz suroviny a zapni mixer
    Release(Miesac);//Pocas miesania nie je pracovnik potrebny
    if (maximum_ciest > 0){
        //Ak este treba, pracovnik prechadza k dalsiemu mixeru
        (new NoveMiesanie)->Activate(Time+MIN_OD_MIXERU_K_MIXERU + Exponential(OD_MIXERU_K_MIXERU));
    }
    Wait(Uniform(MINIMALNE_MIESANIE, MAXIMALNE_MIESANIE));//Vymiesanie cesta
    //Prednostne vyberie cesto pred pripravou noveho
    Priority = my_id;
    Seize(Miesac);
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
    if (my_id == 1)
        all_mixed = 1;
}

int main() {
    RandomSeed(time(NULL));
    SetOutput("output.out");
    Init(0,SIMULACIA);
    (new NoveMiesanie)->Activate();
    Run();
    std::cout<<"Spravne upecenych chlebov: "<<dobre_chleby<<std::endl;
    std::cout<<"Chleby nevyhovujuce:       "<<zle_chleby<<std::endl;
    Miesac.Output();
    Mixer.Output();
    d->Output();
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
