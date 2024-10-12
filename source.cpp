#include <iostream> //for cin
#include <string>   //for basically everything? kek
#include <sstream>  //for dealing with some specific input shinanegans
#include <random>   //for the stable encryption algorithm
#include <stdio.h>  //for file control
#include <string.h> //for c-string stuff
#include <math.h>   //for some specific math

//i pre-emptively apologize for mixing together stuff from C and C++

constexpr unsigned long long int charsize_const{256};           //all the characters possible
constexpr int ASCIIctrl_const{32};                              //ascii ctrl characters
long long int actual_chars{charsize_const - ASCIIctrl_const};   //total amt of chars to be used

unsigned long long int global_seed{0};  //current seed to send to engine
std::string key{""};                    //key to use as encryption fodder
std::mt19937_64* engine{nullptr};       //current instance of RNG

FILE *read_file{nullptr};   //file to read from
FILE *write_file{nullptr};  //file to write to
FILE *key_file{nullptr};    //file to get key from

int file_refs[]{        //positions of filepaths in argv

    0,                  //read_file
    0,                  //write_file
    0                   //key_file

};

constexpr int REF_IN    = 0;
constexpr int REF_OUT   = 1;
constexpr int REF_KEY   = 2;

char *all_args[]{       //all args to look at

    "-T",               //enables full text user interface
    "--text-ui",        //^


    "-i",               //determines filepath for input
    "--file-in",        //^

    "-I",               //enables console input (MUTUALLY EXCLUSIVE WITH -i)
    "--console-in",     //^


    "-o",               //determines filepath for output
    "--file-out",       //^

    "-O",               //enables console output (MUTUALLY EXCLUSIVE WITH -o)
    "--console-out",    //^


    "-k",               //determines filepath for key
    "--file-key",       //^

    "-K",               //enables console for key (MUTUALLY EXCLUSIVE WITH -k)
    "--console-key"     //^
};

bool enabled_args[]{    //enabled args

    false,              //-T

    false,              //-i

    false,              //-I

    false,              //-o

    false,              //-O

    false,              //-k

    false               //-K
};

constexpr int ARG_TEXT_UI   = 0;
constexpr int ARG_IN_FILE   = 1;
constexpr int ARG_IN_CON    = 2;
constexpr int ARG_OUT_FILE  = 3;
constexpr int ARG_OUT_CON   = 4;
constexpr int ARG_KEY_FILE  = 5;
constexpr int ARG_KEY_CON   = 6;

//we call this func at beginning of program to deal with args
int arg_ctrl(int argc, char** argv){

    for(int i = 1; i < argc; i++){

        for(int j = 0; j < sizeof(enabled_args) * 2; j++){

            if(strcmp(argv[i], all_args[j]) == 0){

                int shift_arg = floor((j / 2));  //we do this because there's twice as many entries in all_args as compared to enabled_args

                switch(shift_arg){

                    case(ARG_IN_FILE):    //-i
                    {

                    if(i < argc){
                        i++;                               //skips over the next arg, as it may be a filepath
                        read_file = fopen(argv[i], "r");    //pre-emptively declares next arg as filepath

                        if(read_file == nullptr){
                            std::cout << "File accessing error. The file \"" << argv[i] << "\" is an invalid input source." << std::endl;
                            return 1;
                        }
                        else{
                            fclose(read_file);
                            file_refs[REF_IN] = i;
                        }

                    }
                    else{
                        std::cout << "Bad usage. -i used, but no filepath specified." << std::endl;
                        return 1;
                    }

                    if(enabled_args[ARG_IN_CON]){
                        std::cout << "Bad usage. Arguments -i and -I are incompatible." << std::endl;
                        return 1;
                    }

                    break;
                    }
                    case(ARG_IN_CON):    //-I
                    {

                    if(enabled_args[ARG_IN_FILE]){
                        std::cout << "Bad usage. Arguments -i and -I are incompatible." << std::endl;
                        return 1;
                    }

                    break;
                    }
                    case(ARG_OUT_FILE):    //-o
                    {

                    if(i < argc){
                        i++;                               //skips over the next arg, as it may be a filepath
                        write_file = fopen(argv[i], "r");   //pre-emptively declares next arg as filepath

                        if(write_file == nullptr){
                            write_file = fopen(argv[i], "a");

                            if(write_file == nullptr){
                            std::cout << "File accessing error. The file \"" << argv[i] << "\" is an invalid output destination." << std::endl;
                            return 1;
                            }
                            else{
                                fclose(write_file);
                                file_refs[REF_OUT] = i;
                            }

                        }
                        else{
                            fclose(write_file);
                            file_refs[REF_OUT] = i;
                        }

                    }
                    else{
                        std::cout << "Bad usage. -o used, but no filepath specified." << std::endl;
                        return 1;
                    }

                    if(enabled_args[ARG_OUT_CON]){
                        std::cout << "Bad usage. Arguments -o and -O are incompatible." << std::endl;
                        return 1;
                    }

                    break;
                    }
                    case(ARG_OUT_CON):    //-O
                    {

                    if(enabled_args[ARG_OUT_FILE]){
                        std::cout << "Bad usage. Arguments -o and -O are incompatible." << std::endl;
                        return 1;
                    }

                    break;
                    }
                    case(ARG_KEY_FILE):   //-k
                    {

                    if(i < argc){
                        i++;                               //skips over the next arg, as it may be a filepath
                        key_file = fopen(argv[i], "r");     //pre-emptively declares next arg as filepath

                        if(key_file == nullptr){
                            std::cout << "File accessing error. The file \"" << argv[i] << "\" is an invalid key source." << std::endl;
                            return 1;
                        }
                        else{
                            fclose(key_file);
                            file_refs[REF_KEY] = i;
                        }

                    }
                    else{
                        std::cout << "Bad usage. -k used, but no filepath specified." << std::endl;
                        return 1;
                    }

                    if(enabled_args[ARG_KEY_CON]){
                        std::cout << "Bad usage. Arguments -k and -K are incompatible." << std::endl;
                        return 1;
                    }

                    break;
                    }
                    case(ARG_KEY_CON):   //-K
                    {

                    if(enabled_args[ARG_KEY_FILE]){
                        std::cout << "Bad usage. Arguments -k and -K are incompatible." << std::endl;
                        return 1;
                    }

                    break;
                    }

                }

                enabled_args[shift_arg] = true;
                goto all_good;

            }

        }
        std::cout << "Bad usage. Argument \"" << argv[i] << "\" is invalid." << std::endl;
        return 1;

        all_good:

    }

    return 0;

}

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


//controls input handling if -T is specified
int data_I_ctrl(bool* FileToggle, std::string *data, char** argv){

        //skip if args provide input method
    if(enabled_args[ARG_IN_FILE] || enabled_args[ARG_IN_CON]){
        if(enabled_args[ARG_IN_FILE]){
            *(FileToggle) = true;
            read_file = fopen(argv[file_refs[REF_IN]], "r");
        }
        else{
            *(FileToggle) = false;
        }
        return 0;
    }

    char asker{' '};
    std::string input{""};

        //grant user choice
    tag_Rfrom:
    std::cout << "Reading from (F)ile or (C)onsole?: ";

    getline(std::cin, input);
    std::stringstream(input) >> asker;

    if(asker == 'C' || asker == 'c'){
        tag_Rfrom_JIC:
        *(FileToggle) = false;
        std::cout << "Warning: input to console can sometimes be faulty and result in data loss/corruption during shifting." << std::endl;
    }
    else if(asker == 'F' || asker == 'f'){
        *(FileToggle) = true;

        tag_Rfrom_Fpath:
        std::cout << "Enter input filepath: ";

        getline(std::cin, input);

        read_file = fopen(input.c_str(), "r");

        if(read_file == nullptr){
            tag_Rfrom_acc_fail:
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

        std::cout << "\n";

        }

    }
    else{
        std::cout << "Not a valid input. Try that again." << std::endl;
        goto tag_Rfrom;
    }


    return 0;

}


//controls output handling if -T is specified
int data_O_ctrl(bool* FileToggle, std::string *data, char** argv){

        //skip if args provide output method
    if(enabled_args[ARG_OUT_FILE] || enabled_args[ARG_OUT_CON]){
        if(enabled_args[ARG_OUT_FILE]){
            *(FileToggle) = true;
            write_file = fopen(argv[file_refs[REF_OUT]], "w");
        }
        else{
            *(FileToggle) = false;
        }
        return 0;
    }

    char asker{' '};
    std::string input{""};

    tag_Wto:
    std::cout << "Writing to (F)ile or (C)onsole?: ";

    getline(std::cin, input);
    std::stringstream(input) >> asker;

    if(asker == 'C' || asker == 'c'){
        tag_Wto_JIC:
        *(FileToggle) = false;
        std::cout << "Warning: some characters may fail to print on console, especially when encrypting, decrypting with a bad key or using a custom font.\n" << std::endl;
    }
    else if(asker == 'F' || asker == 'f'){
        *(FileToggle) = true;

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

        std::cout << "\n";

        write_file = freopen(input.c_str(), "a", write_file);

    }
    else{
        std::cout << "Not a valid input. Try that again." << std::endl;
        goto tag_Wto;
    }

    return 0;

}


//controls key handling if -T is specified
int key_ctrl(bool* FileToggle, std::string *key, char** argv){

        //skip if args provide key method
    if(enabled_args[ARG_KEY_FILE] || enabled_args[ARG_KEY_CON]){
        if(enabled_args[ARG_KEY_FILE]){
            *(FileToggle) = true;
            key_file = fopen(argv[file_refs[REF_KEY]], "r");
        }
        else{
            *(FileToggle) = false;
        }
        return 0;
    }

    char asker{' '};
    std::string input{""};

        //grant user choice
    tag_Rfrom:
    std::cout << "Getting key from (F)ile or (C)onsole?: ";

    getline(std::cin, input);
    std::stringstream(input) >> asker;

    if(asker == 'C' || asker == 'c'){
        tag_Rfrom_JIC:
        *(FileToggle) = false;
        std::cout << "Warning: input to console can sometimes be faulty and result in data loss/corruption during shifting." << std::endl;
    }
    else if(asker == 'F' || asker == 'f'){
        *(FileToggle) = true;

        tag_Rfrom_Fpath:
        std::cout << "Enter key filepath: ";

        getline(std::cin, input);

        key_file = fopen(input.c_str(), "r");

        if(key_file == nullptr){
            tag_Rfrom_acc_fail:
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

        std::cout << "\n";
    }
    else{
        std::cout << "Not a valid input. Try that again." << std::endl;
        goto tag_Rfrom;
    }

    return 0;

}


//i feel like this function seldom needs an introduction
int main(int argc, char** argv)
{

    std::string input{""};
    char asker{' '};
    int iasker{0};

    bool DownShift{true};
    bool Rfrom_File{true};
    bool Wto_File{true};
    bool K_File{true};

    std::string data{""};

    if(arg_ctrl(argc, argv)){
        return 1;
    }


        //checks whether the args supplied are sufficient to operate
    if( !((enabled_args[0]) || ( ( (enabled_args[1] || enabled_args[2]) && (enabled_args[3] || enabled_args[4]) && (enabled_args[5] || enabled_args[6]) ))) ){
        std::cout << "Bad usage. Insufficient arguments." << std::endl;
        return 1;
    }

    start:

        //asks for source of data
    data_I_ctrl(&Rfrom_File, &data, argv);


        //asks for the data
    if(Rfrom_File){     //if file input
        iasker = 0;
        while((iasker = fgetc(read_file) )!= EOF){
            data = data + (char)iasker;
        }
    }
    else{               //if con input
        std::cout << "Data to process: ";
        getline(std::cin, data);
        std::cout << "\n";
    }


        //asks for destination for data
    data_O_ctrl(&Wto_File, &data, argv);


        //asks for the key to use as shifting fodder
    key_ctrl(&K_File, &key, argv);


        //asks for the key
    if(K_File){     //if file input
        iasker = 0;
        while((iasker = fgetc(key_file) )!= EOF){
            key = key + (char)iasker;
        }
    }
    else{               //if con input
        std::cout << "Key to use: ";
        getline(std::cin, key);
        std::cout << "\n";
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


        //shifts the data
    for(int i = 0; i < key.length(); i++){
        global_seed += key[i];
        scramble(data, DownShift);
    }


        //returns data
    if(Wto_File){   //if file output
        if(write_file == nullptr){
            std::cout << "Error: file could not be opened for writing." << std::endl;
            return 1;
        }
        for(int i = 0; i < data.length(); i++){
            fputc(data[i], write_file);
        }
        std::cout << "Data written to file successfully." << std::endl;
    }
    else{           //if con output
        std::cout << "\nResult of shifting: " << data << std::endl;
    }


    return 0;
}
