#pragma once
#include <iostream>
#include <vector>
#include<set>
#include<map>
#include<algorithm>
#include<bitset>
#include<cstdint>
#include<fstream>
#include"estrutura_contexto.hpp"
using namespace std;

typedef struct ArquivoInfo{
    string nome;
    uintmax_t tamanho;
}ArquivoInfo;

typedef struct Codificador_aritmetico Codificador_aritmetico;
struct Codificador_aritmetico{

    static const uint32_t BITS_PRECISAO = 32;
    static const uint64_t TOP      = 0xFFFFFFFFULL;
    static const uint64_t HALF     = 0x80000000ULL;
    static const uint64_t FIRST_QTR = 0x40000000ULL;
    static const uint64_t THIRD_QTR = 0xC0000000ULL;

    uint32_t low  = 0;
    uint32_t high = (uint32_t)TOP;
    uint64_t bits_lidos = 0; // Number of bits read from the stream (decode)
    uint64_t bits_consumidos_total;
    vector<bool> bits_buffer;
    uint32_t bits_pendentes = 0;
     uint64_t bits_emitidos_total = 0;
    uint32_t value = 0;

    uint8_t byte_leitura        = 0;
    int     bits_restantes_byte = 0;

    /** Returns the total frequency of symbols excluded from a context. */
    uint32_t count_excluded(No* contexto, const set<uint8_t>& excluidos)
    {
        uint32_t t = 0;
        for(const auto& [s,f] : contexto->frequencias){
            if(s < 256 && excluidos.count((uint8_t)s))
                t += f;
        }
        return t;
    }

    /** Encodes one symbol using the supplied context and exclusion set. */
    bool encode_byte(
        uint32_t simbolo,
        No* contexto,
        const set<uint8_t>& excluidos)
    {
        uint32_t total = contexto->total - count_excluded(contexto, excluidos);

        if(total == 0)
            return false;

        uint32_t cumulativa  = 0;
        uint32_t freq_simbolo = 0;
        bool achou = false;

        for(const auto& [s,f] : contexto->frequencias){
            if(s != ESCAPE && excluidos.count((uint8_t)s)) continue;
            if(f == 0) continue;

            if((uint32_t)s == simbolo){
                freq_simbolo = f;
                achou = true;
                break;
            }
            cumulativa += f;
        }

        if(!achou) return false;

        uint64_t range = (uint64_t)high - (uint64_t)low + 1ULL;

        uint64_t novo_high = (uint64_t)low
            + (range * (uint64_t)(cumulativa + freq_simbolo)) / total - 1ULL;
        uint64_t novo_low  = (uint64_t)low
            + (range * (uint64_t)cumulativa) / total;

        low  = (uint32_t)novo_low;
        high = (uint32_t)novo_high;

        renormalize();
        return true;
    }

    /** Renormalizes the encoder interval and emits stable leading bits. */
    void renormalize(){
        while(true){
            if(high < HALF){
                write_bit_with_pending(0);
            }
            else if(low >= HALF){
                write_bit_with_pending(1);
                low  -= (uint32_t)HALF;
                high -= (uint32_t)HALF;
            }
            else if(low >= FIRST_QTR && high < THIRD_QTR){
                bits_pendentes++;
                low  -= (uint32_t)FIRST_QTR;
                high -= (uint32_t)FIRST_QTR;
            }
            else break;

            low  =  low << 1;
            high = (high << 1) | 1;
        }
    }

    /** Appends one bit to the output buffer and updates the bit counter. */
    void write_bit(bool bit){
        bits_buffer.push_back(bit);
        bits_emitidos_total++;
    }

    /** Emits one bit followed by all deferred complement bits. */
    void write_bit_with_pending(bool bit){
        write_bit(bit);
        for(uint32_t i = 0; i < bits_pendentes; i++)
            write_bit(!bit);
        bits_pendentes = 0;
    }

    /** Finalizes the arithmetic interval after all input symbols are encoded. */
    void finalize_encoding(){
        bits_pendentes++;
        if(low < FIRST_QTR)
            write_bit_with_pending(0);
        else
            write_bit_with_pending(1);
    }

    /** Resets the arithmetic coder state for a new stream. */
    void reset() {
        low = 0;
        high = (uint32_t)TOP;

        bits_buffer.clear();
        bits_pendentes = 0;

        bits_emitidos_total = 0;

        bits_lidos = 0;
        bits_consumidos_total = 0;

        value = 0;
        byte_leitura = 0;
        bits_restantes_byte = 0;
    }

    /** Writes the encoded bit stream and file metadata to a binary archive. */
    bool save_archive(const string& nome_arquivo,
                       const vector<ArquivoInfo>& arquivos_vet,
                       uint64_t tamanho_total_original)
    {
        try{
            ofstream arquivo(nome_arquivo, ios::binary);
            if(!arquivo.is_open()){
                cerr << "[ERROR] Could not open file: " << nome_arquivo << endl;
                return false;
            }

            uint64_t quantidade_arquivos = arquivos_vet.size();
            arquivo.write((char*)&quantidade_arquivos, sizeof(uint64_t));

            for(const auto& arq : arquivos_vet){
                uint16_t nome_tamanho = (uint16_t)arq.nome.size();
                arquivo.write((char*)&nome_tamanho, sizeof(uint16_t));
                arquivo.write(arq.nome.data(), nome_tamanho);
                uint64_t tam = (uint64_t)arq.tamanho;
                arquivo.write((char*)&tam, sizeof(uint64_t));
            }

            uint32_t tamanho_bits = (uint32_t)bits_buffer.size();
            arquivo.write((char*)&tamanho_bits, sizeof(uint32_t));

            uint8_t byte_atual = 0;
            int bit_count = 0;
            for(size_t i = 0; i < bits_buffer.size(); i++){
                byte_atual = (byte_atual << 1) | (bits_buffer[i] ? 1 : 0);
                bit_count++;
                if(bit_count == 8){
                    arquivo.put(byte_atual);
                    byte_atual = 0;
                    bit_count  = 0;
                }
            }
            if(bit_count > 0){
                byte_atual <<= (8 - bit_count);
                arquivo.put(byte_atual);
            }

            arquivo.close();
            cout << "[INFO] Compressed file saved: " << nome_arquivo << endl;
            cout << "[INFO] Total bits: " << bits_buffer.size() << endl;
            cout << "[INFO] Total bytes: "
                 << (bits_buffer.size() / 8 + (bits_buffer.size() % 8 ? 1 : 0)) << endl;
            return true;
        }catch(exception& e){
            cerr << "[ERROR] Exception while saving file: " << e.what() << endl;
            return false;
        }
    }

    /** Clears the encoded bit buffer and restores the interval bounds. */
    void clear_buffer(){
        bits_buffer.clear();
        bits_pendentes = 0;
        low  = 0;
        high = (uint32_t)TOP;
    }

    /** Returns the encoded payload size in bytes, including partial bytes. */
    uint64_t compressed_size(){
        return bits_buffer.size() / 8 + (bits_buffer.size() % 8 ? 1 : 0);
    }

    /** Reads one bit from the compressed stream. */
    bool read_bit(ifstream& arquivo_bits){
        if(bits_restantes_byte == 0){
            uint8_t b = 0;
            arquivo_bits.get((char&)b);
            byte_leitura        = b;
            bits_restantes_byte = 8;
        }
        bool bit = (byte_leitura >> 7) & 1;
        byte_leitura <<= 1;
        bits_restantes_byte--;
        bits_lidos++;
        return bit;
    }

    /** Initializes the decoder register with the first 32 stream bits. */
    void prepare_decoding(ifstream& arquivo_bits){
        byte_leitura        = 0;
        bits_restantes_byte = 0;
        low   = 0;
        high  = (uint32_t)TOP;
        value = 0;
        bits_consumidos_total = 0;
        for(int i = 0; i < 32; i++)
            value = (value << 1) | (read_bit(arquivo_bits) ? 1 : 0);
    }

    /** Renormalizes the decoder interval using bits from the input stream. */
    void renormalize_reading(ifstream& arquivo_bits){
        while(true){
            if(high < HALF){
                low   =  low        << 1;
                high  = (high       << 1) | 1;
                value = (value      << 1) | (read_bit(arquivo_bits) ? 1 : 0);
            }
            else if(low >= HALF){
                low   = (low   - (uint32_t)HALF) << 1;
                high  = ((high - (uint32_t)HALF) << 1) | 1;
                value = ((value - (uint32_t)HALF) << 1) | (read_bit(arquivo_bits) ? 1 : 0);
            }
            else if(low >= FIRST_QTR && high < THIRD_QTR){
                low   = (low   - (uint32_t)FIRST_QTR) << 1;
                high  = ((high - (uint32_t)FIRST_QTR) << 1) | 1;
                value = ((value - (uint32_t)FIRST_QTR) << 1) | (read_bit(arquivo_bits) ? 1 : 0);
            }
            else break;
        }
    }

    /** Decodes one symbol from the compressed stream using a context. */
    uint32_t decode_byte(No* contexto, const set<uint8_t>& excluidos, ifstream& arquivo_bits){
        uint32_t total = contexto->total - count_excluded(contexto, excluidos);

        if(total == 0) return ESCAPE;

        uint64_t range = (uint64_t)high - (uint64_t)low + 1ULL;
        uint64_t ponto = ((((uint64_t)value - (uint64_t)low + 1ULL) * (uint64_t)total) - 1ULL) / range;

        uint32_t cumulativa = 0;
        uint32_t freq_simbolo = 0;
        uint32_t simbolo_encontrado = 0;
        bool encontrou = false;

        for(const auto& [s,f] : contexto->frequencias) {
            if(s != ESCAPE && excluidos.count((uint8_t)s)) continue;
            if(f == 0) continue;

            if(ponto >= cumulativa && ponto < cumulativa + f) {
                simbolo_encontrado = (uint32_t)s;
                freq_simbolo = f;
                encontrou = true;
                break;
            }
            cumulativa += f;
        }

        if(!encontrou) {
            return ESCAPE;
        }

        uint64_t next_low = (uint64_t)low + (range * cumulativa) / total;
        uint64_t next_high = (uint64_t)low + (range * (cumulativa + freq_simbolo)) / total - 1ULL;
        low = (uint32_t)next_low;
        high = (uint32_t)next_high;

        renormalize_reading(arquivo_bits);

        return simbolo_encontrado;
    }
};