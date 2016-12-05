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
#include <unistd.h>
#include <cmath>

Facility Pekar("Chlap co miesa");

Store  Dize("Dize", 10);
Store  HnetaciStroj("Hnetaci Stroj", 3);
Store  Vyvalovacka("Vyvalovaci stroj", 4);
Store  Kysnutie("Kysnutie", 730);
Store  Osatka("Osatka", 1000);
Store  Pec("Pec", 730);

Histogram ChliebCas("Doba pecenia chleba", 5000, 60, 15);
Histogram CestoCas("Doba pripravy cesta", 1250, 60, 15);

int dobre_chleby = 0;
int zle_chleby = 0;
int all_mixed = 0;

/* Pocet ciest, ktore chceme vymiesat*/
int maximum_ciest = 30;

Delicka *d = new Delicka();//TODO cleanup

void Chlieb::Behavior(){
    Prichod = Time;

    Enter(Vyvalovacka);
    Wait(DOBA_VYVALOVANIA);
    Leave(Vyvalovacka);

    Enter(Osatka);

    Enter(Kysnutie);
    Wait(Uniform(MINIMALNE_KYSNUTIE, MAXIMALNE_KYSNUTIE));
    Leave(Kysnutie);

    Leave(Osatka);

    Enter(Pec);
    Wait(DOBA_PECENIA);
    Leave(Pec);


    ChliebCas(Time-Prichod);

    if (Random() <= 0.01)
        zle_chleby++;
    else
        dobre_chleby++;

    if (all_mixed && !d->In() && Dize.Empty() && Kysnutie.Empty() && Pec.Empty()){
        std::cout<<"Vsetky chleby upecene v case: "<<Time<<"s. Koncime simulaciu."<<std::endl;
        Stop();
    }

}

void Delicka::Behavior() {
    if (in){
        double in_time = Time;
        working = 1;
        in--;
        free_capacity++;
        double delenie = Uniform(DOBA_DELENIA_MIN, DOBA_DELENIA_MAX);
        working_time += delenie;
        Activate(in_time+delenie);
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
    Enter(Dize);
    Enter(HnetaciStroj);//Zober volny mixer
    Seize(Pekar);//Zaber miesaca
    int my_id = maximum_ciest--;
    double zaciatok = Time;
    Wait(Uniform(PRIPRAVA_MIXU_MIN, PRIPRAVA_MIXU_MAX));//Naloz suroviny a zapni mixer
    Release(Pekar);//Pocas miesania nie je pracovnik potrebny
    if (maximum_ciest > 0){
        //Ak este treba, pracovnik prechadza k dalsiemu mixeru
        (new NoveMiesanie)->Activate(Time+MIN_OD_MIXERU_K_MIXERU + Exponential(OD_MIXERU_K_MIXERU));
    }
    Wait(Uniform(MINIMALNE_MIESANIE, MAXIMALNE_MIESANIE));//Vymiesanie cesta
    //Prednostne vyberie cesto pred pripravou noveho
    Priority = my_id;
    Seize(Pekar);
    Wait(Uniform(MINIMALNE_VYBERANIE_CESTA, MAXIMALNE_VYBERANIE_CESTA));//Domiesanie a presun Dize vedla hnetac
    Leave(HnetaciStroj);
    Release(Pekar);
    Wait(Uniform(MINIMALNE_PREDKYSNUTIE, MAXIMALNE_PREDKYSNUTIE));
    int vymiesane = Uniform(POCET_CHLEBOV_Z_CESTA_M, POCET_CHLEBOV_Z_CESTA_H);
    // Pokial nie je dost miesta, cakaj
    while ( d->Free_capacity() < vymiesane){
        Wait(15);
    }
    //Vylozenie do delicky
    d->Insert_new(vymiesane);
    Leave(Dize);
    CestoCas(Time - zaciatok);
    if (my_id == 1)
        all_mixed = 1;
}

void vytiskni_napovedu(){
    printf("help here!\n");
}

struct Argumenty {
    int cas;
    int pocet_chlebu;
    const char* vystup;
};

Argumenty zpracuj_argumenty(int argc, char* argv[]){
    int c;
    Argumenty argumenty;
    argumenty.cas = 60 * 60 * 12;
    argumenty.pocet_chlebu = 1500;
    argumenty.vystup = "bakesim.out";

    while((c = getopt(argc, argv, "t:n:o:h")) != -1) {
        switch (c) {
            case 't':
                argumenty.cas = 60 * atoi(optarg);
                break;
            case 'n':
                argumenty.pocet_chlebu = atoi(optarg);
                break;
            case 'o':
                argumenty.vystup = optarg;
                break;
            case 'h':
                vytiskni_napovedu();
                exit(0);
            default:
                fprintf(stderr, "%s\n", "Neznamy argument!");
                exit(1);
        }
    }

    return argumenty;
}

int main(int argc, char *argv[]) {
    Argumenty argumenty = zpracuj_argumenty(argc, argv);

    maximum_ciest = ceil(((float)argumenty.pocet_chlebu) / POCET_CHLEBOV_Z_CESTA_M);

    RandomSeed(time(NULL));
    SetOutput(argumenty.vystup);
    Init(0, argumenty.cas);
    (new NoveMiesanie)->Activate();
    Run();
    std::cout<<"Spravne upecenych chlebov: "<<dobre_chleby<<std::endl;
    std::cout<<"Chleby nevyhovujuce:       "<<zle_chleby<<std::endl;
    Pekar.Output();
    HnetaciStroj.Output();
    Dize.Output();
    d->Output();
    Vyvalovacka.Output();
    Kysnutie.Output();
    Osatka.Output();
    Pec.Output();
    ChliebCas.Output();
    CestoCas.Output();
    return 0;
}
