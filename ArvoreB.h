/* 
 * File:   ArvoreB.h
 * Authors: Daniel Freitas Martins  -   2304
 *          Maria Dalila            -   3030
 *          Gabriel Teixeira Pinto  -   3044
 *
 */
// Agradecimentos ao Prof Nívio Ziviani, códigos obtidos de
// http://www2.dcc.ufmg.br/livros/algoritmos/transparencias.php

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdint.h>

#include "../Little/Little.h"


typedef struct Page Page;
typedef struct Page{
    Little item;
    Page *filho[2*BSIZE + 1];
    size_t qtd_filhos;
} Page;

void arvoreBCria(Page **alvo);
int arvoreBInsere(Page **alvo, const Registro reg, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados);
void arvoreBInsereAux(Registro reg, Page **raiz, int *cresceu, Registro *reg_retorno, Page **ap_retorno, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados);
void arvoreBInsereNaPagina(Page *alvo, const Registro reg, Page *ap_dir, uint64_t *qtd_comparacoes);
int64_t arvoreBCalculaAltura(Page *alvo);
void arvoreBComplementoCalculaAltura(Page *alvo, int64_t *altura);
int arvoreBPesquisa(Page *alvo, Registro *reg, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados);
int arvoreBComplementoPesquisa(Page *alvo, Registro *reg, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados);

void arvoreBRetira(size_t id_data_remocao, Page **alvo, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados);
void arvoreBRetiraAux(size_t id_data_remocao, Page **alvo, short *flag_diminuiu, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados);
void arvoreBAntecessor(Page *alvo, size_t index, Page *father, short *flag_diminuiu, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados);
void arvoreBRetiraReconstitui(Page *alvo, Page *father, size_t provavel_index_insercao_gambiarrento, short *flag_diminiu, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados);

