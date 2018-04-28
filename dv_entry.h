#ifndef DVENTRY_H_
#define DVENTRY_H_

struct dv_entry
{
public:
    ///Check Validity of Router
    bool isValid() const
    {
        return valid;
    }

    ///Set Next Router Port Number
    void setNextRouterPort(int var)
    {
        nextRouterPort = var;
        return;
    }

    ///Set Next Router Name
    void setNextRouterName(char var)
    {
        nextRouterName = var;
        return;
    }

    ///Set Link Cost
    void setCost(int c)
    {
        linkCost = c;
        return;
    }

    ///Validate Router
    void setValid()
    {
        valid = true;
        return;
    }

    ///Set Router Invalid
    void setInvalid()
    {
        valid = false;
        return;
    }

    ///Get Next Router Port Number
    int getNextRouterPort() const
    {
        if(!isValid())      return -1;
        return nextRouterPort;
    }

    ///Get Next Router Name
    char getNextRouterName() const
    {
        if(!isValid())      return '0';
        return nextRouterName;
    }

    ///Get Link Cost
    int cost() const
    {
        if(!isValid())      return -1;
        return linkCost;
    }


private:
    bool valid;                             ///validation check flag
    char nextRouterName;                   ///Name of Next Router
    int nextRouterPort,linkCost;           ///Next Router Port Number and Link Cost

};

#endif
