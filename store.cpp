#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <tuple>
#include <fcntl.h>
#include <dirent.h>
#include "logger.hpp"

using namespace std;

#define BUFFER_SIZE 1024

Logger logger;

string file_name;

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

string convert_vector_to_string(vector<tuple<float, string>> words){
    string result = "";
    for (size_t i = 0; i < words.size(); ++i) { 
        result += (get<1>(words[i]) + ":" + to_string(get<0>(words[i])) + ","); 
    } 
    return result;
}

void get_data(int read_fd, vector<string> &parts){
    char buffer[BUFFER_SIZE];
    read(read_fd, buffer, BUFFER_SIZE);
    string data(buffer);
    vector<string> tokenized = tokenize(data, '>');
    file_name = tokenized[0];
    parts = tokenize(tokenized[1], ',');

    logger.log("INFO", "City name and list of citites has been gotten from the main", "Store " + file_name);
}

int is_product_in_data(string product, vector<vector<tuple<string, float, int>>> data){
    for(int i = 0; i < data.size(); i++){
        if(get<0>(data[i][0]) == product){
            return i;
        }
    }
    return -1;
}

void decrease_from_input(vector<tuple<string, float, int>> &queue, float price, int quantity, tuple<float, string> &total_profit){

    while(get<2>(queue[0]) < quantity){
        quantity -= get<2>(queue[0]);
        get<0>(total_profit) += (price - get<1>(queue[0])) * get<2>(queue[0]);
        queue.erase(queue.begin());
    }
    get<2>(queue[0]) -= quantity;
    get<0>(total_profit) += (price - get<1>(queue[0])) * quantity;
}

void update_inventory(vector<vector<tuple<string, float, int>>> &data, string product, int quantity, float price, string type, vector<tuple<float, string>> &total_profit){
    int index;
    if((index = is_product_in_data(product, data)) != -1){
        if(type == "input" || type == "input\r"){

            data[index].emplace_back(product, price, quantity);
        } 
        else if(type == "output" || type == "output\r"){

            decrease_from_input(data[index], price, quantity, total_profit[index]);
        }
    }  
    else{
        vector<tuple<string, float, int>> temp;
        temp.emplace_back(product, price, quantity);
        total_profit.emplace_back(0, product);
        data.push_back(temp);
    }
}

void get_quantity_price(vector<tuple<string, float, int>> data, float &price, int &quantity){
    for(int i = 0; i < data.size(); i++){
        quantity += get<2>(data[i]);
        price += (get<2>(data[i]) * get<1>(data[i]));
    }
}

void send_msg_to_part(string part, vector<tuple<string, float, int>> data, string file_name){
    string pipe_store_to_part = file_name + "," + part;
    int fd = open(pipe_store_to_part.c_str(), O_WRONLY);

    if (fd < 0) {
        logger.log("ERROR", "Failed to open named pipe: " + pipe_store_to_part, "Store " + file_name);
        exit(0);
    }

    int total_quantity = 0;
    float total_price = 0;
    get_quantity_price(data, total_price, total_quantity);

    string msg = to_string(total_price) + " " + to_string(total_quantity);
    write(fd, msg.c_str(), BUFFER_SIZE);
    logger.log("INFO", "The price and quantity have been sent to " + part + " successfully", "Store " + file_name);
    close(fd);
}

vector<tuple<string, float, int>> find_part_data(string part, vector<vector<tuple<string, float, int>>> data){
    for (int i = 0; i < data.size(); i++){
        if(get<0>(data[i][0]) == part){
            return data[i];
        }
    }
    return data[0];
}

void write_on_named_pipes(vector<vector<tuple<string, float, int>>> data, vector<string> parts){

    for (int i = 0; i < parts.size(); i++) {
        send_msg_to_part(parts[i], find_part_data(parts[i], data), file_name);
    }
}

vector<tuple<float, string>> read_file(vector<string> parts){
    string product, type, line;
    int quantity;
    float price;
    vector<string> row;
    vector<vector<tuple<string, float, int>>> data;
    vector<tuple<float, string>> total_profit;

    ifstream file("./stores/" + file_name + ".csv");
    while (getline(file, line))
    {
        row = tokenize(line, ',');
        product = row[0];
        price = stof(row[1]);
        quantity = stoi(row[2]);
        type = row[3];

        update_inventory(data, product, quantity, price, type, total_profit);
    }
    file.close();
    write_on_named_pipes(data, parts);
    return total_profit;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        logger.log("ERROR", "incorrect number of arguments", "Store " + file_name);
        exit(0);
    }

    int read_fd = stoi(argv[0]);
    int write_fd = stoi(argv[1]);

    vector<string> parts;
    get_data(read_fd, parts); // get file_name and parts list

    close(read_fd);
    vector<tuple<float, string>> total_profit = read_file(parts);

    string result = file_name + ">" + convert_vector_to_string(total_profit);
    write(write_fd, result.c_str(), BUFFER_SIZE);
    close(write_fd);

    logger.log("INFO", "The total profit has been returned to main successfully", "Store " + file_name);

    return 0;
}
