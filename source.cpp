#include <iostream> //for cin
#include <string>   //for basically everything? kek
#include <sstream>  //for dealing with some specific input shinanegans
#include <random>   //for the stable encryption algorithm
#include <stdio.h>  //for file control
#include <string.h> //for c-string stuff
#include <math.h>   //for some specific math

//i pre-emptively apologize for mixing together stuff from C and C++

constexpr int ASCIIctrl_const{0x00000020};                      //ascii ctrl characters

constexpr unsigned long long int ASCII_charsize_const{0x00000100};          //case ASCII, all the characters possible
long long int ASCII_actual_chars{ASCII_charsize_const - ASCIIctrl_const};   //case ASCII, total amt of chars to be used

constexpr unsigned long long int UTF16_charsize_const{0xffffffff};          //case UTF-16, all the characters possible
long long int UTF16_actual_chars{UTF16_charsize_const - ASCIIctrl_const};   //case UTF-16, total amt of chars to be used

unsigned long long int global_seed{0};  //current seed to send to engine
std::string key{""};                    //key to use as encryption fodder
std::u16string UTF16_key{u""};          //key to use as encryption fodder, UTF16 (experimental)
std::mt19937_64* engine{nullptr};       //current instance of RNG

std::string ASCII_data{""};
std::u16string UTF16_data{u""};


FILE *file_I{nullptr};      //file to read from
FILE *file_O{nullptr};      //file to write to
FILE *file_K{nullptr};      //file to get key from

constexpr int REF_IN    = 0;
constexpr int REF_OUT   = 1;
constexpr int REF_KEY   = 2;

int file_refs[]{        //positions of filepaths in argv

    -1,                 //file_I
    -1,                 //file_O
    -1                  //file_K

};


constexpr int ARG_TEXT_UI   = 0;
constexpr int ARG_IN_FILE   = 1;
constexpr int ARG_OUT_FILE  = 2;
constexpr int ARG_KEY_FILE  = 3;
constexpr int ARG_ALLOWASK  = 4;
constexpr int ARG_SHIFT_UP  = 5;
constexpr int ARG_SHIFT_DN  = 6;

char *all_args[]{       //all args to look at

    "-T",               //enables full text user interface
    "--text-ui",        //^


    "-i",               //determines filepath for input
    "--file-in",        //^

    "-o",               //determines filepath for output
    "--file-out",       //^

    "-k",               //determines filepath for key
    "--file-key",       //^


    "-A",               //allows the program to ask for specific input (file/console I/O)
    "--allow-asking",   //^


    "-U",               //declares an upshift
    "--up",             //^

    "-D",               //declares a downshift
    "--down"            //^

};

bool enabled_args[]{    //enabled args

    false,              //-T

    false,              //-i

    false,              //-o

    false,              //-k

    false,              //-A

    false,              //-U

    false,              //-D

};


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

int file_CFA(bool WriteRight, std::string* filepath){

    FILE* testfile = nullptr;

    testfile = fopen((*filepath).c_str(), "r");

    if(testfile == nullptr){

        if(WriteRight){

            testfile = fopen((*filepath).c_str(), "a");

            if(testfile == nullptr){
                return 1;
            }
            else{
                fclose(testfile);
                remove((*filepath).c_str());
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


//call this at beginning of program to deal with args
int arg_ctrl(int argc, char** argv){

    bool allfucked = false;

    for(int i = 1; i < argc; i++){

        for(int j = 0; j < sizeof(all_args) / sizeof(all_args[0]); j++){

            int argnum = floor(j / 2);

            if(strcmp(argv[i], all_args[j]) == 0){

                switch(argnum){

                case(ARG_SHIFT_UP):{

                    if(enabled_args[ARG_SHIFT_DN]){
                        std::cout << "Argument \"" << argv[i] << "\" contradicts argument -U." << std::endl;
                        return 1;
                    }
                    break;
                }

                case(ARG_SHIFT_DN):{

                    if(enabled_args[ARG_SHIFT_UP]){
                        std::cout << "Argument \"" << argv[i] << "\" contradicts argument -D." << std::endl;
                        return 1;
                    }
                    break;
                }

                }

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

            //this is really fucking dangerous. if the conditions are reversed, a bad filepath results in a segfault
            if(file_refs[i] > -1 && file_CFA(writeable, argv[file_refs[i]])){

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


//controls char under- and over-flows, and shifting in general
//  offset              by how much the function shifts the character
//  data                what the original character is
//  DownShift           0 - downshift,  1 - upshift
//  SelectCharWidth     0 - ASCII,      1 - UTF16
//  returns:            char modified by offset
char shift_ctrl(unsigned int offset, unsigned char data, bool Direction, bool SelectCharWidth){

    if(data < ASCIIctrl_const){ //make sure control characters are not ever fucked with
        return data;
    }

    long long int           ACTIVE_actual_chars   = ((SelectCharWidth) ? UTF16_actual_chars     : ASCII_actual_chars);
    unsigned long long int  ACTIVE_charsize_const = ((SelectCharWidth) ? UTF16_charsize_const   : ASCII_charsize_const);

    int movement = offset % ACTIVE_actual_chars;

    if(Direction){              //upshift
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
    else{                       //downshift
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

}


//creates an RNG with the current global_seed
void create_engine(void){

    if(engine != nullptr){
        std::cout << "Engine already exists. Aborting." << std::endl;
        exit(1);
    }

    engine = new std::mt19937_64;
    engine->seed(global_seed);

}

//destroys the current RNG
void destroy_engine(void){

    delete engine;
    engine = nullptr;

}

//creates an RNG, uses it to generate a series of offsets which are appended to or subtracted from characters in data, destroys the RNG
void scramble(std::string& data, bool Direction){

    create_engine();
    for(int i = 0; i < data.length(); i++){
        data[i] = shift_ctrl((*engine)(), data[i], Direction, 0);
    }
    destroy_engine();

}

void run_shifter(std::string& data, int direction){

    for(int i = 0; i < key.length(); i++){
        global_seed += key[i];
        scramble(data, direction);
    }

}


//controls input destination
//  argv                arguments passed
//  Asking              0 - not asking  1 - asking
//  returns:            0 - success     1 - failure
int basic_I_ctrl(char** argv, bool Asking){

    std::string input{""};
    char        inchr{32};

    if(file_refs[REF_IN] > -1){

        file_I = fopen(argv[file_refs[REF_IN]], "r");
        return 0;

    }

    if(Asking){

        retry_asking_start:

        std::cout << "Input by (F)ile/(C)on?  ";
        std::getline(std::cin, input);
        std::stringstream(input) >> inchr;

        if(inchr == 'f' || inchr == 'F'){

            retry_asking_file:
            std::cout << "Enter Input filepath:   ";
            std::getline(std::cin, input);

            if(file_CFA(0, &input)){

                std::cout << "Filepath \"" << input << "\" cannot be accessed. Try again." << std::endl;
                goto retry_asking_file;

            }

            file_I = fopen(input.c_str(), "r");
            return 0;

        }
        else if(inchr == 'c' || inchr == 'C'){

            //we just go back lol everything is set for this to work already
            return 0;

        }
        else{

            std::cout << "Invalid input. Try again." << std::endl;
            goto retry_asking_start;

        }

    }
    else{

        retry_simple:

        std::cout << "Enter Input filepath:   ";
        std::getline(std::cin, input);

        if(file_CFA(0, &input)){

            std::cout << "Filepath \"" << input << "\" cannot be accessed. Try again." << std::endl;
            goto retry_simple;

        }

        file_I = fopen(input.c_str(), "r");

        return 0;

    }

}


//controls output destination
//  argv                arguments passed
//  Asking              0 - not asking  1 - asking
//  returns:            0 - success     1 - failure
int basic_O_ctrl(char** argv, bool Asking){

    std::string input{""};
    char        inchr{32};

    if(file_refs[REF_OUT] > -1){

        file_O = fopen(argv[file_refs[REF_OUT]], "w");
        return 0;

    }

    if(Asking){

        retry_asking_start:

        std::cout << "Output by (F)ile/(C)on? ";
        std::getline(std::cin, input);
        std::stringstream(input) >> inchr;

        if(inchr == 'f' || inchr == 'F'){

            retry_asking_file:
            std::cout << "Enter Output filepath:  ";
            std::getline(std::cin, input);

            if(file_CFA(0, &input)){

                std::cout << "Filepath \"" << input << "\" cannot be accessed. Try again." << std::endl;
                goto retry_asking_file;

            }

            file_O = fopen(input.c_str(), "w");
            return 0;

        }
        else if(inchr == 'c' || inchr == 'C'){

            //we just go back lol everything is set for this to work already
            return 0;

        }
        else{

            std::cout << "Invalid input. Try again." << std::endl;
            goto retry_asking_start;

        }

    }
    else{

        retry_simple:

        std::cout << "Enter Output filepath:  ";
        std::getline(std::cin, input);

        if(file_CFA(0, &input)){

            std::cout << "Filepath \"" << input << "\" cannot be accessed. Try again." << std::endl;
            goto retry_simple;

        }

        file_O = fopen(input.c_str(), "r");

        return 0;

    }

}


//controls key source destination
//  argv                arguments passed
//  Asking              0 - not asking  1 - asking
//  returns:            0 - success     1 - failure
int basic_K_ctrl(char** argv, bool Asking){

    std::string input{""};
    char        inchr{32};

    if(file_refs[REF_KEY] > -1){

        file_K = fopen(argv[file_refs[REF_KEY]], "r");
        return 0;

    }

    if(Asking){

        retry_asking_start:

        std::cout << "Key by (F)ile/(C)on?    ";
        std::getline(std::cin, input);
        std::stringstream(input) >> inchr;

        if(inchr == 'f' || inchr == 'F'){

            retry_asking_file:

            std::cout << "Enter key filepath:     ";
            std::getline(std::cin, input);

            if(file_CFA(0, &input)){
                std::cout << "Filepath \"" << input << "\" cannot be accessed. Try again." << std::endl;
                goto retry_asking_file;
            }

            file_K = fopen(input.c_str(), "r");
            return 0;

        }
        else if(inchr == 'c' || inchr == 'C'){

            //we just go back lol everything is set for this to work already
            return 0;

        }
        else{

            std::cout << "Invalid input. Try again." << std::endl;
            goto retry_asking_start;

        }

    }
    else{

        return 0;

    }

}


bool basic_D_ctrl(void){

    std::string input{""};
    char        inchr{32};
    short       direction = enabled_args[ARG_SHIFT_UP] - enabled_args[ARG_SHIFT_DN];

    if(!direction){

        shift_dir:
        std::cout << "Shift direction (U/d):  ";
        std::getline(std::cin, input);
        std::stringstream(input) >> inchr;

        if(inchr == 'u' || inchr == 'U'){

            return 1;

        }
        else if(inchr == 'd' || inchr == 'D'){

            return 0;

        }
        else{

            std::cout << "Invalid input. Try again." << std::endl;
            goto shift_dir;

        }

    }
    else{
        switch(direction){

        case(-1):{
            return 0;
        }

        case(1):{
            return 1;
        }

        default:{
            exit(1);
        }

        }
    }

}


//handles I/O when -T is not specified
int handler_simple(int argc, char** argv){

    std::string data        {""};
    std::string input       {""};
    char        inchr       {32};
    int         fread       {0};
    bool        Direction   {0};    //0 - down, 1 - up


    //handles input destination
    basic_I_ctrl(argv, enabled_args[ARG_ALLOWASK]);


    //input getter
    if(file_I != nullptr){  //file I

        fread = 0;
        while((fread = fgetc(file_I)) != EOF){
            data = data + (char)fread;
        }

    }
    else{                   //con I

        std::cout << "Enter data:             ";
        std::getline(std::cin, data);

    }

    if(!enabled_args[ARG_IN_FILE]){
        std::cout << "\n";
    }


    //handles output destination
    basic_O_ctrl(argv, enabled_args[ARG_ALLOWASK]);

    if(!enabled_args[ARG_OUT_FILE]){
        std::cout << "\n";
    }


    //handles key destination
    basic_K_ctrl(argv, enabled_args[ARG_ALLOWASK]);


    //key getter
    if(file_K != nullptr){  //file K

        fread = 0;
        while((fread = fgetc(file_K)) != EOF){
            key = key + (char)fread;
        }

    }
    else{                   //con K
        std::cout << "Enter key:              ";
        std::getline(std::cin, key);
    }

    if(!enabled_args[ARG_KEY_FILE]){
        std::cout << "\n";
    }


    //direction getter
    Direction = basic_D_ctrl();

    if(!enabled_args[ARG_SHIFT_UP] && !enabled_args[ARG_SHIFT_DN]){
        std::cout << "\n";
    }


    //shifts
    run_shifter(data, Direction);


    //output
    if(file_O != nullptr){  //file O

        for(int i = 0; i < data.length(); i++){
            fputc(data[i], file_O);
        }

    }
    else{                   //con O
        std::cout << "Result of shifting:" << std::endl;
        for(int i = 0; i < data.length(); i++){
            std::cout << data[i];
        }
        std::cout << "\nEnd of data." << std::endl;
    }


    return 0;

}


//handles I/O when -T is specified
int handler_tui(int argc, char** argv){

    //TBD!!!!

    std::cout << "The TUI mode is not yet implemented. Sorry! >_> " << std::endl;

    return 0;

}


//i feel like this function seldom needs an introduction
int main(int argc, char** argv)
{

    if(arg_ctrl(argc, argv)){
        std::cout << "Bad usage." << std::endl;
        return 1;
    }

    if(enabled_args[ARG_TEXT_UI]){

        return handler_tui(argc, argv);

    }
    else{

        return handler_simple(argc, argv);

    }

}
