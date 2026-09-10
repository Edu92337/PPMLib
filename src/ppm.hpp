#pragma once
#include<algorithm>
#include<cstdint>
#include<deque>
#include<set>
#include"codificador_aritmetico.hpp"
#include"estrutura_contexto.hpp"

using namespace std;
typedef struct Simbolo Simbolo;
struct Simbolo{
    uint8_t byte;
    uint64_t bits_emitidos;
};


struct Ppm{
    trie_contexto arvore;
    deque<uint8_t> janela_atual;
    Codificador_aritmetico aritmetico;
    set<uint8_t>excluidos;
    No* equiprovaveis;
    int Kmax;
    int J;
    int adapta;
    uint64_t bits_inicial;
    uint64_t bits_final;
    uint64_t comprimento_emitido;
    uint64_t total_simbolos_processados;
    double l0 = 0,lf = 0;
    long long bytes_na_janela = 0;
    bool treino;
    Ppm(int k, bool treinando);
    ~Ppm();

    void initialize_equiprobable();
    void reset_model();
    bool context_contains(No* contexto, uint8_t simbolo);
    No* find_largest_context(deque<uint8_t>& janela);
    void update_context_frequency(No* contexto, uint8_t atual);
    uint32_t calculate_escape(No* contexto);
    void add_to_excluded(No* contexto);
    void update_window(uint8_t atual);
    bool update_context(uint8_t atual);
    void process_symbol(uint8_t atual);
    uint8_t decode_symbol(ifstream& arquivo_bits);

};