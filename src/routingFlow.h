#include "module.h"

class Router{
    Chip &_chip;
    vector<Net*> &_net_list;
public:
    Router(Chip &chip, vector<Net*> &net_list);
};