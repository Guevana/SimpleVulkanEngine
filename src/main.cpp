
#include "lve_FirstAPP.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(){
    try{
    lve::FirstAPP app{};
    app.run();
    }catch(const std::exception &e){
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}