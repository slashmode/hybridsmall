//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//x


simple BikeStation
{
    parameters:
        int bikes = default(0);
        int vbikes = default(0);
        int cap = default(0);
        int upul = default(1);
        int upll = default(2);
        int ind = default(999);
        string destinationfreqs = default("0 0 0 0 0");
        string traveltimes = default("0 0 0 0 0");
        string distances = default("0 0 0 0 0");
        @display("i=misc/town_s");

    gates:
        input i_stn_to_stn[5];
        output o_stn_to_stn[5];
        input i_pad_to_stn;
        output o_stn_to_pad;
}

simple ArrivalPad
{
    parameters:
        int ind = default(999);
        double iat @unit(s) = default(999s);
        string destinationfreqs = default("0 0 0 0 0");
        string traveltimes = default("0 0 0 0 0");
        string distances = default("0 0 0 0 0");
        @display("i=abstract/penguin_s");

    gates:
        input i_pad_to_pad[5];
        output o_pad_to_pad[5];
        input i_stn_to_pad;
        output o_pad_to_stn;
}

network UPBSSN
{
    parameters:
        @figure[description](type=text; pos=5,20; font=,,bold; text="Simulation of UP Bike Sharing System Network");
        @figure[systemstats](type=text; pos=5,35; text="total no bike events = N/A\ntotal events = N/A\nService Level = N/A");
        int numstations_network = 5;
        int beventctr_network = 0;
        int deventctr_network = 0;
        int nbeventctr_network = 0;
        int ndeventctr_network = 0;
        int cfactor = 50;
        int nrad = 2500;
        string fillarray_network = "0 0 0 0 0";
        string bikesarray_network = "0 8 15 7 12";
        string vbikesarray_network = "0 8 15 7 12";
        string caparray_network = "14 13 15 18 12";
        string upul_network = "1 1 1 1 1";
        string upll_network = "2 2 2 2 2";

        @display("bgb=1065.47,728.035");
        
    types:
        channel Standard extends ned.DelayChannel
        {
            delay = 250ms;
        }
        
    submodules:

        stn[numstations_network]: BikeStation {
            @display("p=80.128,180,ri,300,250");
        }
        pad[numstations_network]: ArrivalPad {
            @display("p=110.272,210,ri,300,250");
        }
        dummystn: BikeStation {
            @display("p=950.136,639.768");
        }
        dummypad: ArrivalPad {

            @display("p=995.52,625.128");
        }
    connections:

        for i=0..numstations_network-1, for j=0..numstations_network-1 {
            stn[i].o_stn_to_stn[j] --> ned.DelayChannel --> stn[j].i_stn_to_stn[i] if i!=j;
            stn[i].o_stn_to_stn[j] --> ned.DelayChannel --> dummystn.i_stn_to_stn[i] if i==j;
        }

        for i=0..numstations_network-1, for j=0..numstations_network-1 {
            pad[i].o_pad_to_pad[j] --> ned.DelayChannel --> pad[j].i_pad_to_pad[i] if i!=j;
            pad[i].o_pad_to_pad[j] --> ned.DelayChannel --> dummypad.i_pad_to_pad[i] if i==j;
        }

        for i=0..numstations_network-1 {
            dummystn.o_stn_to_stn[i] --> ned.IdealChannel --> stn[i].i_stn_to_stn[i];
            dummypad.o_pad_to_pad[i] --> ned.IdealChannel --> pad[i].i_pad_to_pad[i];
            pad[i].o_pad_to_stn --> ned.IdealChannel --> stn[i].i_pad_to_stn;
            stn[i].o_stn_to_pad --> ned.IdealChannel --> pad[i].i_stn_to_pad;
        }

        dummystn.o_stn_to_pad --> ned.IdealChannel --> dummypad.i_stn_to_pad;
        dummypad.o_pad_to_stn --> ned.IdealChannel --> dummystn.i_pad_to_stn;

}



