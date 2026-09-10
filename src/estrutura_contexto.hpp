#pragma once
#include<utility>
#include<deque>
#include<cstdint>
#include<vector>

using namespace std;

const uint32_t ESCAPE = 256;

struct No{
    vector<pair<uint8_t,No*>> filhos;
    vector<pair<uint16_t,uint32_t>> frequencias;
    No* pai;
    uint32_t total;
    uint16_t distintos = 0;

    No();

    uint32_t get_freq(uint16_t simbolo) const;
    uint32_t& freq_ref(uint16_t simbolo);
    No* busca_filho(uint8_t b) const;
};

struct trie_contexto{
    No* raiz;
    uint64_t num_nos = 1;

    trie_contexto();
    ~trie_contexto();

    void libera(No* no);

    bool insere_byte_em_contexto(const deque<uint8_t>& bytes);
    No* busca_contexto_byte(const deque<uint8_t>& contexto);
    void atualiza_frequencia(No* contexto, uint8_t simbolo);


};