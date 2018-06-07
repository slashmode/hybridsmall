#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
using namespace omnetpp;

#include "Customer_m.h"

class ArrivalPad : public cSimpleModule
{
private:
    int arrivals;
    int isneighbor[5];
    int ind;
    int nrad;
    int numstations;
    double nbeventctr;
    double iat;

protected:

    virtual void refreshDisplay() const override;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(ArrivalPad);

void ArrivalPad::initialize()
{
    arrivals = 0;
    iat = par("iat");
    ind = par("ind");
    numstations = getParentModule()->par("numstations_network");
    nrad = getParentModule()->par("nrad");

    WATCH(arrivals);

    if(ind!=999){

        scheduleAt(exponential(iat), new cMessage("self"));

    }

    const char *vstrss = par("distances");
    std::vector<int> distholdee = cStringTokenizer(vstrss).asIntVector();

    for(int i = 0; i <= numstations-1; i++){//initialization of isneighbor vector
        if(distholdee[i] <= nrad){
            isneighbor[i] = 1;
        }
        else{
            isneighbor[i] = 0;
        }
    }
}

void ArrivalPad::handleMessage(cMessage *msg)
{
    EV << "I AM AT ARRIVALPAD HANDLEMESSAGE" << endl;

    EV << "original start: " << endl;

    EV << "ind: " << ind << endl;

    int travelts[numstations];

    const char *trt = par("traveltimes");
    std::vector<int> times = cStringTokenizer(trt).asIntVector();

    for(int i = 0; i <= numstations-1; i++){

        cDelayChannel *channel = check_and_cast<cDelayChannel*>(gate("o_pad_to_pad",i)->getChannel());
        channel->setDelay(times[i]);

    }


    if (msg->isSelfMessage()){/*if the message is the self-message for probabilistic arrivals*/

        delete msg;

        iat = par("iat");
        scheduleAt(simTime()+exponential(iat),new Customer("walker"));

        //Create a new message named "walker"

        Customer *msg = new Customer("walker");
        Customer *wlkc = check_and_cast<Customer *>(msg);

        int cscoreholder = uniform(1,100);
        wlkc->setCscore(cscoreholder); //Assign random number for customer cooperation factor purposes

        wlkc->setOriginalstart(ind); //Assign original starting station

        //Start computing for the original destination based on the frequency matrices
        const char *vstr = par("destinationfreqs");
        std::vector<int> v = cStringTokenizer(vstr).asIntVector();

        int cumv[numstations];
        int val = 0;

        for(int i = 0; i <= numstations-1; i++){
                val  = val + v[i];
                cumv[i] = val;
        }

        int dest = intuniform(0,(cumv[numstations-1]-1));
        for(int j = 0; j<=numstations-1;j++){
                if(dest < cumv[j]){
                    wlkc->setOriginaldest(j); //Set original destination based on trip frequencies
                    break;
                }
        }


        int sadf = wlkc->getOriginaldest();
        EV << "Just assigned a destination of " << sadf << endl;

        int cfactorholder = getParentModule()->par("cfactor");
        int cscoreholder2 = wlkc->getCscore();


        //Sending customer to starting node


        if(cscoreholder2>cfactorholder){/*If customer is not cooperative*/
            send(wlkc, "o_pad_to_stn");//send to corresponding station
            arrivals++;
        }

        else{/*If customer is cooperative*/

            EV <<"Cooperative customer detected" << endl;;

            double bhold2 = getParentModule()->getSubmodule("stn", ind)->par("bikes");
            double chold2 = getParentModule()->getSubmodule("stn", ind)->par("cap");
            double upul = getParentModule()->getSubmodule("stn", ind)->par("upul");

            if(bhold2/chold2>=fmin(upul/chold2,0.5)){/*if lampas sa thresh yung original station*/
                EV << "Since lampas 50% naman yung original kong gusto, dito na me" << endl;
                send(wlkc, "o_pad_to_stn");//send to corresponding station
            }

            else{//if wala sa threshold yung original station


                //send to nearest neighbor with fill >50%
                const char *vstr = par("distances");
                std::vector<int> dist = cStringTokenizer(vstr).asIntVector();

                int isstartthresh[numstations];

                for(int i = 0; i<=numstations-1; i++){

                    double bhold = getParentModule()->getSubmodule("stn", i)->par("bikes");
                    double chold = getParentModule()->getSubmodule("stn", i)->par("cap");
                    double upul = getParentModule()->getSubmodule("stn", i)->par("upul");

                    if ((bhold/chold)>= fmin(upul/chold,0.5)){
                        isstartthresh[i] = 1;
                    }

                    else{
                        isstartthresh[i] = 0;
                    }
                }

                int sd = 9000;
                int si = -1;

                int sredirectdist[numstations];

                int isnotdesti[numstations];

                for(int i = 0; i<= numstations-1;i++){
                    isnotdesti[i] = 1;
                }

                int ioed = wlkc->getOriginaldest();

                        isnotdesti[ioed] = 0;



                for(int i = 0; i <= numstations-1; i++){
                    sredirectdist[i]  = dist[i]*isneighbor[i]*isstartthresh[i]*isnotdesti[i];
                }



                for(int i = 0; i <= numstations-1; i++){
                    if(sredirectdist[i]>0){
                        if(sredirectdist[i]<sd){
                            sd = sredirectdist[i];
                            si = i;
                        }
                    }
                }

//Printing syntax

                EV << "sredirectdist: ";
                for(int i = 0; i <= numstations-1; i++){
                    EV << sredirectdist[i] << " ";
                }

                EV << endl << "dist: ";
                for(int i = 0; i <= numstations-1; i++){
                                EV << dist[i] << " ";
                            }

                EV << endl << "isneighbor: ";
                for(int i = 0; i <= numstations-1; i++){
                                EV << isneighbor[i] << " ";
                            }

                EV << endl << "isstartthresh: ";
                for(int i = 0; i <= numstations-1; i++){
                                EV << isstartthresh[i] << " ";
                            }

                EV << endl << "isnotdesti: ";
                               for(int i = 0; i <= numstations-1; i++){
                                               EV << isnotdesti[i] << " ";
                                           }
                EV << endl;



//End print syntax

                EV << "My original station was " << ind << " but I was redirected to " << si << endl;

                if(si>=0){
                    send(wlkc, "o_pad_to_pad", si);
                }
                else{
                    EV << "NOWHERE TO REDIRECT " << endl;



                    int dvyt = wlkc->getOriginaldest();

                    EV << "ind: " << ind << endl;
                    EV << "origdest: " << dvyt << endl;

                    send(wlkc, "o_pad_to_pad", dvyt);
                }
            }
        }

    }

    else if (strcmp(msg->getName(),"empty") == 0){ /*if the message name is empty, meaning the corresponding station has no bikes*/

        delete msg;

        Customer *msg = new Customer("walker");
        Customer *wlkc = check_and_cast<Customer *>(msg);

        int cscoreholder = uniform(1,100);
        wlkc->setCscore(cscoreholder);
        wlkc->setRejects(wlkc->getRejects()+1);

        int sd = 2000;
        int si = 0;

        const char *vstr = par("distances");
        std::vector<int> dist = cStringTokenizer(vstr).asIntVector();

        for(int i = 0; i <= numstations-1; i++){
            if(dist[i]>0){
                if(dist[i]<sd){
                    sd = dist[i];
                    si = i;
                }

            }
        }

        send(wlkc,"o_pad_to_pad",si);

        EV << "Sending walkingpassenger to gate " << si << endl;

    }

    else if(strcmp(msg->getName(),"walker") == 0){/*if the message is a customer from another station*/

        Customer *wlkc = check_and_cast<Customer *>(msg);
        int xasd = wlkc->getOriginaldest();

        if(ind!=xasd){
            send(wlkc, "o_pad_to_stn");
            arrivals++;
        }
        else{
        delete wlkc;
        EV << "PINALAKAD MO YUNG CUSTOMER SA DESTINATION NIYA!!!!" << endl;
        }

}

}

void ArrivalPad::refreshDisplay() const
{
    char buf[60];
    sprintf(buf, "arrivals: %d no bike events: %.0f", arrivals, nbeventctr);
    getDisplayString().setTagArg("t",0,buf);

}
