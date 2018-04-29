#ifndef DISTANCEVECTOR_H_
#define DISTANCEVECTOR_H_


#include "headers.h"
#include "node.h"
#include "dv_entry.h"
#include "config.h"

class DV
{




public:



///*********************** CONSTRUCTOR ***********************\\\

    DV(const char *filename, const char *self)
    {


        fstream topology(filename);                     ///FILE READER
        string line,field,temp;                         /// current line and current token
        selfName  = self[0];
        selfIndex = getIndexOf(self[0]);

        /// initialize routerList
        for (int dest = 0; dest < MAX_ROUTERS; dest++)
        {
            routerList[dest].setNextRouterName('0');
            routerList[dest].setNextRouterPort(-1);
            routerList[dest].setCost(-1);
            routerList[dest].setValid();
        }

        getline(topology, temp);                        ///Get Total Nodes
        totalNeighbour = atoi(temp.c_str());



        while (totalNeighbour--)                        ///For each neighbours create dv_entry and node
        {
            getline(topology, line);                    ///PARSE EACH LINE
            stringstream linestream(line);              ///TOKENIZE EACH FIELD FROM SCANNED LINE
            dv_entry entry;
            node n;
            int dest;



            /// Source Router
            char name = self[0];



            /// Destination Router
            getline(linestream, field, ' ');
            dest = getIndexOf(field[0]);
            n.name = field[0];
            entry.setNextRouterName(field[0]);



            /// Link Cost
            getline(linestream, field, ' ');
            entry.setCost(atoi(field.c_str()));



            /// Destination Port Number
            getline(linestream, field, ' ');
            int port = atoi(field.c_str());
            entry.setNextRouterPort(port);
            n.portno = port;


            /// Destination Address and PORT
            memset((char *)&n.addr, 0, sizeof(n.addr));
            n.addr.sin_family = AF_INET;
            n.addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            n.addr.sin_port = htons(port);


            ///Validate Distance Vector Entry
            entry.setValid();


            ///Store Neighbour
            startTimer(n);
            neighbourList.push_back(n); /// store neighbor
            routerList[dest] = entry;


            ///Map Node Name with Port No.
            portOf[n.name] = n.portno;


            ///Map Port No. with Node Name
            m_nodename[n.portno] = n.name;
        }



        /// Keep BackUp of Neighbour Information
        memcpy((void*)routerListBackup, (void*)routerList, sizeof(routerList));

    }





///*********************** GET METHODS ***********************\\\



    /// RETURNS INDEX OF ROUTER
    int getIndexOf(char router) const
    {
        return router - 'A';
    }


    /// RETURNS NAME OF ROUTER
    char getSelfNameOf(int index) const
    {
        return (char)index + 'A';
    }


    /// RETURNS PORT OF ROUTER
    int getPortNoOf(char router)
    {
        return portOf[router];
    }


    /// RETURNS Distance Vectors
    dv_entry *getEntries()
    {
        return routerList;
    }


    ///RETURNS NUMBER OF ROUTERS
    int getSize() const
    {
        return sizeof(routerList);
    }


    ///RETURNS SELF ROUTER NAME
    char getSelfName() const
    {
        return getSelfNameOf(selfIndex);
    }


    ///RETURNS SELF ROUTER PORT
    int getSelfPort()
    {
        return getPortNoOf(getSelfName());
    }


    ///RETURNS SELF ADDRESS
    sockaddr_in myaddr() const
    {
        return myAddress;
    }


    ///RETURNS DISTANCE VECTOR OF DESTINATION
    dv_entry getDestinationDistanceVector(const char dest) const
    {
        return routerList[getIndexOf(dest)];
    };


    ///RETURNS NEIGHBOURS INFO LIST
    vector<node> neighbors() const
    {
        return neighbourList;
    };



    ///CHECKS IF TIMER IS EXPIRED OR NOT
    bool timerExpired(node &n) const
    {
        timespec tend= {0,0};
        clock_gettime(CLOCK_MONOTONIC, &tend);

        if (((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)n.startTime.tv_sec + 1.0e-9*n.startTime.tv_nsec) > 5)
            return true;
        else
            return false;
    }








    ///*********************** SET METHODS ***********************\\\


    /// Initialize My Address and Port No
    void initializeMyAddress(int portno)
    {
        memset((char *)&myAddress, 0, sizeof(myAddress));
        myAddress.sin_port = htons(portno);
        myAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
        myAddress.sin_family = AF_INET;
    }



    ///Start Timer for a Node
    void startTimer(node &n)
    {
        clock_gettime(CLOCK_MONOTONIC, &n.startTime);
    }



    ///RESET ROUTING TABLE AND ROUTING LIST WHEN A NODE IS KILLED
    void reset(char dead)
    {
        for (int i = 0; i < neighbourList.size(); i++)
        {
            if (neighbourList[i].name == dead)
            {
                if (routerListBackup[getIndexOf(dead)].cost() != -1)
                    routerListBackup[getIndexOf(dead)].setInvalid();
            }
        }
        memcpy((void*)routerList, (void*)routerListBackup, sizeof(routerList));
        print(routerList, getSelfNameOf(selfIndex), "Reset routing table", true);
    }







    /// update current router's distance vector based on received advertisements
    /// print distance vector if current router's distance vector was changed


    void update(const void *recvBuffer, char source)
    {

        bool isUpdated = false;


        dv_entry actualEntry[MAX_ROUTERS];
        memcpy((void*)actualEntry, (void*)routerList, sizeof(routerList));



        int middleNode = getIndexOf(source);

        if (!routerListBackup[middleNode].isValid())
        {
            routerListBackup[middleNode].setValid();
            routerList[middleNode].setValid();

            isUpdated = true;
        }


        /// LOAD DISTANCE VECTOR WHICH IS ADVERTISED
        dv_entry adv[MAX_ROUTERS];
        memcpy((void*)adv, recvBuffer, sizeof(adv));



        /// RECALCULATE SELF DISTANCE VECTOR
        for (int i = 0; i < MAX_ROUTERS; i++)
        {
            if (i == selfIndex)
                continue;
            bool entryUpdated = false;
            routerList[i].setCost(min(routerList[i].cost(), routerList[middleNode].cost(), adv[i].cost(), routerList[i].getNextRouterName(), source, entryUpdated));
            if (entryUpdated)
            {
                isUpdated = true;
                routerList[i].setNextRouterPort(getPortNoOf(source));
                routerList[i].setNextRouterName(source);
            }
        }


        routerList[middleNode].setCost(adv[selfIndex].cost());


        if (isUpdated)
        {

            print(actualEntry, getSelfNameOf(selfIndex), "Change Occurred!\nRouting table before change", true);
            print(adv, source, "Distance Vector that caused the change", false);
            print(routerList, getSelfNameOf(selfIndex),"Routing table after change", false);

        }

    }





    ///*********************** UPDATE and PRINT ***********************\\\





    /// MINIMUM COST FINDING and UPDATED FLAG SET
    int min(int originalCost, int selfTomiddleNodeCost, int middleNodeToDestCost, char originalName, char newName, bool &updated) const
    {
        int newCost = selfTomiddleNodeCost + middleNodeToDestCost;

        if (selfTomiddleNodeCost == -1 || middleNodeToDestCost == -1)
        {
            return originalCost;
        }
        else if (originalCost == -1)
        {
            updated = true;
            return newCost;
        }
        else if (newCost < originalCost)
        {
            updated = true;
            return newCost;
        }
        else if (originalCost == newCost)
        {
            if (originalName <= newName)
                updated = false;
            else
                updated = true;
            return newCost;
        }
        else
        {
            return originalCost;
        }
    }





    /// PRINT DISTANCE VECTOR
    ///format: source, destination, cost to destination
    void print(dv_entry dv[], char name, string msg, bool timestamp) const
    {


        ///PRINT THE MESSAGE and NODE
        ///ROUTING TABLE BEFORE CHANGING - CURRENT NODE
        ///CHANGE DETECTED - NODE THAT CAUSED THE CHANGE
        ///ROUTING TABLE AFTER CHANGING - CURRENT NODE

        printf("%s: %c\n",msg.c_str(),name);


        for (int dest = 0; dest < MAX_ROUTERS; dest++)
        {

            if(getSelfNameOf(dest) == selfName) continue;
            if(m_nodename[dv[dest].getNextRouterPort()] == 0) continue;

            int port123 = dv[dest].getNextRouterPort();
            printf("shortest path to node %c: ",getSelfNameOf(dest));
            if (dv[dest].cost() == -1)
            {
                printf("Not Reachable INF\n");
                continue;
            }

            printf("the next hop is %c and the cost is %0.1lf\n",m_nodename[dv[dest].getNextRouterPort()],(double)dv[dest].cost());

        }
        cout<<endl;

    }



private:
    int selfIndex;                                      /// CURRENT ROUTER INDEX
    int totalNeighbour;                                 /// CURRENT ROUTERS TOTAL NEIGHBOUR
    sockaddr_in myAddress;                              /// CURRENT ROUTER ADDRESS
    dv_entry routerList[MAX_ROUTERS];                   /// CURRENT DISTANCE VECTOR OF ALL ROUTERS
    dv_entry routerListBackup[MAX_ROUTERS];             /// INITIAL DISTANCE VECTOR (FOR RESETTING PURPOSE)
    vector<node> neighbourList;                         /// SELF NEIGHBOURS PORT NO.
    map<char, int> portOf;

};


#endif // DISTANCEVECTOR_H_
