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

    double bikes;//# of bikes in the station
    double vbikes;//virtual # of bike sin the station
    double cap;//capacity of the station
    int numstations;//number of stations in the BSS
    int ind;//index of the station
    int nrad;//neighborhood radius
    double deventctr;//dock event counter
    double beventctr;//bike event counter
    double nbeventctr;//no-bike event counter
    double ndeventctr;//no-dock event counter
    double sl;//service level
    int isneighbor[5];//binary matrix for neighbors of the station
    int dist[5];//distances to each of the other stations


protected:

    virtual void refreshDisplay() const override;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(BikeStation);

void BikeStation::initialize()
{
    beventctr = 0;//initialization of bike event counter
    deventctr = 0;//initialization of dock event counter
    nbeventctr = 0;//initialization of no-bike event counter
    ndeventctr = 0;//initialization of no-dock event counter
    sl = 0;//initialization of service level
    bikes = par("bikes");//initialization of the initial number of bikes for all stations
    vbikes = bikes;//making virtual bikes equal to actual bikes at the start
    cap = par("cap");//initialization of bike station capacities
    ind = par("ind");//initialization of station indexes
    numstations = getParentModule()->par("numstations_network");//initialization of the number of stations
    nrad = getParentModule()->par("nrad");//initialization of neighborhood radius

    WATCH(beventctr);//labeling of watchable variables
    WATCH(deventctr);
    WATCH(nbeventctr);
    WATCH(ndeventctr);
    WATCH(sl);
    WATCH(numstations);
    WATCH(bikes);
    WATCH(cap);

    const char *vstr = par("distances");//filling in the dist and isneighbor matrix
    std::vector<int> disthold = cStringTokenizer(vstr).asIntVector();
    for(int i = 0; i <= numstations-1; i++){
        EV <<  disthold[i] << " ";
        dist[i] = disthold[i];
        if(disthold[i] <= nrad){
            isneighbor[i] = 1;
        }
        else{
            isneighbor[i] = 0;
        }
    }
}

void BikeStation::handleMessage(cMessage *msg)
{

    int traveltss[numstations];//transferring stn travel times from ini to specific gate
    const char *trts = par("traveltimes");
    std::vector<int> timess = cStringTokenizer(trts).asIntVector();
    for(int i = 0; i <= numstations-1; i++){
        cDelayChannel *channel = check_and_cast<cDelayChannel*>(gate("o_stn_to_stn",i)->getChannel());
        channel->setDelay(timess[i]);
    }

    //remote submodule par access code:
    //asdfar = getParentModule()->getSubmodule("stn", 2)->par("bikes");

    if (strcmp(msg->getName(),"walker") == 0){/*IF A CUSTOMER WANTING TO RIDE ARRIVES*/

        Customer *wlkc = check_and_cast<Customer *>(msg);//cast pointer to readable label

        int a = wlkc->getOriginaldest();
        EV <<"ind: " << ind << endl;
        EV << a << endl;

        if (bikes > 0){/*IF THERE IS A BIKE AVAILABLE FOR THE CUSTOMER WANTING TO RIDE*/

            int coopholder234 = wlkc->getIscooperative();

            if(coopholder234==0){/*IF THE CUSTOMER WANTING TO RIDE IS NOT COOPERATIVE*/

                EV <<"Bike event!" << endl;

                bikes = par("bikes"); //Update bike parameter value for easy access from other submodules
                bikes--;
                par("bikes") = bikes;

                Customer *bikc = check_and_cast<Customer *>(wlkc);
                bikc->setName("biker");
                bikc->setDisplayString("i=bss/biker");

                int desthshsh = bikc->getOriginaldest();
                send(bikc, "o_stn_to_stn", desthshsh); //Send customer to original destination

                int vbikesholder123 = getParentModule()->getSubmodule("stn", desthshsh)->par("vbikes");//update vbikes of destination
                vbikesholder123++;
                getParentModule()->getSubmodule("stn", desthshsh)->par("vbikes");

                EV << "Sending message from " << ind << "to " << desthshsh << endl;

                beventctr = getParentModule()->par("beventctr_network");//update counters with beventctr++
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


            else{/*IF THE CUSTOMER WANTING TO RIDE IS COOPERATIVE*/

                EV <<"Bike event!" << endl;

                bikes = par("bikes"); //Update bike parameter value for easy access from other submodules
                bikes--;
                par("bikes") = bikes;

                Customer *bikc = check_and_cast<Customer *>(wlkc);//change message name to biker
                bikc->setName("biker");
                bikc->setDisplayString("i=bss/biker");


                int origdest = bikc->getOriginaldest();//determine the neighbors of the destination station and fill the isneighborofdest array
                const char *vstrrryr = getParentModule()->getSubmodule("stn", origdest)->par("distances");
                std::vector<int> distfromorigdest = cStringTokenizer(vstrrryr).asIntVector();
                int isneighborofdest[numstations];
                for(int i = 1; i<=numstations-1; i++){
                    if(distfromorigdest[i]<=nrad){
                        isneighborofdest[i] = 1;
                    }
                    else{
                        isneighborofdest[i] = 0;
                    }
                }

                int isdestthresh[numstations];//fil the isdestthresh array
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

                int destdist[numstations];//compute for the destdist array
                for(int i = 0; i <= numstations-1; i++){
                    destdist[i] = isneighborofdest[i]*dist[i]*isdestthresh[i];
                }


                int sd = 9000;//compute for the shortest distance and its corresponding index
                int si = -1;
                for(int i = 0; i <= numstations-1; i++){
                    if(destdist[i]>0){
                        if(destdist[i]<sd){
                            sd = destdist[i];
                            si = i;
                        }
                    }
                }

                beventctr = getParentModule()->par("beventctr_network");//update counters with beventctr++
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

                if(si>=0){/*IF SHORTEST DISTANCE EXISTS, SENT TO CORRESPONDING INDEX*/

                    int vbikesholder123 = getParentModule()->getSubmodule("stn", si)->par("vbikes");//update vbikes of destination
                    vbikesholder123 = vbikesholder123 + 1;
                    getParentModule()->getSubmodule("stn", si)->par("vbikes");
                    send(bikc, "o_stn_to_stn", si);

                }

                else{/*IF SHORTEST DISTANCE DOES NOT EXIST, BIKE TO ORIGINAL DESTINATION*/

                    int vbikesholder123 = getParentModule()->getSubmodule("stn", origdest)->par("vbikes");//update vbikes of destination
                    vbikesholder123 = vbikesholder123 + 1;
                    getParentModule()->getSubmodule("stn", origdest)->par("vbikes");
                    send(bikc, "o_stn_to_stn", origdest);

                }
            }
        }


        else{/*IF THERE IS NO BIKE AVAILABLE FOR THE CUSTOMER WANTING TO RIDE*/

            EV <<"No bike event!" << endl;

            beventctr = getParentModule()->par("beventctr_network");//update counters with nbeventctr++
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

            if(wlkc->getRejects()>0){/*IF THE CUSTOMER HAS ALREADY BEEN REJECTED BY AN EMPTY STATION PREVIOUSLY*/
                EV <<"COMPLETELY UNSERVICED CUSTOMER LEAVING SYSTEM" << endl;
                delete wlkc;
            }

            else{/*IF CUSTOMER WAS REJECTED BY AN EMPTY STATION FOR THE FIRST TIME*/
                wlkc ->setName("empty");
                send(wlkc, "o_stn_to_pad");
            }
        }
    }


    else{/*IF A BIKING CUSTOMER ARRIVES FROM ANOTHER STATION*/

        Customer *bikc = check_and_cast<Customer *>(msg);
        int a = bikc->getOriginaldest();
        EV << ind << endl;
        EV << a << endl;

        if(bikes < cap){/*IF THERE IS SPACE FOR DOCKING*/
            EV <<"Dock event!" << endl;


            bikes = par("bikes");
            bikes++;
            par("bikes") = bikes;

            beventctr = getParentModule()->par("beventctr_network");//update counters with deventctr++
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

            delete bikc;//programatically dispose of the customer

        }

        else{/*IF THERE IS NO SPACE FOR DOCKING*/

            EV <<"No dock event!" << endl;

            beventctr = getParentModule()->par("beventctr_network");//update counters with ndeventctr++
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

            bikc->setStat(ind, 1);//fill in stat array for stations already visited

            int sd = 9000;
            int si = -1;

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

            if(si>=0){

                int vbikesholder123 = getParentModule()->getSubmodule("stn", si)->par("vbikes");//update vbikes of destination
                vbikesholder123++;
                getParentModule()->getSubmodule("stn", si)->par("vbikes");

                send(bikc,"o_stn_to_stn",si);
            }
            else{

                delete bikc;
                EV <<"BIKE IS DELETED UMIKOT NA YUNG BIKER SA LAHAT NG STATION" ;
            }
        }
    }

}

void BikeStation::refreshDisplay() const
{

//    char buf[60];
//    sprintf(buf, "vbikes: %.0f bikes: %.0f cap:%.0f fill level:%.3f %%", vbikes, bikes, cap, 100*bikes/cap);
//    getDisplayString().setTagArg("t",0,buf);

    char buf[60];
    sprintf(buf, "bikes: %.0f cap:%.0f fill level:%.3f %%", bikes, cap, 100*bikes/cap);
    getDisplayString().setTagArg("t",0,buf);

}


