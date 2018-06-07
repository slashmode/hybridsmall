#include <omnetpp.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>

#include "../../include/omnetpp/ccanvas.h"
#include "../../include/omnetpp/ccomponent.h"
#include "../../include/omnetpp/cdisplaystring.h"
#include "../../include/omnetpp/checkandcast.h"
#include "../../include/omnetpp/clog.h"
#include "../../include/omnetpp/cnamedobject.h"
#include "../../include/omnetpp/cobjectfactory.h"
#include "../../include/omnetpp/cpar.h"
#include "../../include/omnetpp/csimplemodule.h"
#include "../../include/omnetpp/cstringtokenizer.h"
#include "../../include/omnetpp/cwatch.h"
#include "../../include/omnetpp/regmacros.h"

using namespace omnetpp;
#include "Customer_m.h"

class BikeStation : public cSimpleModule
{

private:

    double bikes;
    double vbikes;
    double cap;
    int numstations;
    int ind;
    int nrad;
    double deventctr;
    double beventctr;
    double nbeventctr;
    double ndeventctr;
    double sl;
    int isneighbor[5];
    int dist[5];
    int asdfar;

protected:

    virtual void refreshDisplay() const override;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(BikeStation);

void BikeStation::initialize()
{
    beventctr = 0;
    deventctr = 0;
    nbeventctr = 0;
    ndeventctr = 0;
    asdfar = 0;
    sl = 0;
    numstations = 0;
    bikes = par("bikes");
    vbikes = bikes;
    cap = par("cap");


    ind = par("ind");
    numstations = getParentModule()->par("numstations_network");
    nrad = getParentModule()->par("nrad");

    WATCH(beventctr);
    WATCH(deventctr);
    WATCH(nbeventctr);
    WATCH(ndeventctr);
    WATCH(sl);
    WATCH(numstations);
    WATCH(bikes);
    WATCH(cap);

    const char *vstr = par("distances");
    std::vector<int> disthold = cStringTokenizer(vstr).asIntVector();

    EV << "I am in initialize matrix" << endl;
    EV << "distance matrix of ind =  " << ind << " : ";
    for(int i = 0; i <= numstations-1; i++){ //Initialization of isneighbor vector
        EV <<  disthold[i] << " ";
        dist[i] = disthold[i];
        if(disthold[i] <= nrad){
            isneighbor[i] = 1;
        }
        else{
            isneighbor[i] = 0;
        }
    }
    EV << endl;

}

void BikeStation::handleMessage(cMessage *msg)
{

    int traveltss[numstations];

    const char *trts = par("traveltimes");
    std::vector<int> timess = cStringTokenizer(trts).asIntVector();

    for(int i = 0; i <= numstations-1; i++){

        cDelayChannel *channel = check_and_cast<cDelayChannel*>(gate("o_stn_to_stn",i)->getChannel());
        channel->setDelay(timess[i]);

    }

    //  remote submodule par access code:
//        asdfar = getParentModule()->getSubmodule("stn", 2)->par("bikes");

    if (strcmp(msg->getName(),"walker") == 0){/* if a customer wanting to ride arrives */

        Customer *wlkc = check_and_cast<Customer *>(msg);

        int a = wlkc->getOriginaldest();
         EV <<"ind: " << ind << endl;
         EV << a << endl;

            if (bikes > 0){ /*If there is a bike available*/

                    EV << "I AM HERE 1" << endl;

                    int cfactorholder = getParentModule()->par("cfactor");

                    int cscoreholder2 = wlkc->getCscore();

                    if(cscoreholder2>cfactorholder){/*If customer is not cooperative*/

                        EV << "I AM HERE 2" << endl;

                        EV <<"Bike event!" << endl;

                        bikes = par("bikes"); //Update bike parameter value for easy access from other submodules
                        bikes--;
                        par("bikes") = bikes;


                        Customer *bikc = check_and_cast<Customer *>(wlkc);
                        bikc->setName("biker");
                        bikc->setDisplayString("i=bss/biker");

                        int desthshsh = bikc->getOriginaldest();
                        send(bikc, "o_stn_to_stn", desthshsh); //Send customer to original destination

                        EV << "Sending message from " << ind << "to " << desthshsh << endl;

        //                freqsendoutMessage(bikc);

                        beventctr = getParentModule()->par("beventctr_network");
                        deventctr = getParentModule()->par("deventctr_network");
                        nbeventctr = getParentModule()->par("nbeventctr_network");
                        ndeventctr = getParentModule()->par("ndeventctr_network");
                        beventctr++;
                        getParentModule()->par("beventctr_network") = beventctr;


                        if(hasGUI()){
                            char label[50];
                            sprintf(label,"b = %.0f   d = %.0f\nnb = %.0f   nd = %.0f\nSL = %.4f%%", beventctr, deventctr, nbeventctr, ndeventctr, 100*(beventctr+deventctr)/(beventctr+deventctr+nbeventctr+ndeventctr));
                            cCanvas *canvas = getParentModule()->getCanvas();
                            cTextFigure *textFigure = check_and_cast<cTextFigure*>(canvas->getFigure("systemstats"));
                            textFigure->setText(label);
                        }
                    }


                    else{//if the customer is cooperative

                        EV << "I AM HERE 3" << endl;

                        //send to station nearest the original destination that is within radius and has a fill level under 50%
                        //destdist[i] = isneighborofdest[i]*isdestthresh[i]*dist[i]

                        bikes = par("bikes"); //Update bike parameter value for easy access from other submodules
                        bikes--;
                        par("bikes") = bikes;

                        Customer *bikc = check_and_cast<Customer *>(wlkc);
                        bikc->setName("biker");
                        bikc->setDisplayString("i=bss/biker");

                        int origdest = bikc->getOriginaldest();

                        const char *vstrrryr = getParentModule()->getSubmodule("stn", origdest)->par("distances");
                        std::vector<int> distfromorigdest = cStringTokenizer(vstrrryr).asIntVector();

                        int isneighborofdest[numstations] ;

                        EV << "isneighborofdest0 array: ";
                        for(int i = 0; i<= numstations-1; i++){

                            EV << isneighborofdest[i] << " ";

                        }
                        EV << endl;

                        for(int i = 0; i<=numstations-1; i++){

                                isneighborofdest[i] = 0;

                            }

                        EV << "isneighborofdest1 array: ";
                        for(int i = 0; i<= numstations-1; i++){

                            EV << isneighborofdest[i] << " ";

                        }
                        EV << endl;

                        for(int i = 1; i<=numstations-1; i++){
                            if(distfromorigdest[i]<=nrad){
                                isneighborofdest[i] = 1;
                            }

                            else{
                                isneighborofdest[i] = 0;

                            }

                        }

                        EV << "isneighborofdest2 array: ";
                        for(int i = 0; i<= numstations-1; i++){

                            EV << isneighborofdest[i] << " ";

                        }
                        EV << endl;

                        const char *vstrryr = par("distances");
                        std::vector<int> distt = cStringTokenizer(vstrrryr).asIntVector();

                        int isdestthresh[numstations];

                        for(int i = 0; i<=numstations-1; i++){

                            double bhold = par("bikes");
                            double chold = par("cap");
                            double upll = par("upll");

                            if ((bhold/chold)>=fmax(upll/chold,0.5)){
                                isdestthresh[i] = 1;
                            }
                            else{
                                isdestthresh[i] = 0;
                            }
                        }

                        int destdist[numstations];

                        int sd = 2000;
                        int si = -1;

                        for(int i = 0; i <= numstations-1; i++){
                            destdist[i] = isneighborofdest[i]*dist[i]*isdestthresh[i];
                        }

                        for(int i = 0; i <= numstations-1; i++){
                            if(destdist[i]>0){
                                if(destdist[i]<sd){
                                    sd = destdist[i];
                                    si = i;
                                }
                            }
                        }


                        EV << "isneighborofdest3 array: ";
                        for(int i = 0; i<= numstations-1; i++){

                            EV << isneighborofdest[i] << " ";

                        }
                        EV << endl;

                        EV << "dist array: ";
                        for(int i = 0; i<= numstations-1; i++){

                            EV << dist[i] << " ";

                        }
                        EV << endl;

                        EV << "isdestthresh array: ";
                        for(int i = 0; i<= numstations-1; i++){

                            EV << isdestthresh[i] << " ";

                        }
                        EV << endl;

                        EV << "destdist array: ";
                        for(int i = 0; i<= numstations-1; i++){

                            EV << destdist[i] << " ";

                        }
                        EV << endl;


                        EV << "I AM HERE 4" << endl;

                        EV << "si: " << si << endl;

                        if(si>=0){

                        send(bikc, "o_stn_to_stn", si);

                        }

                        else{

                        send(bikc, "o_stn_to_stn", origdest);

                        }

                        EV << "Sending message from " << ind << " to " << si << endl;
                    }
            }


            else{/* if there are no bikes available*/
                EV <<"No bike event!" << endl;

                beventctr = getParentModule()->par("beventctr_network");
                deventctr = getParentModule()->par("deventctr_network");
                nbeventctr = getParentModule()->par("nbeventctr_network");
                ndeventctr = getParentModule()->par("ndeventctr_network");

                nbeventctr++;

                getParentModule()->par("nbeventctr_network") = nbeventctr;

                    if(hasGUI()){
                        char label[50];
                        sprintf(label,"b = %.0f   d = %.0f\nnb = %.0f   nd = %.0f\nSL = %.4f%%", beventctr, deventctr, nbeventctr, ndeventctr, 100*(beventctr+deventctr)/(beventctr+deventctr+nbeventctr+ndeventctr));
                        cCanvas *canvas = getParentModule()->getCanvas();
                        cTextFigure *textFigure = check_and_cast<cTextFigure*>(canvas->getFigure("systemstats"));
                        textFigure->setText(label);
                    }

                    if(wlkc->getRejects()>0){/*if the customer has already been rejected previously)*/
                        EV <<"COMPLETELY UNSERVICED CUSTOMER LEAVING SYSTEM" << endl;
                        delete wlkc;
                    }

                    else{//If rejected for the first time

                        delete wlkc;

                        cMessage *emp = new cMessage("empty");
                        send(emp, "o_stn_to_pad");
                    }
            }
    }


    else {//if a riding customer from another station arrives
        Customer *bikc = check_and_cast<Customer *>(msg);


        int a = bikc->getOriginaldest();
         EV << ind << endl;
         EV << a << endl;


        if(bikes < cap){/*if there is space for docking*/
            EV <<"Dock event!" << endl;


            bikes = par("bikes");
            bikes++;
            par("bikes") = bikes;

            beventctr = getParentModule()->par("beventctr_network");
            deventctr = getParentModule()->par("deventctr_network");
            nbeventctr = getParentModule()->par("nbeventctr_network");
            ndeventctr = getParentModule()->par("ndeventctr_network");

            deventctr++;
            getParentModule()->par("deventctr_network") = deventctr;

            if(hasGUI()){
                char label[50];
                sprintf(label,"b = %.0f   d = %.0f\nnb = %.0f   nd = %.0f\nSL = %.4f%%", beventctr, deventctr, nbeventctr, ndeventctr, 100*(beventctr+deventctr)/(beventctr+deventctr+nbeventctr+ndeventctr));
                cCanvas *canvas = getParentModule()->getCanvas();
                cTextFigure *textFigure = check_and_cast<cTextFigure*>(canvas->getFigure("systemstats"));
                textFigure->setText(label);
            }

        delete bikc;
    }

        else{/*if there is no space for docking*/

            EV << "I AM HERE 6" << endl;

            EV <<"No dock event!" << endl;

            int a = bikc->getOriginaldest();
            int b = bikc->getOriginalstart();
            EV << b << endl;
            EV << "ind: " << ind << endl;
            EV << a << endl;
            EV << "I AM HERE 10" << endl;

            beventctr = getParentModule()->par("beventctr_network");
            deventctr = getParentModule()->par("deventctr_network");
            nbeventctr = getParentModule()->par("nbeventctr_network");
            ndeventctr = getParentModule()->par("ndeventctr_network");

            ndeventctr++;
            getParentModule()->par("ndeventctr_network") = ndeventctr;

            if(hasGUI()){
                char label[50];
                sprintf(label,"b = %.0f   d = %.0f\nnb = %.0f   nd = %.0f\nSL = %.4f%%", beventctr, deventctr, nbeventctr, ndeventctr, 100*(beventctr+deventctr)/(beventctr+deventctr+nbeventctr+ndeventctr));
                cCanvas *canvas = getParentModule()->getCanvas();
                cTextFigure *textFigure = check_and_cast<cTextFigure*>(canvas->getFigure("systemstats"));
                textFigure->setText(label);
            }


            EV << "I AM HERE 11" << endl;

            bikc->setStat(ind, 1);

            EV << "I AM HERE 12" << endl;

            int sd = 2000;
            int si = -1;

            EV << "I AM HERE 13" << endl;

            const char *vstr = par("distances");
            std::vector<int> disttt = cStringTokenizer(vstr).asIntVector();

            for(int i=0; i<=numstations-1; i++){
                bikc->setProddist(i,dist[i]);
            }

            for(int i=0; i<=numstations-1; i++){
                bikc->setProddist(i, (bikc->getProddist(i))*(1-bikc->getStat(i)));
            }

            for(int i = 0; i <= numstations-1; i++){
                if(bikc->getProddist(i)>0){
                    if(bikc->getProddist(i)<sd){
                        sd = bikc->getProddist(i);
                        si = i;
                    }
                }
            }
            EV << "I AM HERE 7" << endl;

            if(si>=0){

                EV << "I AM HERE 8" << endl;

                EV << "Sending out to the nearest station: Station " << si << " at a distance of " << sd << endl;

                send(bikc,"o_stn_to_stn",si);
            }
            else{
                EV << "I AM HERE 9" << endl;
                delete bikc;
                EV <<"BIKE IS DELETED UMIKOT NA YUNG BIKER SA LAHAT NG STATION" ;
            }
        }
    }

}

//##############################################################################################################################################################################
//######################################################################Functions###############################################################################################
//##############################################################################################################################################################################

void BikeStation::refreshDisplay() const
{

//    char buf[60];
//    sprintf(buf, "vbikes: %.0f bikes: %.0f cap:%.0f fill level:%.3f %%", vbikes, bikes, cap, 100*bikes/cap);
//    getDisplayString().setTagArg("t",0,buf);

    char buf[60];
    sprintf(buf, "bikes: %.0f cap:%.0f fill level:%.3f %%", bikes, cap, 100*bikes/cap);
    getDisplayString().setTagArg("t",0,buf);

}


