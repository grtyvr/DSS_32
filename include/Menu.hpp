#pragma once

class Menu {

public:
    virtual ~Menu(){
    }
    virtual void handle() = 0;
};