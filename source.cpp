#include <iostream>     //for cin
#include <string>       //for basically everything? kek
#include <sstream>      //for dealing with some specific input shinanegans
#include <fstream>      //for file handling
#include <random>       //for the stable encryption algorithm
#include <filesystem>   //for filepaths
#include <cuchar>       //for narrow to multibyte conversion

#define DEBUGMODE 0 //we switch this to 1 if we want extra debug output


int selected_tstd = 1;

constexpr int TSTD_ASCII    = 0;
constexpr int TSTD_UTF8     = 1;
constexpr int TSTD_UTF16    = 2;

std::string all_tstds[]{

    "ASCII",

    "UTF8",

    "UTF16"

};


#if defined (_WIN32)

#include <conio.h>
#include <windows.h>

void virtualise(void){

    DWORD consoleMode;
    HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode( handleOut , &consoleMode);

    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode( handleOut , consoleMode );

    return;
}

void devirtualise(void){

    DWORD consoleMode;
    DWORD bufferMode;
    HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode( handleOut , &consoleMode);
    bufferMode = ~ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    consoleMode = consoleMode & bufferMode;
    SetConsoleMode( handleOut , consoleMode );

    return;
}



#endif // defined


constexpr   int              ASCIIctrl_const        {0x00000020};                               //ASCII ctrl characters

constexpr   long long int    ASCII_charsize_const   {0x00000100};                               //case ASCII, all the characters possible
            long long int    ASCII_actual_chars     {ASCII_charsize_const - ASCIIctrl_const};   //case ASCII, total amt of chars to be used

constexpr   long long int    UTF8_charsize_const    {0x00110000};                               //case UTF-8, all the characters possible
            long long int    UTF8_actual_chars      {UTF8_charsize_const - ASCIIctrl_const};    //case UTF-8, total amt of chars to be used

constexpr   long long int    UTF16_surrogate_low    {0x0000d800};                               //low-end surrogate delimeter in UTF-16
constexpr   long long int    UTF16_surrogate_high   {0x0000dfff};                               //high-end surrogate delimeter in UTF-16


unsigned long long int  global_seed{0};     //current seed to send to engine
std::u32string          key{U""};           //key to use as encryption fodder
std::mt19937_64*        engine{nullptr};    //current instance of RNG


std::fstream   file_I;      //file to read from
std::fstream   file_O;      //file to write to
std::fstream   file_K;      //file to get key from

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
constexpr int ARG_TEXT_STD  = 7;

std::string all_args[]{       //all args to look at

    "-T",               //enables full text user interface
    "--text-ui",        //^



    "-i",               //determines filepath for input
    "--file-in",        //^

    "-o",               //determines filepath for output
    "--file-out",       //^

    "-k",               //determines filepath for key
    "--file-key",       //^



    "-A",               //allows the program to ask for specific input (file/console I/O, text standard)
    "--allow-asking",   //^



    "-U",               //declares an upshift
    "--up",             //^

    "-D",               //declares a downshift
    "--down",           //^



    "-s",               //determines text standard (ascii, UTF8, UTF16, UTF32)
    "--text-standard"   //^

};

bool enabled_args[]{    //enabled args

    false,              //-T


    false,              //-i

    false,              //-o

    false,              //-k


    false,              //-A


    false,              //-U

    false,              //-D


    false               //-s

};


constexpr int FILE_CANREAD  = 0;
constexpr int FILE_CANWRITE = 1;

//checks a file for access (cfa - check for access :3)
//  WriteRight          0 - only read   1 - can write
//  filepath            filepath to check
//  returns:            0 - success     1 - failure
int file_CFA(bool WriteRight, std::u32string filepath_src){

    std::filesystem::path filepath = filepath_src;

    std::fstream testfile;

    testfile.open(filepath, std::ios::in);

    if(!testfile.is_open()){

        if(WriteRight){

            testfile.open(filepath, std::ios::out);

            if(!testfile.is_open()){

                return 1;

            }
            else{

                testfile.close();
                return 0;

            }

        }
        else{

            return 1;

        }

    }
    else{

        testfile.close();
        return 0;

    }

}

int file_CFA(bool WriteRight, std::string filepath_src){

    std::filesystem::path filepath = filepath_src;

    std::fstream testfile;

    testfile.open(filepath, std::ios::in);

    if(!testfile.is_open()){

        if(WriteRight){

            testfile.open(filepath, std::ios::out);

            if(!testfile.is_open()){

                return 1;

            }
            else{

                testfile.close();
                return 0;

            }

        }
        else{

            return 1;

        }

    }
    else{

        testfile.close();
        return 0;

    }

}


//call this at beginning of program to deal with args
int arg_ctrl(int argc, std::string* argv){

    bool allfucked = false;

    for(int i = 1; i < argc; i++){

        for(int j = 0; j < sizeof(all_args) / sizeof(all_args[0]); j++){

            int argnum = floor(j / 2);

            if(argv[i] == all_args[j]){

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

                case(ARG_TEXT_STD):{

                    if(enabled_args[ARG_TEXT_STD]){
                        std::cout << "Argument\"" << argv[i] << "\" cannot be specified more than once." << std::endl;
                        return 1;
                    }
                    break;

                }

                }

                if(argnum == ARG_IN_FILE || argnum == ARG_OUT_FILE || argnum == ARG_KEY_FILE || argnum == ARG_TEXT_STD){

                    if(i + 1 >= argc){
                        std::cout << "Argument \"" << argv[i] << "\" has no filepath specified." << std::endl;
                        return 1;
                    }

                    i++;

                    if(argnum == ARG_TEXT_STD){

                        //for parsing text standard
                        for(int k = 0; k < argv[i].length(); k++){

                            //magic numbers declaring lowercase letters in ASCII
                            if((argv[i])[k] > 96 && (argv[i])[k] < 123){
                                (argv[i])[k] -= 32; //uppercases them if they're lowercase
                            }

                        }

                        for(int k = 0; k < sizeof(all_tstds) / sizeof(all_tstds[0]); k++){

                            if(argv[i] == all_tstds[k]){
                                selected_tstd = k;
                                if(DEBUGMODE){
                                    std::cout << "Declared text standard: " << all_tstds[k] << std::endl;
                                }
                                goto tstd_all_good;
                            }
                        }
                        std::cout << "Text standard \"" << argv[i] << "\" not a valid text standard. Try 'ASCII', 'UTF8' or 'UTF16'." << std::endl;
                        return 1;

                        tstd_all_good:

                        ;   //mingw my beloathed

                    }
                    else{

                        //for file references
                        file_refs[argnum - 1] = i;

                    }

                }

                enabled_args[argnum] = true;
                goto all_good;

            }

        }

        std::cout << "Argument \"" << argv[i] << "\" is invalid." << std::endl;
        return 1;

        all_good:

        ;  //god i hate mingw so much it's abhorrent

    }

    //check if the arguments provided filepaths are valid.
    //if TUI is enabled, disregard and skip
    if(enabled_args[ARG_TEXT_UI] == false){

        for(int i = 0; i < sizeof(file_refs) / sizeof(file_refs[0]); i++){

            bool writeable = i == 1;

            if(DEBUGMODE){
                std::cout << "writable considered " << writeable << " for " << i << std::endl;
            }

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

    if(DEBUGMODE){
        std::cout << "Arg handling successful." << std::endl;
    }

    return 0;

}


std::u32string uni_str_parse(std::string* indata){

    std::u32string outdata;

    char32_t uchr;
    const char* indata_ref = (*indata).c_str();
    const char* indata_end = (*indata).c_str() + (*indata).size() + 1;
    std::mbstate_t state{};

    if(DEBUGMODE){
    std::cout << "Bytesize of outdata: " << std::dec << indata_end - indata_ref << std::endl;
    }

    while(std::size_t check = std::mbrtoc32(&uchr, indata_ref, indata_end - indata_ref, &state)){
        if (check == (std::size_t) - 1){
            if(DEBUGMODE){
                std::cout << "Error. mbrtoc32 returned -1. The data may be corrupt." << std::endl;
            }
            break;
        }
        if (check == (std::size_t) - 2){
            if(DEBUGMODE){
                std::cout << "Error. mbrtoc32 returned -2. The data is incomplete." << std::endl;
            }
            break;
        }

        if(DEBUGMODE){
            std::cout << std::dec << check << " bytes [ ";
            for (std::size_t n = 0; n < check; ++n)
                std::cout << std::hex << +static_cast<unsigned char>(indata_ref[n]) << ' ';
            std::cout << "]\n";
        }

        outdata += uchr;

        indata_ref += check;
    }

    if(DEBUGMODE){
    std::cout << "Charsize of outdata: " << std::dec << outdata.length() << std::endl;

    for(int i = 0; i < outdata.length(); i++){
        std::cout << std::hex << (int)outdata[i] << std::endl;
    }
    }

    return outdata;

}

std::string nrw_str_parse(std::u32string* indata){

    std::string outdata;

    char nchr[16];
    std::mbstate_t state{};

    if(DEBUGMODE){
        std::cout << std::dec << "indata elements, " << (*indata).length() << " length:" << std::endl;
        for(int i = 0; i < (*indata).length(); i++){
            std::cout << std::hex << (int)(*indata)[i] << std::endl;
        }
    }
    std::cout << std::dec << std::endl;


    for(int i = 0; i < (*indata).length(); i++){

        if(DEBUGMODE){
            std::cout << std::hex << (int)(*indata)[i] << std::endl;
        }
        std::size_t check = c32rtomb(nchr, (*indata)[i], &state);

        if(check == (std::size_t) - 1){
            if(DEBUGMODE){
                std::cout << "Error. c32rtomb returned -1. The data was corrupted in processing." << std::endl;
            }
            break;
        }


        if(DEBUGMODE){
        std::cout << check << " bytes [ ";
        }
        for(int j = 0; j < check; j++){
            if(DEBUGMODE){
                std::cout << std::hex << (short)nchr[j] << " ";
            }
            outdata += nchr[j];
        }
        if(DEBUGMODE){
        std::cout << "]" << std::endl;
        }

    }

    return outdata;

}


//controls char under- and over-flows, and shifting in general
//  offset              by how much the function shifts the character
//  data                what the original character is
//  Direction           0 - downshift,  1 - upshift
//  returns:            char modified by offset
char32_t shift_ctrl(unsigned int offset, char32_t data, bool Direction){

    if(data < ASCIIctrl_const){ //make sure control characters are not ever fucked with
        return data;
    }

    long long int           char_equivalent       = data;

    long long int           ACTIVE_actual_chars   = ((selected_tstd > 0) ? UTF8_actual_chars     : ASCII_actual_chars);
    unsigned long long int  ACTIVE_charsize_const = ((selected_tstd > 0) ? UTF8_charsize_const   : ASCII_charsize_const);

    int movement = offset % ACTIVE_actual_chars;

    if(DEBUGMODE){
        std::cout << std::dec << ACTIVE_charsize_const << " " << ACTIVE_actual_chars << " " << movement << " " << char_equivalent << std::endl;
    }

    if(Direction){              //upshift
        if(movement == 0){
            return (char32_t)char_equivalent;
        }
        else if(char_equivalent + movement >= ACTIVE_charsize_const){
            if(ASCIIctrl_const + ((char_equivalent + movement) - ACTIVE_charsize_const) > ACTIVE_charsize_const ){
                std::cout << "Overflow error. Case U1." << std::endl;;
            }
            return (char32_t)(ASCIIctrl_const + ((char_equivalent + movement) - ACTIVE_charsize_const));
        }
        else{
            if(char_equivalent + movement > ACTIVE_charsize_const ){
                std::cout << "Overflow error. Case U2.";
            }
            return (char32_t)(char_equivalent + movement);
        }
    }
    else{                       //downshift
        if(movement == 0){
            return (char32_t)char_equivalent;
        }
        else if((char_equivalent - movement) < ASCIIctrl_const){
            if(ACTIVE_actual_chars + (char_equivalent - movement) < ASCIIctrl_const ){
                std::cout << "Overflow error. Case D1.";
            }
            return (char32_t)(ACTIVE_actual_chars + (char_equivalent - movement));
        }
        else{
            if(char_equivalent - movement < ASCIIctrl_const ){
                std::cout << "Overflow error. Case D2.";
            }
            return (char32_t)(char_equivalent - movement);
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
void scramble(std::u32string& data, bool Direction){

    create_engine();
    for(int i = 0; i < data.length(); i++){
        data[i] = shift_ctrl((*engine)(), data[i], Direction);
    }
    destroy_engine();

}

void run_shifter(std::u32string& data, int direction){

    for(int i = 0; i < key.length(); i++){
        global_seed += key[i];
        scramble(data, direction);
    }

}


std::u32string tstd_ntu_formatter(std::string indata){

    std::u32string outdata = U"";

    if(selected_tstd > 0){      //anything Unicode

        outdata = uni_str_parse(&indata);

    }
    else{                       //ASCII

        for(int i = 0; i < indata.length(); i++){
            outdata += +static_cast<unsigned char>(indata[i]);
        }

    }

    return outdata;

}

std::string tstd_utn_formatter(std::u32string indata){

    std::string outdata = "";

    if(selected_tstd > 0){      //anything Unicode

        outdata = nrw_str_parse(&indata);

    }
    else{                       //ASCII

        for(int i = 0; i < indata.length(); i++){
            outdata += (char)indata[i];
        }

    }

    return outdata;

}


int basic_T_ctrl(bool Asking){

    std::string     input{""};
    char            inchr{32};

    if(enabled_args[ARG_TEXT_STD]){

        return 0;

    }

    if(Asking){

        retry_asking_start:

            std::cout << "Text standard to use?   ";
            std::getline(std::cin, input);

            for(int i = 0; i < input.length(); i++){

                //magic numbers declaring lowercase letters in ASCII
                if(input[i] > 96 && input[i] < 123){
                    input[i] -= 32; //uppercases them if they're lowercase
                }

            }

            if(DEBUGMODE){
                std::cout << input << std::endl;
            }

            if      (input == "ASCII"){
                selected_tstd = TSTD_ASCII;
            }
            else if (input == "UTF8"   || input == "UTF-8"){
                selected_tstd = TSTD_UTF8;
            }
            else if (input == "UTF16"  || input == "UTF-16"){
                selected_tstd = TSTD_UTF16;
            }
            else{
                std::cout << "Incorrect text standard. Try ASCII, UTF8 or UTF16." << std::endl;
                input = "";
                goto retry_asking_start;
            }

    }

    return 0;
}


//controls input destination
//  argv                arguments passed
//  Asking              0 - not asking  1 - asking
//  returns:            0 - success     1 - failure
int basic_I_ctrl(std::string* argv, bool Asking){

    std::string     input{""};

    if(file_refs[REF_IN] > -1){

        file_I.open(argv[file_refs[REF_IN]], std::ios::in);
        return 0;

    }


    retry:

    std::cout << "Enter Input filepath:   ";
    std::getline(std::cin, input);

    if(file_CFA(0, input)){
        std::cout << "Filepath \"" << input << "\" cannot be accessed. Try again." << std::endl;
        goto retry;
    }

    file_I.open(input, std::ios::in);

    return 0;

}


//controls output destination
//  argv                arguments passed
//  Asking              0 - not asking  1 - asking
//  returns:            0 - success     1 - failure
int basic_O_ctrl(std::string* argv, bool Asking){

    std::string     input{""};
    char            inchr{32};

    if(file_refs[REF_OUT] > -1){

        file_O.open(argv[file_refs[REF_OUT]], std::ios::out);
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

            if(file_CFA(1, input)){

                std::cout << "Filepath \"" << input << "\" cannot be accessed. Try again." << std::endl;
                goto retry_asking_file;

            }

            file_O.open(input, std::ios::out);
            return 0;

        }
        else if(inchr == 'c' || inchr == 'C'){

            if(selected_tstd > 0){
            std::cout << "\033[48;2;255;0;0;38;2;0;0;0mWARNING: Output to console with Unicode may appear illegible, especially when encoding or decoding with a bad key.\033[0m\n\033[48;2;255;0;0;38;2;0;0;0mAre you sure?\033[0m (Y/n): ";

            std::getline(std::cin, input);
            std::stringstream(input) >> inchr;

            if(inchr != 'y' && inchr != 'Y'){
                goto retry_asking_start;
            }

            }

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

        if(file_CFA(0, input)){

            std::cout << "Filepath \"" << input << "\" cannot be accessed. Try again." << std::endl;
            goto retry_simple;

        }

        file_O.open(input, std::ios::out);

        return 0;

    }

}


//controls key source destination
//  argv                arguments passed
//  Asking              0 - not asking  1 - asking
//  returns:            0 - success     1 - failure
int basic_K_ctrl(std::string* argv, bool Asking){

    std::string     input{""};
    char            inchr{32};

    if(file_refs[REF_KEY] > -1){

        file_K.open(argv[file_refs[REF_KEY]], std::ios::in);
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

            if(file_CFA(0, input)){
                std::cout << "Filepath \"" << input << "\" cannot be accessed. Try again." << std::endl;
                goto retry_asking_file;
            }

            file_K.open(input, std::ios::in);
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

    std::string     input{""};
    char            inchr{32};
    short           direction = enabled_args[ARG_SHIFT_UP] - enabled_args[ARG_SHIFT_DN];

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
            std::cout << "Error: Shift direction determined to be contradictory." << std::endl;
            exit(1);
        }

        }
    }

}

//handles I/O when -T is not specified
int handler_simple(int argc, std::string* argv){

    std::u32string      data        {U""};
    std::string         input       {""};
    char                inchr       {32};
    int                 fread       {0};
    bool                Direction   {0};    //0 - down, 1 - up

    //handles the text standard
    basic_T_ctrl(enabled_args[ARG_ALLOWASK]);

    if(DEBUGMODE){

        std::cout << "DEBUGMODE DIRECT INPUT?:";
        std::getline(std::cin, input);
        std::stringstream(input) >> inchr;

        switch(inchr){

        case('Y'):
        case('y'):{
            int counter = 0;
            std::cout << "Enter direct input size:";
            std::getline(std::cin, input);
            std::stringstream(input) >> counter;
            data = U"";
            for(int i = 0; i < counter; i++){
                std::cout << "ELEMENT " << i << ": ";
                std::getline(std::cin, input);
                std::stringstream(input) >> fread;
                data += fread;
            }
            fread = 0;
            goto avoid_I_ctrl;
        }

        default:{

        }


        }

    }


    //handles input destination
    basic_I_ctrl(argv, enabled_args[ARG_ALLOWASK]);


    //input getter
    if(file_I.is_open()){   //file I

        fread = 0;
        input = "";
        while((fread = file_I.get()) && !file_I.eof()){
            input += (char)fread;
        }

        file_I.clear();
        file_I.seekg(0);

        data = tstd_ntu_formatter(input);

        input = "";

    }
    else{
        std::cout << "Error: failure to access input file.";
        return 1;
    }


    avoid_I_ctrl:

    ;   //mingw you utter bitch


    if(DEBUGMODE){
        for(int i = 0; i < data.length(); i++){
            std::cout << std::dec << (long int)data[i] << " " << std::hex << (long int)data[i] << std::endl;
        }
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
    if(file_K.is_open()){   //file K

        fread = 0;
        input = "";
        while((fread = file_K.get()) && !file_K.eof()){
                input += (char)fread;
        }
        file_K.clear();
        file_K.seekg(0);

        key = tstd_ntu_formatter(input);

        input = "";

    }
    else{                   //con K
        std::cout << "Enter key:              ";
        std::getline(std::cin, input);
        key = uni_str_parse(&input);
    }

    if(!enabled_args[ARG_KEY_FILE]){
        std::cout << "\n";
    }


    //direction getter
    Direction = basic_D_ctrl();

    if(!enabled_args[ARG_SHIFT_UP] && !enabled_args[ARG_SHIFT_DN]){
        std::cout << "\n";
    }

    run_shifter(data, Direction);


    //output
    if(file_O.is_open()){   //file O

        input = tstd_utn_formatter(data);
        for(int i = 0; i < input.length(); i++){
            file_O.put(input[i]);
        }
        input = "";

    }
    else{                   //con O

        input = tstd_utn_formatter(data);
        std::cout << "Result of shifting:" << std::endl;
        #if defined (_WIN32)
        devirtualise();
        #endif
        for(int i = 0; i < data.length(); i++){
            std::cout << (char)data[i];
        }
        #if defined (_WIN32)
        virtualise();
        #endif
        std::cout << "\nEnd of data." << std::endl;
    }

    if(DEBUGMODE){

        std::cout << "\n";
        for(int i = 0; i < input.length(); i++){
            std::cout << std::dec << (short)input[i] << std::endl;
        }

    }


    return 0;

}


//handles I/O when -T is specified
int handler_tui(int argc, std::string* argv){

    //TBD!!!!

    std::cout << "The TUI mode is not yet implemented. Sorry! >_> " << std::endl;

    return 0;

}


//i feel like this function seldom needs an introduction
int main(int argc, char** argv)
{

    #if defined (_WIN32)
    virtualise();
    #endif

    std::string* argv_str;
    argv_str = new std::string[argc];

    int* argsize;
    argsize = new int[argc];

    char reader;

    for(int i = 0; i < argc; i++){
        argsize[i] = 0;
    }

    for(int i = 0; i < argc; i++){
        reader = 32;
        while(reader != 0){
            argsize[i]++;
            reader = *(*(argv + i) + argsize[i]);
        }
    }

    for(int i = 0; i < argc; i++){
        argv_str[i] = "";
        for(int j = 0; j < argsize[i]; j++){
            argv_str[i] += *(*(argv + i) + j);  //lord forgive me for i have sinned
        }
        if(DEBUGMODE){
            std::cout << "arg " << i << " " << argv[i] << " read as " << argv_str[i] << std::endl;
        }
    }

    if(arg_ctrl(argc, argv_str)){
        std::cout << "Bad usage." << std::endl;
        return 1;
    }


    if(enabled_args[ARG_TEXT_UI]){

        if(DEBUGMODE){
            std::cout << "TUI enabled." << std::endl;
        }

        return handler_tui(argc, argv_str);

    }
    else{

        if(DEBUGMODE){
            std::cout << "Simple mode enabled." << std::endl;
        }

        return handler_simple(argc, argv_str);

    }

}
