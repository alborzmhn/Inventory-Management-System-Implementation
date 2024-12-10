#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include "logger.hpp"

#define BUFFER_SIZE 1024

using namespace std;

Logger logger;

string part_name;

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

void get_data(int read_fd, vector<string> &stores){
    char buffer[BUFFER_SIZE];
    read(read_fd, buffer, BUFFER_SIZE);
    string data(buffer);
    vector<string> tokenized = tokenize(data, ' ');
    part_name = tokenized[0];
    stores = tokenize(tokenized[1], ',');
    logger.log("INFO", "Part name and list of citites has been gotten from the main", "Part " + part_name);

}

string get_msg_from_store(string store){
    string pipe_store_to_part = store + "," + part_name;
    int fd = open(pipe_store_to_part.c_str(), O_RDONLY);

    if (fd < 0) {
        logger.log("ERROR", "Failed to open named pipe: " + pipe_store_to_part, "Part " + part_name);
        exit(0);
    }

    char store_buffer[BUFFER_SIZE];
    int bytes_read = read(fd, store_buffer, BUFFER_SIZE);
    if (bytes_read > 0) {
        store_buffer[bytes_read] = '\0';
    }
    logger.log("INFO", "The price and quantity have been recieved from " + store + " successfully", "Part " + part_name);

    close(fd);
    string msg = store_buffer;
    return msg;
}

void make_result(int &quantity, double &price, vector<string> stores){

    int temp_quantity;
    double temp_price; 
    string result = "";

    for (int i = 0; i < stores.size(); i++) {
        string msg = get_msg_from_store(stores[i]);
        istringstream iss(msg);

        iss >> temp_price >> temp_quantity;
        quantity += temp_quantity;
        price += temp_price;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        logger.log("ERROR", "incorrect number of arguments", "Part " + part_name);
        exit(0);
    }

    int read_fd = stoi(argv[0]);
    int write_fd = stoi(argv[1]);

    vector<string> stores;
    get_data(read_fd, stores); // get part_name and stores list

    close(read_fd);

    int quantity = 0;
    double price = 0;
    make_result(quantity, price, stores);
    string result = part_name + "> : total quantity = " + to_string(quantity) + " and total price = " + to_string((int)price);
    
    write(write_fd, result.c_str(), BUFFER_SIZE);
    close(write_fd);

    logger.log("INFO", "The total quantity and total price have been returned to main successfully", "Part " + part_name);

    return 0;
}