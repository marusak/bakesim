/*
 * Header file for bakesim - Bakery simulation
 * bakesim.hh
 *
 * Define classes and constants.
 *
 * Authors: Matej Marusak   xmarus06@stud.fit.vutbr.cz
 *          Ondrej Budai    xbudai00@stud.fit.vutbr.cz
 *
 * 2.12.2016
 * Created as school project at VUT FIT Brno
 */

#include "simlib.h"

#define MINIMALNE_MIESANIE          60*4
#define MAXIMALNE_MIESANIE          60*6
#define PRIPRAVA_MIXU_MIN           60*2
#define PRIPRAVA_MIXU_MAX           60*3
#define OD_MIXERU_K_MIXERU          60*2
#define MIN_OD_MIXERU_K_MIXERU      60*1
#define MINIMALNE_VYBERANIE_CESTA   60*2
#define MAXIMALNE_VYBERANIE_CESTA   60*3
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

/*
 * Deli cesto na jednotlive bochniky.
 */
class Delicka : public Event {
    private:
        //Maximalna volna kapacita
        int free_capacity = 300;
        //Prave bochnikov v zasobniku cesta
        int in = 0;
        //Prebieha delenie
        int working = 0;
        //Maximalny pocet bochnikov v zasobniku
        int max_used = 0;
        //Pocet vkladani ciest
        int insert_requests = 0;
        //Pocet vlozenych bochnikov
        int insert_volume = 0;
        //Doba aktivity
        double working_time = 0.0;

        void Behavior();

    public:
        int Free_capacity();
        int In();
        void Insert_new(int n);
        void Output();
};


/*
 * Bochnik po vystupe z delicky az kym nie je upeceny chlieb.
 */
class Chlieb : public Process {
    double Prichod;
    void Behavior();
};


/*
 * Miesanie cesta.
 */
class NoveMiesanie : public Process {
    void Behavior();
};
