#include <iostream> //for cin
#include <string>   //for basically everything? kek
#include <sstream>  //for dealing with some specific input shinanegans
#include <random>   //for the stable encryption algorithm
#include <stdio.h>  //for file control


constexpr unsigned long long int charsize_const{256};           //all the characters possible
constexpr int ASCIIctrl_const{32};                              //ascii ctrl characters
long long int actual_chars{charsize_const - ASCIIctrl_const};   //total amt of chars to be used


unsigned long long int global_seed{0};  //current seed to send to engine
std::string key{""};                    //key to use as encryption fodder
std::mt19937_64* engine{nullptr};       //current instance of RNG

FILE *read_file{nullptr};   //file to read from
FILE *write_file{nullptr};  //file to write to


//this func controls char under- and over-flows, and shifting in general
char shift_ctrl(unsigned int offset, unsigned char data, bool DownShift){

    if(data < ASCIIctrl_const){ //make sure control characters are not ever fucked with
        return data;
    }

    int movement = offset % actual_chars;

    if(DownShift){  //downshift
        if(movement == 0){
            return data;
        }
        else if((data - movement) < ASCIIctrl_const){
            return actual_chars + (data - movement);
        }
        else{
            return data - movement;
        }
    }
    else{           //upshift
        if(movement == 0){
            return data;
        }
        else if(data + movement >= charsize_const){
            return ASCIIctrl_const + ((data + movement) - charsize_const);
        }
        else{
            return data + movement;
        }
    }

}

//function that creates an RNG with the current global_seed
void create_engine(void){

    if(engine != nullptr){
        std::cout << "Engine already exists. Aborting." << std::endl;
        exit(1);
    }

    engine = new std::mt19937_64;
    engine->seed(global_seed);

}

//function that destroys the current RNG
void destroy_engine(void){

    delete engine;
    engine = nullptr;

}

//creates an RNG, uses it to generate a series of offsets which are appended to or subtracted from characters in data, destroys the RNG
void scramble(std::string& data, bool DownShift){

    create_engine();
    for(int i = 0; i < data.length(); i++){
        data[i] = shift_ctrl((*engine)(), data[i], DownShift);
    }
    destroy_engine();

}

//i feel like this function seldom needs an introduction
int main()
{

    std::string input{""};
    char asker{' '};
    int iasker{0};

    bool DownShift{true};
    bool Rfrom_File{true};
    bool Wto_File{true};

    std::string data{""};


    //asks for source of data
    tag_Rfrom:
    std::cout << "Reading from (F)ile or (C)onsole?: ";

    getline(std::cin, input);
    std::stringstream(input) >> asker;

    if(asker == 'C' || asker == 'c'){
        tag_Rfrom_JIC:
        Rfrom_File = false;
        std::cout << "Warning: input to console can sometimes be faulty and result in data loss/corruption during shifting." << std::endl;
    }
    else if(asker == 'F' || asker == 'f'){
        Rfrom_File = true;

        tag_Rfrom_Fpath:
        std::cout << "Enter input filepath: ";

        getline(std::cin, input);

        read_file = fopen(input.c_str(), "r");

        if(read_file == nullptr){
            std::cout << "File failed to access. Try another filepath or enter from console? (F/c): ";

            tag_Rfrom_retry:
            getline(std::cin, input);
            std::stringstream(input) >> asker;

            if(asker == 'C' || asker == 'c'){
                goto tag_Rfrom_JIC;
            }
            else if(asker == 'F' || asker == 'f'){
                goto tag_Rfrom_Fpath;
            }
            else{
                std::cout << "Not a valid input. Try that again." << std::endl;
                goto tag_Rfrom_retry;
            }
        }

        std::cout << "File accessing success!" << std::endl;
    }
    else{
        std::cout << "Not a valid input. Try that again." << std::endl;
        goto tag_Rfrom;
    }

        //asks for the data
    if(Rfrom_File){     //if file input
        iasker = 0;
        while((iasker = fgetc(read_file) )!= EOF){
            data = data + (char)iasker;
        }
        std::cout << "Data read from file successfully." << std::endl;
    }
    else{               //if con input
        std::cout << "Data to process: ";
        getline(std::cin, data);
    }

    //asks for destination for data
    tag_Wto:
    std::cout << "Writing to (F)ile or (C)onsole?: ";

    getline(std::cin, input);
    std::stringstream(input) >> asker;

    if(asker == 'C' || asker == 'c'){
        tag_Wto_JIC:
        Wto_File = false;
        std::cout << "Warning: some characters may fail to print on console, especially when encrypting, decrypting with a bad key or using a custom font." << std::endl;
    }
    else if(asker == 'F' || asker == 'f'){
        Wto_File = true;

        tag_Wto_Fpath:
        std::cout << "Warning: this method of output will overwrite everything in the destination file.\nEnter output filepath: ";
        getline(std::cin, input);

        write_file = fopen(input.c_str(), "w");

        if(write_file == nullptr){
            std::cout << "File failed to access. Try another filepath or write to console? (F/c): ";

            tag_Wto_retry:
            getline(std::cin, input);
            std::stringstream(input) >> asker;

            if(asker == 'C' || asker == 'c'){
                goto tag_Wto_JIC;
            }
            else if(asker == 'F' || asker == 'f'){
                goto tag_Wto_Fpath;
            }
            else{
                std::cout << "Not a valid input. Try that again." << std::endl;
                goto tag_Wto_retry;
            }

        }

        write_file = freopen(input.c_str(), "a", write_file);

        std::cout << "File accessing success!" << std::endl;

    }
    else{
        std::cout << "Not a valid input. Try that again." << std::endl;
        goto tag_Wto;
    }


    //asks for direction to shift data
    tag_shift:
    std::cout << "Upshift or downshift? (U/d): ";

    getline(std::cin, input);
    std::stringstream(input) >> asker;

    if(asker == 'U' || asker == 'u'){
        DownShift = false;
    }
    else if(asker == 'D' || asker == 'd'){
        DownShift = true;
    }
    else{
        std::cout << "Not a valid input. Try that again." << std::endl;
        goto tag_shift;
    }


    //asks for the key to use as shifting fodder
    std::cout << "Key to use: ";
    getline(std::cin, key);


    //shifts the data
    for(int i = 0; i < key.length(); i++){
        global_seed += key[i];
        //std::cout << "\nseed is: " << global_seed << " and element is " << key[i] << std::endl;
        scramble(data, DownShift);
    }


    //returns data
    if(Wto_File){   //if file output
        for(int i = 0; i < data.length(); i++){
            fputc(data[i], write_file);
        }
        std::cout << "Data written to file successfully." << std::endl;
        //make filewriter
    }
    else{           //if con output
        std::cout << "\nResult of shifting: " << data << std::endl;
    }


    return 0;
}
