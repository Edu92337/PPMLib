#include "estrutura_contexto.hpp"

/** Initializes an empty context node without a parent. */
No::No() : pai(nullptr), total(0) {}

/** Returns the stored frequency for a symbol, or zero if absent. */
uint32_t No::get_freq(uint16_t simbolo) const {
	for (const auto& [s, f] : frequencias) {
		if (s == simbolo) return f;
	}
	return 0;
}

/** Returns a mutable frequency entry, creating it when necessary. */
uint32_t& No::freq_ref(uint16_t simbolo) {
	for (auto& [s, f] : frequencias) {
		if (s == simbolo) return f;
	}
	frequencias.push_back({simbolo, 0});
	return frequencias.back().second;
}

/** Returns the child node associated with a byte, or nullptr if absent. */
No* No::busca_filho(uint8_t b) const {
	for (const auto& [byte, no] : filhos) {
		if (byte == b) return no;
	}
	return nullptr;
}

/** Creates an empty context trie with an allocated root node. */
trie_contexto::trie_contexto() : raiz(new No()) {}

/** Releases every node owned by the context trie. */
trie_contexto::~trie_contexto() {
	libera(raiz);
}

/** Recursively releases a node and all of its descendants. */
void trie_contexto::libera(No* no) {
	if (!no) return;
	for (auto& [_, filho] : no->filhos) {
		libera(filho);
	}
	num_nos--;
	delete no;
}

/** Inserts all suffix contexts represented by a byte sequence. */
bool trie_contexto::insere_byte_em_contexto(const deque<uint8_t>& bytes) {
	No* atual = raiz;
	for (auto it = bytes.rbegin(); it != bytes.rend(); ++it) {
		uint8_t b = *it;
		No* filho = atual->busca_filho(b);
		if (!filho) {
			filho = new No();
			filho->pai = atual;
			atual->filhos.push_back({b, filho});
			num_nos++;
		}
		atual = filho;
	}
	return true;
}

/** Returns the deepest context matching the supplied byte sequence. */
No* trie_contexto::busca_contexto_byte(const deque<uint8_t>& contexto) {
	No* atual = raiz;
	for (auto it = contexto.rbegin(); it != contexto.rend(); ++it) {
		No* filho = atual->busca_filho(*it);
		if (filho) atual = filho;
		else return atual;
	}
	return atual;
}

/** Increments a symbol frequency in the context and all ancestor contexts. */
void trie_contexto::atualiza_frequencia(No* contexto, uint8_t simbolo) {
	while (contexto != nullptr) {
		uint32_t& f = contexto->freq_ref(simbolo);
		if (f == 0) contexto->distintos++;
		f++;
		contexto->total++;
		contexto = contexto->pai;
	}
}
