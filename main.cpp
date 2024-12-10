#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include "logger.hpp"

using namespace std;

#define READ 0
#define WRITE 1
#define BUFFER_SIZE 1024

Logger logger;

void create_named_pipes(vector<string> parts, vector<string> stores) {
    for (int i = 0; i < stores.size(); i++) {
        for (int j = 0; j < parts.size(); j++) {
            string pipe_store_to_part = stores[i] + "," + parts[j];
            mkfifo(pipe_store_to_part.c_str(), 0666);
            logger.log("INFO", "named pipe " + pipe_store_to_part + " has been created", "main");

        }
    }
}

void delete_named_pipes(vector<string> parts, vector<string> stores) {

    for (int i = 0; i < stores.size(); i++) {
        for (int j = 0; j < parts.size(); j++) {
            string pipe_store_to_part = stores[i] + "," + parts[j];
            unlink(pipe_store_to_part.c_str());
            logger.log("INFO", "named pipe " + pipe_store_to_part + " has been unlinked", "main");
        }
    }
}

string convert_vector_to_string(vector<string> words, char delimiter){
    string result = "";
    for (size_t i = 0; i < words.size(); ++i) { 
        result += words[i]; 
        if (i < words.size() - 1) {
            result += delimiter; 
        } 
    }  
    return result;
}


void create_process(string executed, int &_pid, int &write_fd, int &read_fd){

    int pipe1_fd[2];
    int pipe2_fd[2];

    if (pipe(pipe1_fd) == -1 || pipe(pipe2_fd) == -1) {
        logger.log("ERROR", "Pipe creation failed", "main");
        exit(0);
    }

    logger.log("INFO", "Pipe creation succeed", "main");

    int pid = fork();
    if (pid < 0) {
        logger.log("ERROR", "Fork failed", "main");
        exit(0);
    }

    if (pid == 0) {
        close(pipe1_fd[WRITE]);
        close(pipe2_fd[READ]);

        char pipe1_read_fd[10];
        char pipe2_write_fd[10];
        snprintf(pipe1_read_fd, sizeof(pipe1_read_fd), "%d", pipe1_fd[READ]);
        snprintf(pipe2_write_fd, sizeof(pipe2_write_fd), "%d", pipe2_fd[WRITE]);

        execl(executed.c_str(), pipe1_read_fd, pipe2_write_fd, NULL);

    } else {
        close(pipe1_fd[READ]);
        close(pipe2_fd[WRITE]);
    }

    _pid = pid;
    write_fd = pipe1_fd[WRITE];
    read_fd = pipe2_fd[READ];
}

vector<string> tokenize(string input, char seperator)
{
    stringstream ss(input); 
    string s; 
    vector <string> line;
    while (getline(ss, s, seperator)) {    
        line.push_back(s);
    }
    return line; 
}

void create_store_processes(vector<int> &read_pipes_fd, vector<int> &child_pid, vector<string> stores, vector<string> parts){
    int pid, write_fd, read_fd;
    for (int i = 0; i < stores.size(); i++) {

        create_process("./store.out", pid, write_fd, read_fd);

        read_pipes_fd.push_back(read_fd);
        child_pid.push_back(pid);

        logger.log("INFO", "Store in city of " + stores[i] + " has been created", "main");

        string data = stores[i] + ">" + convert_vector_to_string(parts, ','); // name and parts list
        write(write_fd, data.c_str(), BUFFER_SIZE);

        close(write_fd);
    }
}

void create_part_processes(vector<int> &read_pipes_fd,vector<int> &child_pid, vector<string> parts, vector<string> stores){
    int pid, write_fd, read_fd;

    for (int i = 0; i < parts.size(); i++){

        create_process("./part.out", pid, write_fd, read_fd);

        read_pipes_fd.push_back(read_fd);
        child_pid.push_back(pid);

        logger.log("INFO", "part " + parts[i] + " has been created", "main");

        string data = parts[i] + " " + convert_vector_to_string(stores, ','); //name and stores list
        write(write_fd, data.c_str(), BUFFER_SIZE);

        close(write_fd);
    }
}

int is_in_chosen_parts(string part_name, vector<string> chosen_parts){
    for(int i = 0; i < chosen_parts.size(); i++){
        if(chosen_parts[i] == part_name){
            return 1;
        }
    }
    return 0;
}

float get_store_results(string buffer, vector<string> chosen_parts){
    float result = 0;
    vector<string> name_results = tokenize(buffer, '>');
    string name = name_results[0];
    vector<string> results = tokenize(name_results[1], ',');
    for(int i = 0; i < results.size(); i++){
        if(is_in_chosen_parts(tokenize(results[i], ':')[0], chosen_parts)){
            result += stof(tokenize(results[i], ':')[1]);
        }
    }
    return result;
}

void show_part_results(string buffer, vector<string> chosen_parts){

    vector<string> name_results = tokenize(buffer, '>');
    string part_name = name_results[0];

    if(is_in_chosen_parts(part_name, chosen_parts)){
        cout << "info for product " << part_name  << name_results[1] << "\n";
    }
}

void show_result (vector<int> &stores_fd ,vector<int> &parts_fd, vector<string> stores, vector<string> parts, vector<string> chosen_parts){
    char buffer[BUFFER_SIZE];
    float result = 0;
    for(int i = 0; i < stores_fd.size(); i++){
        memset(buffer, 0, BUFFER_SIZE);
        read(stores_fd[i], buffer, sizeof(buffer));
        //logger.log("INFO", "The total profit has been recieved successfully", "main");
        result += get_store_results(buffer, chosen_parts);
        close(stores_fd[i]);
    }
    cout << "total profit for chosen parts : " << result << "\n";
    for(int i = 0; i < parts_fd.size(); i++){
        memset(buffer, 0, BUFFER_SIZE);
        read(parts_fd[i], buffer, sizeof(buffer));
        //logger.log("INFO", "The price and quantity has been recieved successfully", "main");
        show_part_results(buffer, chosen_parts);
        close(parts_fd[i]);
    }
}

vector<string> read_csvfile(string filename)
{
    ifstream file(filename);
    if (!file.is_open()){
        logger.log("ERROR", "Error opening file", "main");
        exit(0);
    }
    else{
        vector<vector<string>> data;
        string line;
        while (getline(file, line))
        {
            vector<string> rowdata = tokenize(line, ',');
            data.push_back(rowdata);
        }
        file.close();
        return data[0];
    }
}

vector <string> find_stores(string path)
{
    vector <string> files;
    vector <string> org_files;
    DIR *dr;
    struct dirent *en;
    dr = opendir(path.c_str());
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            files.push_back(en->d_name);
        }
        closedir(dr);
    }
    for (int i = 0; i< files.size(); i++)
    {
        if (files[i] != "." && files[i] != ".." && files[i] != "Parts.csv")
        {
            string temp = tokenize(files[i], '.')[0];
            org_files.push_back(temp);
        }
    }
    return org_files;
}

vector<string> get_user_input(vector<string> parts){
    vector<string> chosen_numbers, chosen_parts;
    string line;
    getline(cin, line);

    chosen_numbers = tokenize(line, ' ');
    for (int i = 0; i < chosen_numbers.size(); i++){
        chosen_parts.push_back(parts[stoi(chosen_numbers[i])]);
    }

    return chosen_parts;
}

void show_the_menu(vector<string> parts){
    cout << "choose parts from the menu:\n";
    for(int i = 0; i < parts.size(); i++){
        cout << i << " " << parts[i] << "\n";
    }

}

int main(int argc, char const *argv[])
{
    vector<string> stores = find_stores(argv[1]);

    vector<string> parts = read_csvfile("./stores/Parts.csv");

    show_the_menu(parts);
    vector<string> chosen_parts = get_user_input(parts);

    vector<int> read_stores_pipe_fd = {};
    vector<int> read_parts_pipe_fd = {};
    vector<int> child_pid = {};

    create_named_pipes(parts, stores);

    create_store_processes(read_stores_pipe_fd, child_pid, stores, parts);
    create_part_processes(read_parts_pipe_fd, child_pid, parts, stores);

    wait(NULL);

    show_result(read_stores_pipe_fd, read_parts_pipe_fd, stores, parts, chosen_parts);

    delete_named_pipes(parts, stores);

    return 0;
}



