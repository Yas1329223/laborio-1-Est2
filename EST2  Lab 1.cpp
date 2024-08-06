#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

unordered_map<string, json> inventory;

void processInputFile(const string& inputFile) {
    ifstream file(inputFile);
    if (!file.is_open()) {
        cerr << "No se pudo abrir el archivo de entrada." << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        auto pos = line.find(';');
        if (pos == string::npos) continue;

        string operation = line.substr(0, pos);
        string data = line.substr(pos + 1);

        try {
            json item = json::parse(data);

            if (operation == "INSERT") {
                inventory[item["isbn"].get<string>()] = item;
            }
            else if (operation == "PATCH") {
                string isbn = item["isbn"].get<string>();
                if (inventory.find(isbn) != inventory.end()) {
                    auto& existingItem = inventory[isbn];
                    for (auto& el : item.items()) {
                        existingItem[el.key()] = el.value();
                    }
                }
            }
            else if (operation == "DELETE") {
                string isbn = item["isbn"].get<string>();
                inventory.erase(isbn);
            }
        }
        catch (const json::parse_error& e) {
            cerr << "Error de análisis JSON: " << e.what() << " en la línea: " << line << endl;
        }
        catch (const std::exception& e) {
            cerr << "Error: " << e.what() << " en la línea: " << line << endl;
        }
    }

    file.close();
}

void writeOutputFile(const string& outputFile) {
    ofstream file(outputFile);
    if (!file.is_open()) {
        cerr << "No se pudo abrir el archivo de salida." << endl;
        return;
    }

    // Almacenar los datos en un vector para ordenar
    vector<pair<string, json>> sortedItems;
    for (const auto& entry : inventory) {
        sortedItems.emplace_back(entry);
    }

    // Ordenar el vector por ISBN
    sort(sortedItems.begin(), sortedItems.end(),
        [](const pair<string, json>& a, const pair<string, json>& b) {
            return a.second["isbn"].get<string>() < b.second["isbn"].get<string>();
        });

    // Escribir los datos ordenados en el archivo
    for (const auto& entry : sortedItems) {
        json item = entry.second;

        // Crear un objeto JSON en el orden deseado
        json orderedJson;
        orderedJson["isbn"] = item["isbn"];
        orderedJson["name"] = item["name"];
        orderedJson["author"] = item["author"];
        orderedJson["price"] = item["price"];
        orderedJson["quantity"] = item["quantity"];

        file << orderedJson.dump() << endl;
    }

    file.close();
}

void processSearchFile(const string& searchFile, const string& outputFile, const string& finalFile) {
    ifstream searchFileStream(searchFile);
    if (!searchFileStream.is_open()) {
        cerr << "No se pudo abrir el archivo de búsqueda." << endl;
        return;
    }

    ifstream outputFileStream(outputFile);
    if (!outputFileStream.is_open()) {
        cerr << "No se pudo abrir el archivo de salida para lectura." << endl;
        return;
    }

    unordered_map<string, json> updatedInventory;
    string line;
    while (getline(outputFileStream, line)) {
        try {
            json item = json::parse(line);
            updatedInventory[item["isbn"].get<string>()] = item;
        }
        catch (const json::parse_error& e) {
            cerr << "Error de análisis JSON en el archivo de salida: " << e.what() << endl;
        }
        catch (const std::exception& e) {
            cerr << "Error: " << e.what() << endl;
        }
    }

    outputFileStream.close();

    unordered_map<string, json> searchResults;
    while (getline(searchFileStream, line)) {
        auto pos = line.find(';');
        if (pos == string::npos) continue;

        string searchType = line.substr(0, pos);
        string data = line.substr(pos + 1);

        try {
            json searchItem = json::parse(data);

            if (searchType == "SEARCH") {
                string name = searchItem["name"].get<string>();
                for (const auto& entry : updatedInventory) {
                    if (entry.second["name"].get<string>() == name) {
                        searchResults[entry.first] = entry.second;
                    }
                }
            }
        }
        catch (const json::parse_error& e) {
            cerr << "Error de análisis JSON en el archivo de búsqueda: " << e.what() << endl;
        }
        catch (const std::exception& e) {
            cerr << "Error: " << e.what() << endl;
        }
    }

    searchFileStream.close();

    ofstream finalFileOut(finalFile);
    if (!finalFileOut.is_open()) {
        cerr << "No se pudo abrir el archivo final para escritura." << endl;
        return;
    }

    // Almacenar los resultados de búsqueda en un vector para ordenar
    vector<pair<string, json>> searchResultsVector;
    for (const auto& entry : searchResults) {
        searchResultsVector.emplace_back(entry);
    }

    // Ordenar el vector por ISBN
    sort(searchResultsVector.begin(), searchResultsVector.end(),
        [](const pair<string, json>& a, const pair<string, json>& b) {
            return a.second["isbn"].get<string>() < b.second["isbn"].get<string>();
        });

    // Escribir los datos ordenados en el archivo final
    for (const auto& entry : searchResultsVector) {
        json item = entry.second;

        // Crear un objeto JSON en el orden deseado
        json orderedJson;
        orderedJson["isbn"] = item["isbn"];
        orderedJson["name"] = item["name"];
        orderedJson["author"] = item["author"];
        orderedJson["price"] = item["price"];
        orderedJson["quantity"] = item["quantity"];

        finalFileOut << orderedJson.dump() << endl;
    }

    finalFileOut.close();
}

int main() {
    string inputFile = "lab01_books.csv";  // Nombre del archivo de entrada
    string outputFile = "output.txt"; // Nombre del archivo de salida
    string searchFile = "lab01_search.csv"; // Nombre del archivo de búsqueda
    string finalFile = "final_output.txt"; // Nombre del archivo final

    processInputFile(inputFile);
    writeOutputFile(outputFile);
    processSearchFile(searchFile, outputFile, finalFile);

    return 0;
}
