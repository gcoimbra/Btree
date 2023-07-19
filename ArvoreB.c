/* 
 * File:   ArvoreB.c
 * Authors: Daniel Freitas Martins  -   2304
 *          Maria Dalila            -   3030
 *          Gabriel Teixeira Pinto  -   3044
 *
 */

// Agradecimentos ao Prof Nívio Ziviani, códigos obtidos de
// http://www2.dcc.ufmg.br/livros/algoritmos/transparencias.php

#include "ArvoreB.h"

/**
 * Procedimento responsável por preparar a árvore B.
 * @param alvo - Endereço para um ponteiro do tipo Page. Representa o endereço da raíz.
 */
void arvoreBCria(Page **alvo){
    
    (*alvo) = NULL;    
}

/**
 * Função responsável por inserir um registro na árvore.
 * @param alvo - Endereço para um ponteiro do tipo Page. Representa o endereço da raíz.
 * @param reg - Registro a ser inserido.
 * @param qtd_comparacoes - Endereço para size_t que irá armazenar a quantidade de condições avaliadas (Ex.: IF/ELSE) para realizar a inserção do registro.
 * @param qtd_nos_visitados - Endereço para size_t que irá armazenar a quantidade de nós (páginas) acessados para realizar a inserção do registro.
 * @return true se a operação foi realizada com sucesso.
 */
int arvoreBInsere(Page **alvo, const Registro reg, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados){
    int cresceu;
    Registro reg_retorno;
    Page *ap_retorno, *nova_pagina;
    (*qtd_comparacoes) = 0;
    (*qtd_nos_visitados) = 0;
    
    arvoreBInsereAux(reg, alvo, &cresceu, &reg_retorno, &ap_retorno, qtd_comparacoes, qtd_nos_visitados);
    if(cresceu){ // Se a árvore cresceu pela raíz...

        nova_pagina = (Page *) malloc(sizeof(Page));
        
        littleCria(&(nova_pagina->item));
        littleInsere(&(nova_pagina->item), reg_retorno, qtd_comparacoes); // Aqui já faz littleSizeSet(&(nova_pagina->item), 1);
        nova_pagina->qtd_filhos = 2;
        nova_pagina->filho[0] = *alvo;
        nova_pagina->filho[1] = ap_retorno;
        *alvo = nova_pagina;
        (*qtd_nos_visitados)++; // Nova página foi criada, então mais um acesso a disco foi feito... /////////////// VERIFICAR
    }

    return true;
}

/**
 * Procedimento auxiliar para inserção de registros na árvore.
 * @param reg - Registro a ser inserido. 
 * @param alvo - Endereço para um ponteiro do tipo Page. 
 * @param cresceu - Endereço da variável que irá armazenar a informação se a árvore aumentou sua altura ou não.
 * @param reg_retorno - Endereço que irá armazenar o registro de retorno para caso a árvore aumente sua altura.
 * @param ap_retorno - Endereço para um ponteiro do tipo Page. Representa o endereço da última página à direita.
 * @param qtd_comparacoes - Endereço para size_t que irá armazenar a quantidade de condições avaliadas (Ex.: IF/ELSE) para realizar a inserção do registro.
 * @param qtd_nos_visitados - Endereço para size_t que irá armazenar a quantidade de nós (páginas) acessados para realizar a inserção do registro.
 */
void arvoreBInsereAux(const Registro reg, Page **alvo, int *cresceu, Registro *reg_retorno, Page **ap_retorno, uint64_t (*qtd_comparacoes), uint64_t *qtd_nos_visitados){
    long i = 0;
    int k;
    Page *page_temp;
    short int tricotomia; // Variável que indica se o elemento a ser inserido é igual a um já presente na árvore.
    (*qtd_comparacoes)++;
    if(*alvo == NULL){
        *cresceu = true;
        *reg_retorno = reg;
        *ap_retorno = NULL;
        return;
    }
    (*qtd_nos_visitados)++;
    i = littlePesquisa(&(*alvo)->item, reg.id_data, &tricotomia, false, qtd_comparacoes);

    (*qtd_comparacoes)++;
    if(tricotomia == IGUAL){
        printf("arvoreBInsereAux(): ERRO registro %zu já presente\n", reg.id_data);
        *cresceu = false;
        return;
    }

    //if(tricotomia == MAIOR)
    //    i--;

    arvoreBInsereAux(reg, &((*alvo)->filho[i]), cresceu, reg_retorno, ap_retorno, qtd_comparacoes, qtd_nos_visitados);

    (*qtd_comparacoes)++;
    if(!*cresceu) // if(*cresceu == FALSE)
        return;
    // else
    (*qtd_comparacoes)++;
    // Se a página tem espaço, então basta inserir nela...
    if(littleSizeGet(&((*alvo)->item)) < BSIZE_NODE){
        arvoreBInsereNaPagina(*alvo, *reg_retorno, *ap_retorno, qtd_comparacoes);
        *cresceu = false;
        return;
    } // else

    page_temp = (Page *) malloc(sizeof(Page));
    page_temp->qtd_filhos = 0;
    littleCria(&(page_temp->item)); // preparando criação do vetor de elementos da página.
    page_temp->filho[0] = NULL;
    (*qtd_nos_visitados)++; // Nova página foi criada, então mais um acesso a disco foi feito... /////////////// VERIFICAR
    (*qtd_comparacoes)++;
    if(i < BSIZE + 1){
        arvoreBInsereNaPagina(page_temp, littleGet(&(*alvo)->item, BSIZE_NODE - 1) , (*alvo)->filho[BSIZE_NODE], qtd_comparacoes);        
        littleSizeSet(&((*alvo)->item), littleSizeGet(&((*alvo)->item)) - 1);
        arvoreBInsereNaPagina(*alvo, *reg_retorno, *ap_retorno, qtd_comparacoes);
    } else{
        arvoreBInsereNaPagina(page_temp, *reg_retorno, *ap_retorno, qtd_comparacoes);
    }
    
    for (k = BSIZE + 2; k <= BSIZE_NODE; k++){
        (*qtd_comparacoes)++; // Considerando comparação do for...
        arvoreBInsereNaPagina(page_temp, littleGet(&(*alvo)->item, (size_t) k - 1), (*alvo)->filho[k], qtd_comparacoes);
    }
    (*qtd_comparacoes)++; // Considerando a última comparação do for...

    littleSizeSet(&((*alvo)->item), BSIZE);
    page_temp->filho[0] = (*alvo)->filho[BSIZE+1];
    *reg_retorno = littleGet(&(*alvo)->item, BSIZE);
    *ap_retorno = page_temp;
}

/**
 * Procedimento responsável pela inserção do registro dada uma página com espaço disponível.
 * @param alvo - Endereço de Page. Representa a página que possui espaço disponível.
 * @param reg - Registro a ser inserido na página.
 * @param ap_dir - Endereço de Page. Representa a página filha mais a direita.
 * @param qtd_comparacoes - Endereço para size_t que irá armazenar a quantidade de condições avaliadas (Ex.: IF/ELSE) para realizar a inserção do registro.
 */
void arvoreBInsereNaPagina(Page *alvo, const Registro reg,  Page *ap_dir, uint64_t *qtd_comparacoes){
    short int tricotomia;
    int64_t k, i_insercao;

    i_insercao = littlePesquisa(&(alvo->item), reg.id_data, &tricotomia, false, qtd_comparacoes);

    for(k = BSIZE_NODE; k > i_insercao; k--){
        (*qtd_comparacoes)++; // Considerando comparação do for...
        alvo->filho[k+1] = alvo->filho[k];
    }
    (*qtd_comparacoes)++; // Considerando a última comparação do for...
    littleInsere(&(alvo->item), reg, qtd_comparacoes);
    alvo->filho[i_insercao + 1] = ap_dir;
}

/**
 * Função responsável por calcular a altura da árvore B.
 * @param alvo - Endereço de Page. Representa a raíz da árvore.
 * @return A altura da árvore B.
 */
int64_t arvoreBCalculaAltura(Page *alvo){
    int64_t altura;
    altura = -1;
    arvoreBComplementoCalculaAltura(alvo, &altura);
    return altura;
}

/**
 * Procedimento auxiliar para calcular a altura da árvore B.
 * @param alvo - Endereço de Page. Representa a subárvore a ser considerada.
 * @param altura - Endereço da variável que irá computar a altura.
 */
void arvoreBComplementoCalculaAltura(Page *alvo, int64_t *altura){
    if(alvo != NULL){
        *altura += 1;
        arvoreBComplementoCalculaAltura(alvo->filho[0], altura);
    }
}

/**
 * Função responsável por pesquisar por um registro na página. A pesquisa é feita com base na chave do Registro, apenas.
 * @param alvo - Endereço de Page. Representa a página em que será iniciada a pesquisa.
 * @param reg - Endereço do registro a ser procurado (pela chave). Ao ser encontrado, ele receberá os dados do registro encontrado.
 * @param qtd_comparacoes - Endereço para size_t que irá armazenar a quantidade de condições avaliadas (Ex.: IF/ELSE) para se encontrar o registro.
 * @param qtd_nos_visitados - Endereço para size_t que irá armazenar a quantidade de nós (páginas) acessados para se encontrar o registro.
 * @return true se o registro foi encontrado; false caso contrário.
 */
//int arvoreBPesquisa(Page *alvo, Registro *reg, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados){
//    int encontrou;
//    (*qtd_comparacoes) = 0;
//    (*qtd_nos_visitados) = 0;
//
//    encontrou = arvoreBComplementoPesquisa(alvo, reg, qtd_comparacoes, qtd_nos_visitados);
//
//    return encontrou;
//    //    (*qtd_comparacoes) = 0;
////    (*qtd_nos_visitados) = 0; // == qtd_paginas_visitadas
////    return arvoreBComplementoPesquisa(alvo, reg, qtd_comparacoes, qtd_nos_visitados);
//}

int arvoreBPesquisa(Page *alvo, Registro *reg, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados){

    (*qtd_comparacoes) = 0;
    (*qtd_nos_visitados) = 0;
    return arvoreBComplementoPesquisa(alvo,reg,qtd_comparacoes,qtd_nos_visitados);
}


/**
 * Procedimento auxiliar responsável por pesquisar por um registro na página. A pesquisa é feita com base na chave do Registro, apenas.
 * @param alvo - Endereço de Page. Representa a página em que será iniciada a pesquisa.
 * @param reg - Endereço do registro a ser procurado (pela chave). Ao ser encontrado, ele receberá os dados do registro encontrado.
 * @param qtd_comparacoes - Endereço para size_t que irá armazenar a quantidade de condições avaliadas (Ex.: IF/ELSE) para se encontrar o registro.
 * @param qtd_nos_visitados - Endereço para size_t que irá armazenar a quantidade de nós (páginas) acessados para se encontrar o registro.
 * @return true se o registro foi encontrado; false caso contrário.
 */
int arvoreBComplementoPesquisa(Page *alvo, Registro *reg, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados){
    short int tricotomia;
    int64_t index;
    (*qtd_comparacoes)++;
    if(alvo == NULL){
        printf("arvoreBPesquisa(): Registro %zu não está na árvore!\n", reg->id_data);
        return false;
    }
    (*qtd_nos_visitados)++;
    index = littlePesquisa(&(alvo->item), reg->id_data, &tricotomia, false, qtd_comparacoes);
    (*qtd_comparacoes)++;
    if(tricotomia == IGUAL){
        // Encontramos no filho atual;
        *reg = (alvo->item).itens[index];
        printf("arvoreBPesquisa(): Registro %zu encontrado!\n", reg->id_data);
    } else{ // Está nos filhos
        return arvoreBComplementoPesquisa(alvo->filho[index], reg,qtd_comparacoes,qtd_nos_visitados);
    }

    return true;

}

/**
 * Procedimento responsável pela remoção de um dado elemento.
 * @param id_data_remocao - Chave do registro a ser removido.
 * @param alvo - Endereço para um ponteiro de Page. Representa a raíz da árvore.
 * @param qtd_comparacoes - Endereço para size_t que irá armazenar a quantidade de condições avaliadas (Ex.: IF/ELSE) para realizar a remoção.
 * @param qtd_nos_visitados - Endereço para size_t que irá armazenar a quantidade de nós (páginas) acessados para realizar a remoção.
 */
void arvoreBRetira(size_t id_data_remocao, Page **alvo, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados){  
    short flag_diminuiu;
    Page *page_temp;
    Registro reg;
    (*qtd_comparacoes) = 0;
    (*qtd_nos_visitados) = 0;

    arvoreBRetiraAux(id_data_remocao, alvo, &flag_diminuiu, qtd_comparacoes, qtd_nos_visitados);

    (*qtd_comparacoes)++;
    if (flag_diminuiu && littleSizeGet(&((*alvo)->item)) == 0){  /* Arvore diminui na altura */

        page_temp = *alvo;
        *alvo = page_temp->filho[0];
        free(page_temp);
    }
}

/**
 * Procedimento auxiliar para remoção de um dado elemento.
 * @param id_data_remocao - Chave do registro a ser removido.
 * @param alvo - Endereço para um ponteiro de Page. Representa a subárvore de procura para a remoção.
 * @param flag_diminuiu - Flag que indica se a árvore sofreu diminuição em sua altura.
 * @param qtd_comparacoes - Endereço para size_t que irá armazenar a quantidade de condições avaliadas (Ex.: IF/ELSE) para realizar a remoção.
 * @param qtd_nos_visitados - Endereço para size_t que irá armazenar a quantidade de nós (páginas) acessados para realizar a remoção.
 */
void arvoreBRetiraAux(size_t id_data_remocao, Page **alvo, short *flag_diminuiu, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados){
    long int j, index = 1;
    Page *page_temp;

    (*qtd_comparacoes)++;
    if (*alvo == NULL){
        printf("arvoreBRetiraAux(): Erro: registro não está na árvore!\n");
        *flag_diminuiu = 0; // FALSE
        return;
    }
    (*qtd_nos_visitados)++;
    
    page_temp = *alvo;
    while (index < littleSizeGet(&(page_temp->item)) && id_data_remocao > (page_temp->item).itens[index-1].id_data){
        (*qtd_comparacoes) += 2;
        index++;
    }

    (*qtd_comparacoes) += 2; // Considerando a última comparação feita no while e do if que vem na sequência...

    if (id_data_remocao == (page_temp->item).itens[index-1].id_data){ // Se foi encontrado a chave de remoção, então...

        (*qtd_comparacoes)++;
        if (page_temp->filho[index-1] == NULL){   // Se a página é uma página folha, apenas remova o elemento
            
            littleSizeSet(&(page_temp->item), littleSizeGet(&(page_temp->item)) - 1);
            *flag_diminuiu = (littleSizeGet(&(page_temp->item)) < BSIZE); // trata violações da estrutura da árvore quanto à altura.
            for (j = index; j <= littleSizeGet(&(page_temp->item)); j++){
                (*qtd_comparacoes)++;
                (page_temp->item).itens[j-1] = (page_temp->item).itens[j];
                page_temp->filho[j] = page_temp->filho[j+1];
            }
            (*qtd_comparacoes)++;
            return;
        } // else
        
        (*qtd_nos_visitados)++; // Se refere ao if anterior...
        // Página não é folha: realize a troca com o antecessor... 
        arvoreBAntecessor(*alvo, index, page_temp->filho[index-1], flag_diminuiu, qtd_comparacoes, qtd_nos_visitados);
        (*qtd_comparacoes)++;
        if (*flag_diminuiu)
            arvoreBRetiraReconstitui(page_temp->filho[index-1], *alvo, index - 1, flag_diminuiu, qtd_comparacoes, qtd_nos_visitados);
        return;
    } // else

    (*qtd_comparacoes)++;
    if (id_data_remocao > (page_temp->item).itens[index-1].id_data)
        index++;

    arvoreBRetiraAux(id_data_remocao, &(page_temp->filho[index-1]), flag_diminuiu, qtd_comparacoes, qtd_nos_visitados);

    (*qtd_comparacoes)++;
    if (*flag_diminuiu)
        arvoreBRetiraReconstitui(page_temp->filho[index-1], *alvo, index - 1, flag_diminuiu, qtd_comparacoes, qtd_nos_visitados);
}

/**
 * Procedimento auxiliar para encontrar o elemento que possui a maior chave menor que a chave do registro a ser removido.
 * @param alvo - Endereço de Page. Representa a página do registro a ser removido.
 * @param index - Índice do elemento no vetor a ser considerado.
 * @param father - Endereço de Page. Representa a página pai.
 * @param flag_diminuiu - Flag que indica se a árvore sofreu diminuição em sua altura.
 * @param qtd_comparacoes - Endereço para size_t que irá armazenar a quantidade de condições avaliadas (Ex.: IF/ELSE) para realizar a remoção.
 * @param qtd_nos_visitados - Endereço para size_t que irá armazenar a quantidade de nós (páginas) acessados para realizar a remoção.
 */
void arvoreBAntecessor(Page *alvo, size_t index, Page *father, short *flag_diminuiu, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados){
  
    (*qtd_comparacoes)++;
    if (father->filho[littleSizeGet(&(father->item))] != NULL){
        (*qtd_nos_visitados)++;
        arvoreBAntecessor(alvo, index, father->filho[littleSizeGet(&(father->item))], flag_diminuiu, qtd_comparacoes, qtd_nos_visitados);
        (*qtd_comparacoes)++;
        if (*flag_diminuiu)
            arvoreBRetiraReconstitui(father->filho[littleSizeGet(&(father->item))], father, littleSizeGet(&(father->item)), flag_diminuiu, qtd_comparacoes, qtd_nos_visitados);
        return;
    }
    (alvo->item).itens[index-1] = (father->item).itens[littleSizeGet(&(father->item)) - 1];
    littleSizeSet(&(father->item), littleSizeGet(&(father->item)) - 1);
    *flag_diminuiu = (littleSizeGet(&(father->item)) < BSIZE);
}

/**
 * Procedimento auxiliar para reconstituir as propriedades da árvore quando as mesmas são violadas devido o processo de remoção.
 * @param alvo - Endereço de Page. Representa a página que aparenta ter violado as propriedades da árvore B.
 * @param father - Endereço de Page. Representa a página pai de alvo.
 * @param posicao_father - Posição do elemento de father que levou até a página alvo.
 * @param flag_diminiu - Flag que indica se a árvore diminuiu de tamanho.
 * @param qtd_comparacoes - Endereço para size_t que irá armazenar a quantidade de condições avaliadas (Ex.: IF/ELSE) para realizar a remoção.
 * @param qtd_nos_visitados - Endereço para size_t que irá armazenar a quantidade de nós (páginas) acessados para realizar a remoção.
 */
void arvoreBRetiraReconstitui(Page *alvo, Page *father, size_t posicao_father, short *flag_diminiu, uint64_t *qtd_comparacoes, uint64_t *qtd_nos_visitados){
    Page *page_temp;
    long int checa_existencia_elementos_insuficientes, j;
    
    (*qtd_comparacoes)++;
    (*qtd_nos_visitados)++; // Devido ao acesso à irmã...
    if (posicao_father < littleSizeGet(&(father->item))){  // Se a posição do item da página pai a ser considerado for menor que a quantidade de itens da página pai, então...
        
        page_temp = father->filho[posicao_father+1];
        
        // Se o resultado desta divisão inteira der 0, significa que não há elementos suficientes na página...
        checa_existencia_elementos_insuficientes = (littleSizeGet(&(page_temp->item)) - BSIZE + 1) / 2;
        (alvo->item).itens[littleSizeGet(&(alvo->item))] = (father->item).itens[posicao_father];
        alvo->filho[littleSizeGet(&(alvo->item)) + 1] = page_temp->filho[0];
        littleSizeSet(&(alvo->item), littleSizeGet(&(alvo->item)) + 1);

        (*qtd_comparacoes)++;
        if (checa_existencia_elementos_insuficientes > 0){  // Se existe elementos para redistribuir, então...
            
            for (j = 1; j < checa_existencia_elementos_insuficientes; j++){
                (*qtd_comparacoes)++;
                arvoreBInsereNaPagina(alvo, littleGet(&(page_temp->item), j-1), page_temp->filho[j], qtd_comparacoes);            
            }
            (*qtd_comparacoes)++; // Considerando a última comparação feita pelo for...
            
            (father->item).itens[posicao_father] = (page_temp->item).itens[checa_existencia_elementos_insuficientes-1];
            littleSizeSet(&(page_temp->item), littleSizeGet(&(page_temp->item)) - checa_existencia_elementos_insuficientes);
      
            for (j = 0; j < littleSizeGet(&(page_temp->item)); j++){
                (*qtd_comparacoes)++;
                (page_temp->item).itens[j] = (page_temp->item).itens[j + checa_existencia_elementos_insuficientes];
            }
            (*qtd_comparacoes)++; // Considerando a última comparação feita pelo for...
            
            for (j = 0; j <= littleSizeGet(&(page_temp->item)); j++){
                (*qtd_comparacoes)++;
                page_temp->filho[j] = page_temp->filho[j + checa_existencia_elementos_insuficientes];
            }
            (*qtd_comparacoes)++; // Considerando a última comparação feita pelo for...

            *flag_diminiu = 0; // FALSE
        } else{ // Senão, concatena...
            
            for (j = 1; j <= BSIZE; j++){
                (*qtd_comparacoes)++;
                arvoreBInsereNaPagina(alvo, littleGet(&(page_temp->item), j-1), page_temp->filho[j], qtd_comparacoes);
            }
            (*qtd_comparacoes)++; // Considerando a última comparação feita pelo for...

            free(page_temp);

            for (j = posicao_father + 1; j < littleSizeGet(&(father->item)); j++){
                (*qtd_comparacoes)++;
                (father->item).itens[j-1] = (father->item).itens[j];
                father->filho[j] = father->filho[j+1];
            }
            (*qtd_comparacoes) += 2; // Considerando a última comparação feita pelo for e do if que vem na sequência...
            littleSizeSet(&(father->item), littleSizeGet(&(father->item)) - 1);

            if (littleSizeGet(&(father->item)) >= BSIZE)
                *flag_diminiu = 0; // FALSE
        }
    } else{ // Senão, considere o elemento mais à direita não vazio...
    
        page_temp = father->filho[posicao_father-1]; 
        checa_existencia_elementos_insuficientes = (littleSizeGet(&(page_temp->item)) - BSIZE + 1) / 2;
        
        for (j = littleSizeGet(&(alvo->item)); j >= 1; j--){
            (*qtd_comparacoes)++;
            (alvo->item).itens[j] = (alvo->item).itens[j-1];
        }
        (*qtd_comparacoes)++; // Considerando a última comparação feita pelo for...
        
        (alvo->item).itens[0] = (father->item).itens[posicao_father-1];
        
        for (j = littleSizeGet(&(alvo->item)); j >= 0; j--){
            (*qtd_comparacoes)++;
            alvo->filho[j+1] = alvo->filho[j];
        }
        (*qtd_comparacoes) += 2; // Considerando a última comparação feita pelo for e do if que vem na sequência...
        
        littleSizeSet(&(alvo->item), littleSizeGet(&(alvo->item)) + 1);
        if (checa_existencia_elementos_insuficientes > 0){ // Se existe elementos para redistribuir, então...
        
            for (j = 1; j < checa_existencia_elementos_insuficientes; j++){
                (*qtd_comparacoes)++;
                arvoreBInsereNaPagina(alvo, littleGet(&(page_temp->item), littleSizeGet(&(page_temp->item)) - j), page_temp->filho[littleSizeGet(&(page_temp->item)) - j], qtd_comparacoes);
            }
            (*qtd_comparacoes)++; // Considerando a última comparação feita pelo for...
            alvo->filho[0] = page_temp->filho[littleSizeGet(&(page_temp->item)) - checa_existencia_elementos_insuficientes + 1];
            (father->item).itens[posicao_father-1] = (page_temp->item).itens[littleSizeGet(&(page_temp->item)) - checa_existencia_elementos_insuficientes];
            littleSizeSet(&(page_temp->item), littleSizeGet(&(page_temp->item)) - checa_existencia_elementos_insuficientes);
            *flag_diminiu = 0; // FALSE
        }
        else{ // Senão, concatena...
         
            for (j = 1; j <= BSIZE; j++){
                (*qtd_comparacoes)++;
                arvoreBInsereNaPagina(page_temp, littleGet(&(alvo->item), j-1), alvo->filho[j], qtd_comparacoes);
            }
            (*qtd_comparacoes) += 2; // Considerando a última comparação feita pelo for e do if que vem na sequência
            
            
            free(alvo);
            littleSizeSet(&(father->item), littleSizeGet(&(father->item)) - 1);
            if (littleSizeGet(&(father->item)) >= BSIZE)
                *flag_diminiu = 0; // FALSE
          }
      }
}
