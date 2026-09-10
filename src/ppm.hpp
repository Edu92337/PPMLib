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

    void inicia_equiprovaveis();
    void reinicia_modelo();
    bool existe_contexto(No* contexto, uint8_t simbolo);
    No* busca_maior_contexto(deque<uint8_t>& janela);
    void atualiza_frequencia_contexto(No* contexto, uint8_t atual);
    uint32_t calcula_escape(No* contexto);
    void insere_em_excluidos(No* contexto);
    void atualiza_janela(uint8_t atual);
    bool atualiza_contexto(uint8_t atual);
    void processa_simbolo(uint8_t atual);
    uint8_t decodifica_simbolo(ifstream& arquivo_bits);

};