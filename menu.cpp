#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <vector>
#include <fstream>
using namespace std;
namespace fs = std::filesystem;


vector<string> listarArchivosData(const string& carpeta) {
    vector<string> archivos;
    try {
        for (auto& p : fs::directory_iterator(carpeta)) {
            if (p.is_regular_file()) {
                archivos.push_back(p.path().string());
            }
        }
    } catch (...) {
        cerr << "Error: no se pudo abrir carpeta " << carpeta << "\n";
    }
    return archivos;
}


string elegirArchivo(const vector<string>& archivos, const string& mensaje, const string& def) {
    cout << mensaje << "\n";
    for (size_t i = 0; i < archivos.size(); i++) {
        cout << i+1 << ") " << archivos[i] << "\n";
    }
    cout << "Selecciona [1-" << archivos.size() << "] o Enter para usar [" << def << "]: ";

    string input;
    getline(cin, input);
    if (input.empty()) return def;

    int idx = stoi(input);
    if (idx >= 1 && (size_t)idx <= archivos.size()) {
        return archivos[idx-1];
    }
    return def;
}

int main() {
    try { fs::create_directories("results"); } catch(...) {}

    while (true) {
        cout << "\n=== Menu Lab7 ===\n";
        cout << "1) Comprimir archivo\n";
        cout << "2) Descomprimir archivo\n";
        cout << "3) Salir\n";
        cout << "Opcion: ";
        string opcion;
        getline(cin, opcion);

        if (opcion == "3") break;

        if (opcion == "1") {
           
            auto archivos = listarArchivosData("data");
            if (archivos.empty()) {
                cerr << "No se encontraron archivos en data/\n";
                continue;
            }

            string in = elegirArchivo(archivos, "Archivos disponibles en data/:", "data/entrada.txt");
            string out = "results/out_par.bin";

            cout << "Archivo salida [" << out << "]: ";
            string tmp; getline(cin, tmp);
            if (!tmp.empty()) out = tmp;

            cout << "Hilos [4]: ";
            getline(cin, tmp);
            int threads = tmp.empty() ? 4 : stoi(tmp);

            cout << "MB por bloque [1]: ";
            getline(cin, tmp);
            int mb = tmp.empty() ? 1 : stoi(tmp);

            string cmd = "./pcompress \"" + in + "\" \"" + out + "\" "
                       + to_string(threads) + " " + to_string(mb);
            cout << "-> " << cmd << "\n";
            system(cmd.c_str());

           
            ofstream meta("results/last_input.txt");
            if (meta) meta << in;
            meta.close();
        }
        else if (opcion == "2") {
            string in = "results/out_par.bin";
            string out = "results/recuperado.txt";

            cout << "Archivo comprimido [" << in << "]: ";
            string tmp; getline(cin, tmp);
            if (!tmp.empty()) in = tmp;

            cout << "Archivo salida [" << out << "]: ";
            getline(cin, tmp);
            if (!tmp.empty()) out = tmp;

            cout << "Hilos [4]: ";
            getline(cin, tmp);
            int threads = tmp.empty() ? 4 : stoi(tmp);

            string cmd = "./pdecompress \"" + in + "\" \"" + out + "\" "
                       + to_string(threads);
            cout << "-> " << cmd << "\n";
            system(cmd.c_str());

            
            string original = "data/entrada.txt"; 
            ifstream meta("results/last_input.txt");
            if (meta) getline(meta, original);
            meta.close();

            string diff = "diff -q \"" + original + "\" \"" + out + "\" && echo 'Son idénticos'";
            system(diff.c_str());
        }
        else {
            cout << "Opcion invalida\n";
        }
    }
    return 0;
}

