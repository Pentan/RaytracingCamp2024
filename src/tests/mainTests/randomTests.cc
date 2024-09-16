#include <iostream>
#include <sstream>
#include <fstream>

#include <doctest.h>
#include "../testsupport.h"

#include <pinkycore/pptypes.h>
#include <pinkycore/random.h>

using namespace PinkyPi;

TEST_CASE("Random test [Random]") {
    bool writelog = true;
    
    std::stringstream ss;
    ss << PINKYPI_TEST_OUTPUT_DIR << "/randomout.txt";
    std::string outpath = ss.str();
    
    std::ofstream fs(outpath, std::ios::binary);
    if(!fs.is_open()) {
        std::cerr << "random log file cannot open." << std::endl;
        writelog = false;
    }
    
    const int kNUMGEN = 1000;
    
//    SUBCASE("generate integer")
    {
        Random rng(time(NULL));
        
        if(writelog) {
            fs << "NextInt32\n";
        }
        for(int i = 0; i < kNUMGEN; i++) {
            int x = rng.nextInt32();
            if(writelog) {
                fs << x << " ";
                if(i % 10 == 9) {
                    fs << "\n";
                }
            }
        }
        fs << "\n";
    }
    
//    SUBCASE("generate float [0,1]")
    {
        Random rng(time(NULL));
        
        if(writelog) {
            fs << "nextDoubleCC\n";
        }
        for(int i = 0; i < kNUMGEN; i++) {
            double x = rng.nextDoubleCC();
            REQUIRE((x >= 0.0 && x <= 1.0));
            if(writelog) {
                fs << x << " ";
                if(i % 10 == 9) {
                    fs << "\n";
                }
            }
        }
        fs << "\n";
    }
    
//    SUBCASE("generate float [0,1)")
    {
        Random rng(time(NULL));
        
        if(writelog) {
            fs << "nextDoubleCO\n";
        }
        for(int i = 0; i < kNUMGEN; i++) {
            double x = rng.nextDoubleCO();
            REQUIRE((x >= 0.0 && x < 1.0));
            if(writelog) {
                fs << x << " ";
                if(i % 10 == 9) {
                    fs << "\n";
                }
            }
        }
        fs << "\n";
    }
    
//    SUBCASE("generate float (0,1)")
    {
        Random rng(time(NULL));
        
        if(writelog) {
            fs << "nextDoubleOO\n";
        }
        for(int i = 0; i < kNUMGEN; i++) {
            double x = rng.nextDoubleOO();
            REQUIRE((x > 0.0 && x < 1.0));
            if(writelog) {
                fs << x << " ";
                if(i % 10 == 9) {
                    fs << "\n";
                }
            }
        }
        fs << "\n";
    }
    
    fs << std::endl;
    fs.close();
}
