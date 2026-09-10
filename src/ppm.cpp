#include "ppm.hpp"

/** Constructs a PPM model with the specified maximum context order. */
Ppm::Ppm(int k, bool treinando) {
	Kmax = k;
	equiprovaveis = new No();
	inicia_equiprovaveis();
	total_simbolos_processados = 0;
	treino = treinando;
}

/** Releases the dynamically allocated equiprobable model. */
Ppm::~Ppm() {
	delete equiprovaveis;
}

/** Initializes the fallback model with uniform byte frequencies. */
void Ppm::inicia_equiprovaveis() {
	for (int i = 0; i < 256; i++) {
		equiprovaveis->freq_ref(static_cast<uint16_t>(i)) = 1;
	}
	equiprovaveis->total = 256;
}

/** Resets the arithmetic interval, fallback model, and active context window. */
void Ppm::reinicia_modelo() {
	aritmetico.low = 0;
	aritmetico.high = static_cast<uint32_t>(Codificador_aritmetico::TOP);
	aritmetico.bits_pendentes = 0;
	inicia_equiprovaveis();
	janela_atual.clear();
}

/** Reports whether a context contains a nonzero frequency for a symbol. */
bool Ppm::existe_contexto(No* contexto, uint8_t simbolo) {
	return contexto->get_freq(simbolo) > 0;
}

/** Returns the largest context represented by the current byte window. */
No* Ppm::busca_maior_contexto(deque<uint8_t>& janela) {
	return arvore.busca_contexto_byte(janela);
}

/** Propagates a symbol-frequency update through the context hierarchy. */
void Ppm::atualiza_frequencia_contexto(No* contexto, uint8_t atual) {
	arvore.atualiza_frequencia(contexto, atual);
}

/** Calculates the escape frequency for a context. */
uint32_t Ppm::calcula_escape(No* contexto) {
	return max(1u, static_cast<uint32_t>(contexto->distintos));
}

/** Adds all observed symbols in a context to the exclusion set. */
void Ppm::insere_em_excluidos(No* contexto) {
	for (const auto& [s, f] : contexto->frequencias) {
		if (f > 0 && s < 256) {
			excluidos.insert(static_cast<uint8_t>(s));
		}
	}
}

/** Appends a symbol to the context window and enforces Kmax. */
void Ppm::atualiza_janela(uint8_t atual) {
	janela_atual.push_back(atual);
	if (janela_atual.size() > static_cast<size_t>(Kmax)) {
		janela_atual.pop_front();
	}
}

/** Updates the context window and optionally extends the context trie. */
bool Ppm::atualiza_contexto(uint8_t atual) {
	atualiza_janela(atual);
	if (treino) {
		return arvore.insere_byte_em_contexto(janela_atual);
	}
	return true;
}

/** Encodes one input symbol and updates the adaptive PPM model. */
void Ppm::processa_simbolo(uint8_t atual) {
	total_simbolos_processados++;
	bool codificado = false;
	bits_inicial = aritmetico.bits_emitidos_total;
	No* contexto = arvore.busca_contexto_byte(janela_atual);
	No* contexto_inicial = contexto;

	while (contexto) {
		if (excluidos.count(atual) == 0) {
			uint32_t freq_esc = calcula_escape(contexto);
			contexto->freq_ref(ESCAPE) = freq_esc;
			contexto->total += freq_esc;

			if (existe_contexto(contexto, atual)) {
				aritmetico.encode_byte(atual, contexto, excluidos);
				codificado = true;

				contexto->freq_ref(ESCAPE) = 0;
				contexto->total -= freq_esc;
				break;
			}

			aritmetico.encode_byte(ESCAPE, contexto, excluidos);

			contexto->freq_ref(ESCAPE) = 0;
			contexto->total -= freq_esc;
			insere_em_excluidos(contexto);
		}
		contexto = contexto->pai;
	}

	if (!codificado) {
		aritmetico.encode_byte(atual, equiprovaveis, excluidos);
	}

	bits_final = aritmetico.bits_emitidos_total;
	bytes_na_janela++;

	if (treino) {
		arvore.atualiza_frequencia(contexto_inicial, atual);
	}
	atualiza_contexto(atual);

	excluidos.clear();
}

/** Decodes one symbol and updates the adaptive PPM model. */
uint8_t Ppm::decodifica_simbolo(ifstream& arquivo_bits) {
	uint32_t simbolo_decodificado = ESCAPE;

	bits_inicial = aritmetico.bits_consumidos_total;

	No* contexto = arvore.busca_contexto_byte(janela_atual);
	No* contexto_inicial = contexto;

	while (contexto) {
		uint32_t freq_esc = calcula_escape(contexto);
		contexto->freq_ref(ESCAPE) = freq_esc;
		contexto->total += freq_esc;

		uint32_t resultado = aritmetico.decode_byte(contexto, excluidos, arquivo_bits);

		contexto->freq_ref(ESCAPE) = 0;
		contexto->total -= freq_esc;

		if (resultado != ESCAPE) {
			simbolo_decodificado = resultado;
					break;
		}

		insere_em_excluidos(contexto);
		contexto = contexto->pai;
	}

	if (simbolo_decodificado == ESCAPE) {
		simbolo_decodificado = aritmetico.decode_byte(equiprovaveis, excluidos, arquivo_bits);
	}

	bits_final = aritmetico.bits_consumidos_total;
	bytes_na_janela++;

	if (treino) {
		arvore.atualiza_frequencia(contexto_inicial, (uint8_t)simbolo_decodificado);
	}
	atualiza_contexto((uint8_t)simbolo_decodificado);

	total_simbolos_processados++;
	excluidos.clear();

	return (uint8_t)simbolo_decodificado;
}