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
    No* find_child(uint8_t b) const;
};

struct trie_contexto{
    No* raiz;
    uint64_t num_nos = 1;

    trie_contexto();
    ~trie_contexto();

    void release(No* no);

    bool insert_byte_context(const deque<uint8_t>& bytes);
    No* find_context(const deque<uint8_t>& contexto);
    void update_frequency(No* contexto, uint8_t simbolo);


};