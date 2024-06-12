﻿// Transport Catalogue
// Code review #8

#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"
//#include "tests.h"

using namespace std;

int main() {

    //------------------ for tests ------------------
    
    //transport_catalogue::tests::Test();
    //cout << "Test was passed"s << endl;
   
    //-----------------------------------------------

    transport_catalogue::TransportCatalogue catalogue;

    transport_catalogue::reader::InputReader reader;
    reader.LoadCommands(cin, catalogue);

    transport_catalogue::reader::GetStat(catalogue, cin, cout);
    
}