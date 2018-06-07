#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
using namespace omnetpp;

#include "Customer_m.h"

class ArrivalPad : public cSimpleModule
{
private:
    int arrivals;//total number of arrivals in the pad
    int isneighbor[5];//binary matrix for neighbors of the pad
    int dist[5];//distance matrix for neighbors of the pad
    int ind;//index of the pad
    int nrad;//neighborhood radius
    int numstations;//number of stations in the BSS
    double iat;//customer inter-arrival time of the station

protected:

    virtual void refreshDisplay() const override;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(ArrivalPad);

void ArrivalPad::initialize()
{
    arrivals = 0;//initialization of arrivals to 0
    iat = par("iat");//transferring IAT parameter
    ind = par("ind");//initialization of index
    numstations = getParentModule()->par("numstations_network");//initialization of station count
    nrad = getParentModule()->par("nrad");//initialization of neighborhood radius

    WATCH(arrivals);//set arrivals as watchable

    if(ind!=999){//initialization of regular arrivals following the IAT
        scheduleAt(exponential(iat), new cMessage("self"));
    }

    const char *vstrss = par("distances");//initialization of dist and isneighbor vector
    std::vector<int> distholdee = cStringTokenizer(vstrss).asIntVector();
    for(int i = 0; i <= numstations-1; i++){
        dist[i] = distholdee[i];
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

    int travelts[numstations];//transferring arrival pad travel times from ini to specific gate
    const char *trt = par("traveltimes");
    std::vector<int> times = cStringTokenizer(trt).asIntVector();
    for(int i = 0; i <= numstations-1; i++){
        cDelayChannel *channel = check_and_cast<cDelayChannel*>(gate("o_pad_to_pad",i)->getChannel());
        channel->setDelay(times[i]);
    }


    if (msg->isSelfMessage()){/*IF THE MESSAGE IS THE SELF-MESSAGE FOR PROBABILISTIC ARRIVALS*/

        delete msg;//dispose of the self-message that just arrived
        iat = par("iat");//transferring IAT parameter
        scheduleAt(simTime()+exponential(iat),new Customer("walker"));//scheduling next arrival to follow the given IAT parameter

        Customer *msg = new Customer("walker");//Create a new message named "walker"
        Customer *wlkc = check_and_cast<Customer *>(msg);

        wlkc->setOriginalstart(ind);//Assign original starting station

        int crandom = uniform(1,100);//Determine if customer is cooperative
        int cfactorholderst = getParentModule()->par("cfactor");
        if(crandom <= cfactorholderst){
        wlkc->setIscooperative(1);
        }

        const char *vstr = par("destinationfreqs");//Start computing for the original destination based on the frequency matrices
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

        int coopholder123 = wlkc->getIscooperative();

        //Sending customer to starting node


        if(coopholder123==0){/*IF THE CUSTOMER IS NOT COOPERATIVE*/
            send(wlkc, "o_pad_to_stn");//send to corresponding station
            arrivals++;
        }

        else{/*IF THE CUSTOMER IS COOPERATIVE*/

            EV <<"Cooperative customer detected" << endl;;

            double bhold2 = getParentModule()->getSubmodule("stn", ind)->par("vbikes");
            double chold2 = getParentModule()->getSubmodule("stn", ind)->par("cap");
            double upul = getParentModule()->getSubmodule("stn", ind)->par("upul");

            if(bhold2/chold2>=fmin(upul/chold2,0.5)){/*IF THE RESULTING LEVEL IS WITHIN THE THRESHOLD*/
                EV << "Since lampas 50% naman yung original kong gusto, dito na me" << endl;
                send(wlkc, "o_pad_to_stn");//send to corresponding station
            }

            else{/*IF THE RESULTING LEVEL IS OUTSIDE THE THRESHOLD*/

                //send to nearest neighbor with fill >50%

                int isstartthresh[numstations];//filling isstartthresh array
                for(int i = 0; i<=numstations-1; i++){
                    double bhold = getParentModule()->getSubmodule("stn", i)->par("vbikes");
                    double chold = getParentModule()->getSubmodule("stn", i)->par("cap");
                    double upul = getParentModule()->getSubmodule("stn", i)->par("upul");
                    if ((bhold/chold)>= fmin(upul/chold,0.5)){
                        isstartthresh[i] = 1;
                    }
                    else{
                        isstartthresh[i] = 0;
                    }
                }

                int isnotdesti[numstations];//filling isnotdesti array
                for(int i = 0; i<= numstations-1;i++){
                    isnotdesti[i] = 1;
                }
                int ioed = wlkc->getOriginaldest();
                isnotdesti[ioed] = 0;


                int sredirectdist[numstations];//filling sredirectdist array
                for(int i = 0; i <= numstations-1; i++){
                    sredirectdist[i]  = dist[i]*isneighbor[i]*isstartthresh[i]*isnotdesti[i];
                }

                int sd = 9000;//choose the shortest distance factoring in all other conditions
                int si = -1;
                for(int i = 0; i <= numstations-1; i++){
                    if(sredirectdist[i]>0){
                        if(sredirectdist[i]<sd){
                            sd = sredirectdist[i];
                            si = i;
                        }
                    }
                }

////Printing syntax
//
//                EV << "sredirectdist: ";
//                for(int i = 0; i <= numstations-1; i++){
//                    EV << sredirectdist[i] << " ";
//                }
//
//                EV << endl << "dist: ";
//                for(int i = 0; i <= numstations-1; i++){
//                                EV << dist[i] << " ";
//                            }
//
//                EV << endl << "isneighbor: ";
//                for(int i = 0; i <= numstations-1; i++){
//                                EV << isneighbor[i] << " ";
//                            }
//
//                EV << endl << "isstartthresh: ";
//                for(int i = 0; i <= numstations-1; i++){
//                                EV << isstartthresh[i] << " ";
//                            }
//
//                EV << endl << "isnotdesti: ";
//                               for(int i = 0; i <= numstations-1; i++){
//                                               EV << isnotdesti[i] << " ";
//                                           }
//                EV << endl;
//
////End print syntax

                EV << "My original station was " << ind << " but I was redirected to " << si << endl;

                if(si>=0){//if shortest distance exists, send to corresponding index

                    int vbikesholder12 = getParentModule()->getSubmodule("stn", si)->par("vbikes");//update vbikes of destination
                    vbikesholder12 = vbikesholder12 - 1;
                    getParentModule()->getSubmodule("stn", si)->par("vbikes");

                    send(wlkc, "o_pad_to_pad", si);
                }

                else{//IF SHORTEST DISTANCE DOES NOT EXIST, SEND TO NEAREST STATION

                    int sd = 9000;
                    int si = -1;
                    for(int i = 0; i <= numstations-1; i++){
                        if(dist[i]>0){
                            if(dist[i]<sd){
                                sd = dist[i];
                                si = i;
                            }

                        }
                    }

                    int vbikesholder12 = getParentModule()->getSubmodule("stn", si)->par("vbikes");//update vbikes of destination
                    vbikesholder12 = vbikesholder12 - 1;
                    getParentModule()->getSubmodule("stn", si)->par("vbikes");

                    send(wlkc, "o_pad_to_pad", si);

                    }
            }
        }
    }

    else if (strcmp(msg->getName(),"empty") == 0){ /*if the message name is empty, meaning the corresponding station has no bikes*/

        Customer *wlkc = check_and_cast<Customer *>(msg);//cast pointer to readable label

        wlkc->setRejects(wlkc->getRejects()+1);//increase reject counter

        int sd = 9000;
        int si = -1;

        for(int i = 0; i <= numstations-1; i++){
            if(dist[i]>0){
                if(dist[i]<sd){
                    sd = dist[i];
                    si = i;
                }

            }
        }

        wlkc ->setName("walker");

        if(si>=0){//if shortest distance exists, send to corresponding index

            int vbikesholder12 = getParentModule()->getSubmodule("stn", si)->par("vbikes");//update vbikes of destination
            vbikesholder12 = vbikesholder12 - 1;
            getParentModule()->getSubmodule("stn", si)->par("vbikes");

            send(wlkc, "o_pad_to_pad", si);
        }

        else{//if shortest distance does not exist, send to original destination
            EV << "NOWHERE TO REDIRECT " << endl;
            int dvyt = wlkc->getOriginaldest();
            EV << "ind: " << ind << endl;
            EV << "origdest: " << dvyt << endl;
            send(wlkc, "o_pad_to_pad", dvyt);
        }
    }

    else if(strcmp(msg->getName(),"walker") == 0){/*if the message is a customer from another station*/

        Customer *wlkc = check_and_cast<Customer *>(msg);//cast pointer to readable label

        int xasd = wlkc->getOriginaldest();

        if(ind!=xasd){//if the this station is not the customer's original destination, send to stn
            send(wlkc, "o_pad_to_stn");
            arrivals++;
        }

        else{//if this station is the original destion, delete message, customer quits
        delete wlkc;
        EV << "PINALAKAD MO YUNG CUSTOMER SA DESTINATION NIYA!!!!" << endl;
        }

}

}

void ArrivalPad::refreshDisplay() const
{
    char buf[60];
    sprintf(buf, "arrivals: %d", arrivals);
    getDisplayString().setTagArg("t",0,buf);

}
