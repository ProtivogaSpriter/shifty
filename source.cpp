#include <iostream> //for cin
#include <string>   //for basically everything? kek
#include <sstream>  //for dealing with some specific input shinanegans
#include <random>   //for the stable encryption algorithm
#include <stdio.h>  //for file control
#include <string.h> //for c-string stuff
#include <math.h>   //for some specific math

//i pre-emptively apologize for mixing together stuff from C and C++

constexpr int ASCIIctrl_const{0x00000020};                      //ascii ctrl characters

constexpr unsigned long long int ASCII_charsize_const{0x00000100};    //case ASCII, all the characters possible
long long int ASCII_actual_chars{ASCII_charsize_const - ASCIIctrl_const};   //case ASCII, total amt of chars to be used

constexpr unsigned long long int UTF16_charsize_const{0xffffffff};    //case UTF-16, all the characters possible
long long int UTF16_actual_chars{UTF16_charsize_const - ASCIIctrl_const};   //case UTF-16, total amt of chars to be used

unsigned long long int global_seed{0};  //current seed to send to engine
std::string key{""};                    //key to use as encryption fodder
std::u16string UTF16_key{u""};          //key to use as encryption fodder, UTF16 (experimental)
std::mt19937_64* engine{nullptr};       //current instance of RNG

std::string ASCII_data{""};
std::u16string UTF16_data{u""};

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


    "-o",               //determines filepath for output
    "--file-out",       //^


    "-k",               //determines filepath for key
    "--file-key",       //^
};

bool enabled_args[]{    //enabled args

    false,              //-T

    false,              //-i

    false,              //-o

    false               //-k
};

constexpr int ARG_TEXT_UI   = 0;
constexpr int ARG_IN_FILE   = 1;
constexpr int ARG_OUT_FILE  = 2;
constexpr int ARG_KEY_FILE  = 3;

constexpr int FILE_CANREAD  = 0;
constexpr int FILE_CANWRITE = 1;

//checks a file for access (cfa - check for access :3)
//  WriteRight          0 - only read   1 - can write
//  filepath            filepath to check
//  returns:            0 - success     1 - failure
int file_CFA(bool WriteRight, char* filepath){

    FILE* testfile = nullptr;

    testfile = fopen(filepath, "r");

    if(testfile == nullptr){

        if(WriteRight){

            testfile = fopen(filepath, "a");

            if(testfile == nullptr){
                return 1;
            }
            else{
                fclose(testfile);
                remove(filepath);
                return 0;
            }

        }
        else{
            return 1;
        }

    }

    fclose(testfile);

    return 0;

}


//we call this func at beginning of program to deal with args
int arg_ctrl(int argc, char** argv){

    bool allfucked = false;

    for(int i = 1; i < argc; i++){

        for(int j = 0; j < sizeof(all_args) / sizeof(all_args[0]); j++){

            int argnum = floor(j / 2);

            if(strcmp(argv[i], all_args[j]) == 0){

                if(argnum >= 1 && argnum <= 3){

                    if(i + 1 >= argc){
                        std::cout << "Argument \"" << argv[i] << "\" has no filepath specified." << std::endl;
                        return 1;
                    }

                    i++;

                    file_refs[argnum - 1] = i;

                }

                enabled_args[argnum] = true;
                goto all_good;

            }

        }

        std::cout << "Argument \"" << argv[i] << "\" is invalid." << std::endl;
        return 1;

        all_good:

    }

    //check if the arguments provided filepaths are valid.
    //if TUI is enabled, disregard and skip
    if(enabled_args[ARG_TEXT_UI] == false){

        for(int i = 0; i < sizeof(file_refs) / sizeof(file_refs[0]); i++){

            bool writeable = i == 1;

            if(file_CFA(writeable, argv[file_refs[i]]) && file_refs[i] != 0){

                allfucked = true;
                switch(i){

                    case(REF_IN):{

                        std::cout << "Failure to access input file: \"" << argv[file_refs[REF_IN]] << "\". " << std::endl;

                        break;
                    }

                    case(REF_OUT):{

                        std::cout << "Failure to access output file: \"" << argv[file_refs[REF_OUT]] << "\". " << std::endl;

                        break;
                    }

                    case(REF_KEY):{

                        std::cout << "Failure to access key file: \"" << argv[file_refs[REF_KEY]] << "\". " << std::endl;

                        break;
                    }

                }

            }

        }

    }


    if(allfucked){
        return 1;
    }

    return 0;

}


//this func controls char under- and over-flows, and shifting in general
//  offset              by how much the function shifts the character
//  data                what the original character is
//  DownShift           0 - downshift,  1 - upshift
//  SelectCharWidth     0 - ASCII,      1 - UTF16
char shift_ctrl(unsigned int offset, unsigned char data, bool DownShift, bool SelectCharWidth){

    if(data < ASCIIctrl_const){ //make sure control characters are not ever fucked with
        return data;
    }

    long long int           ACTIVE_actual_chars   = ((SelectCharWidth) ? UTF16_actual_chars     : ASCII_actual_chars);
    unsigned long long int  ACTIVE_charsize_const = ((SelectCharWidth) ? UTF16_charsize_const   : ASCII_charsize_const);

    int movement = offset % ACTIVE_actual_chars;

    if(DownShift){  //downshift
        if(movement == 0){
            return data;
        }
        else if((data - movement) < ASCIIctrl_const){
            return ACTIVE_actual_chars + (data - movement);
        }
        else{
            return data - movement;
        }
    }
    else{           //upshift
        if(movement == 0){
            return data;
        }
        else if(data + movement >= ACTIVE_charsize_const){
            return ASCIIctrl_const + ((data + movement) - ACTIVE_charsize_const);
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
        data[i] = shift_ctrl((*engine)(), data[i], DownShift, 0);
    }
    destroy_engine();

}

/*
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


//handles I/O when -T is specified
int handler_tui(int argc, char** argv){



    return 0;

}


//handles I/O when -T is not specified
int handler_simple(int argc, char** argv){

    std::string input{""};
    char asker{' '};
    int iasker{0};

    bool DownShift{true};
    bool Rfrom_File{true};
    bool Wto_File{true};
    bool K_File{true};

    std::string data{""};

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
    std::cout << "Up/down shifting? (U/d): ";

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
        std::cout << "\nResult of shifting: \n" << data << std::endl;
    }

    return 0;

}
*/

//i feel like this function seldom needs an introduction
int main(int argc, char** argv)
{

    if(arg_ctrl(argc, argv)){
        std::cout << "Bad usage." << std::endl;
        return 1;
    }

    //handler_simple(argc, argv);

    return 0;
}
