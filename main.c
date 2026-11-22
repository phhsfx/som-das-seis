/*
 * O SOM DAS SEIS - Gerenciador de Personagens
 * Projeto acadêmico em C (C11), com persistência binária e exportação TXT.
 * Regras de criação inspiradas no livro O Som das Seis, de Ramon Mineiro.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define DB_FILE "personagens.dat"
#define MAX_NOME 64
#define MAX_TEXTO 160
#define MAX_ANTECEDENTES 8
#define MAX_HABILIDADES 4
#define MAX_ITENS 20
#define MAX_MONTARIAS 3
#define MAX_CARTAS 2
#define MAX_REGISTROS 500

static const char *ANTECEDENTES[MAX_ANTECEDENTES] = {
    "Combate", "Labuta", "Negocios", "Exploracao",
    "Montaria", "Medicina", "Tradicao", "Roubo"
};
static const char *HABILIDADES[] = {
    "Light My Fire", "Let's Dance", "Fortunate Son", "Don't Stop Believing",
    "Immigrant Song", "Gimme Shelter", "Another One Bites the Dust",
    "Riders on the Storm", "Born to Be Wild", "Smoke on the Water",
    "Under Pressure", "Heartbreaker", "Barracuda", "Sweet Emotion",
    "Crazy Train", "Carry On My Wayward Son", "War Pigs", "Ace of Spades",
    "A Horse with No Name", "I Want to Hold Your Hand", "Paranoid", "Ramble On",
    "Aqualung", "More Than a Feeling"
};
#define QTD_HABILIDADES ((int)(sizeof(HABILIDADES)/sizeof(HABILIDADES[0])))

/* Registro binário: o status permite exclusão lógica sem remover bytes do arquivo. */
typedef struct {
    int id;
    int ativo;
    char nome[MAX_NOME];
    char jogador[MAX_NOME];
    int idade;
    char origem[MAX_TEXTO];
    char descricao[MAX_TEXTO];
    int nivel, xp;
    int fisico, agilidade, intelecto, coragem;
    int antecedentes[MAX_ANTECEDENTES];
    int pv_atual, pv_max;
    int iniciativa, defesa, acoes;
    char tormento[MAX_TEXTO];
    char reputacao[MAX_TEXTO];
    int reputacao_valor; /* -1, 0, +1 */
    char habilidades[MAX_HABILIDADES][MAX_TEXTO];
    int qtd_habilidades;
    double dinheiro;
    char itens[MAX_ITENS][MAX_TEXTO];
    int qtd_itens;
    char montarias[MAX_MONTARIAS][MAX_TEXTO];
    int qtd_montarias;
    char cartas[MAX_CARTAS][16];
    int qtd_cartas;
    char anotacoes[MAX_TEXTO * 2];
} Personagem;

static void pausar(void) { printf("\nPressione ENTER para continuar..."); int c; while ((c=getchar())!='\n' && c!=EOF) {} }
static void linha(void) { puts("======================================================================"); }
static void limpar_tela(void) {
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}
static void ler_texto(const char *rotulo, char *dest, size_t tam) {
    printf("%s", rotulo);
    if (fgets(dest, (int)tam, stdin)) {
        size_t n=strlen(dest); if(n && dest[n-1]=='\n') dest[n-1]='\0';
        else { int c; while((c=getchar())!='\n' && c!=EOF){} }
    }
}
static int ler_int(const char *rotulo, int min, int max) {
    char buf[80], *fim; long v;
    for (;;) {
        printf("%s", rotulo);
        if (!fgets(buf,sizeof buf,stdin)) return min;
        v=strtol(buf,&fim,10);
        while(isspace((unsigned char)*fim)) fim++;
        if(fim!=buf && *fim=='\0' && v>=min && v<=max) return (int)v;
        printf("Valor invalido. Informe um numero entre %d e %d.\n",min,max);
    }
}
static double ler_double(const char *rotulo, double min, double max) {
    char buf[80], *fim; double v;
    for (;;) {
        printf("%s",rotulo);
        if(!fgets(buf,sizeof buf,stdin)) return min;
        for(char *p=buf;*p;p++) if(*p==',') *p='.';
        v=strtod(buf,&fim); while(isspace((unsigned char)*fim)) fim++;
        if(fim!=buf && *fim=='\0' && v>=min && v<=max) return v;
        printf("Valor invalido. Informe um valor entre %.2f e %.2f.\n",min,max);
    }
}
static FILE *abrir_db(const char *modo) { return fopen(DB_FILE,modo); }
static int proximo_id(void) {
    FILE *f=abrir_db("rb"); Personagem p; int maior=0;
    if(!f) return 1;
    while(fread(&p,sizeof p,1,f)==1) if(p.id>maior) maior=p.id;
    fclose(f); return maior+1;
}
static int localizar(int id, Personagem *saida, long *posicao) {
    FILE *f=abrir_db("rb"); Personagem p; long pos=0;
    if(!f) return 0;
    while(fread(&p,sizeof p,1,f)==1) {
        if(p.id==id && p.ativo) { if(saida)*saida=p; if(posicao)*posicao=pos; fclose(f); return 1; }
        pos += (long)sizeof p;
    }
    fclose(f); return 0;
}
static int gravar_novo(const Personagem *p) {
    FILE *f=abrir_db("ab"); if(!f) { perror("Nao foi possivel abrir o banco"); return 0; }
    int ok=fwrite(p,sizeof *p,1,f)==1; fclose(f); return ok;
}
static int atualizar_registro(const Personagem *p) {
    FILE *f=abrir_db("r+b"); long pos;
    if(!f) return 0;
    if(!localizar(p->id,NULL,&pos)) { fclose(f); return 0; }
    if(fseek(f,pos,SEEK_SET)!=0) { fclose(f); return 0; }
    int ok=fwrite(p,sizeof *p,1,f)==1; fclose(f); return ok;
}
static int contar_ativos(void) {
    FILE *f=abrir_db("rb"); Personagem p; int n=0;
    if(!f) return 0;
    while(fread(&p,sizeof p,1,f)==1) if(p.ativo) n++;
    fclose(f); return n;
}
static void exibir_menu(void) {
    limpar_tela(); linha();
    puts("                 O SOM DAS SEIS | REGISTRO DE PERSONAGENS");
    linha();
    printf(" Personagens ativos: %d\n\n",contar_ativos());
    puts("  1. Criar personagem");
    puts("  2. Consultar personagem por codigo");
    puts("  3. Listar personagens");
    puts("  4. Editar dados de personagem");
    puts("  5. Excluir personagem (logicamente)");
    puts("  6. Gerenciar ficha durante a campanha");
    puts("  7. Exportar fichas para TXT");
    puts("  8. Relatorio geral / estatisticas");
    puts("  0. Sair");
    linha();
}
static void mostrar_ficha(const Personagem *p) {
    linha(); printf(" FICHA #%d | %s\n",p->id,p->nome); linha();
    printf("Jogador: %s | Idade: %d | Nivel: %d | XP: %d\n",p->jogador,p->idade,p->nivel,p->xp);
    printf("Origem: %s\nConceito: %s\n",p->origem,p->descricao);
    linha(); puts("ATRIBUTOS");
    printf("Fisico: %d | Agilidade: %d | Intelecto: %d | Coragem: %d\n",p->fisico,p->agilidade,p->intelecto,p->coragem);
    printf("PV: %d/%d | Iniciativa: %d | Defesa: %d | Acoes em combate: %d\n",p->pv_atual,p->pv_max,p->iniciativa,p->defesa,p->acoes);
    linha(); puts("ANTECEDENTES");
    for(int i=0;i<MAX_ANTECEDENTES;i++) printf("%-13s %d%s",ANTECEDENTES[i],p->antecedentes[i],i%2?"\n":" | ");
    linha(); printf("TORmento: %s\nReputacao: %s (%+d)\n",p->tormento,p->reputacao,p->reputacao_valor);
    puts("HABILIDADES:"); for(int i=0;i<p->qtd_habilidades;i++) printf("  - %s\n",p->habilidades[i]);
    printf("Dinheiro: $%.2f\nItens (%d):\n",p->dinheiro,p->qtd_itens);
    for(int i=0;i<p->qtd_itens;i++) printf("  - %s\n",p->itens[i]);
    printf("Montarias (%d):\n",p->qtd_montarias); for(int i=0;i<p->qtd_montarias;i++) printf("  - %s\n",p->montarias[i]);
    printf("Cartas de Sina (%d/%d):\n",p->qtd_cartas,MAX_CARTAS); for(int i=0;i<p->qtd_cartas;i++) printf("  - %s\n",p->cartas[i]);
    printf("Anotacoes: %s\n",p->anotacoes); linha();
}
static void escolher_tormento(char *dest, size_t tam) {
    static const char *t[6][6] = {
      {"Vinganca pelo amor da minha vida","Vinganca pelos meus filhos","Vinganca por minha terra roubada","Vinganca pela morte de um parceiro","Vinganca pela minha tribo","Vinganca pelo fim da minha gangue"},
      {"Fugindo da lei","Fugindo da gangue inimiga","Fugindo de um mercenario","Fugindo de uma familia poderosa","Fugindo de um culto religioso","Fugindo de um casamento"},
      {"Vicio em alcool","Vicio em drogas","Vicio em brigas","Vicio em apostas","Vicio em adrenalina","Vicio em algo bizarro"},
      {"Segredo: sou um infiltrado da lei","Segredo: sou de uma familia rica","Segredo: tenho algo contagioso","Segredo: matei alguem importante","Segredo: tenho uma grande divida","Segredo: tenho um tesouro comigo"},
      {"Doenca na pele do rosto","Doenca na mente","Doenca no intestino","Doenca nas pernas","Doenca nos olhos","Doenca nas maos"},
      {"Dever com a familia","Dever com a lei","Dever com a gangue","Dever com Deus","Dever com meu cavalo","Dever com minha divida"}
    };
    int coluna, linha;
    int modo=ler_int("1-Sortear 2D6 automaticamente | 2-Informar resultados: ",1,2);
    if(modo==1) { coluna=rand()%6; linha=rand()%6; printf("Resultado: %d e %d\n",coluna+1,linha+1); }
    else { coluna=ler_int("Resultado do primeiro D6 (1-6): ",1,6)-1; linha=ler_int("Resultado do segundo D6 (1-6): ",1,6)-1; }
    snprintf(dest,tam,"%s",t[coluna][linha]);
    printf("Tormento definido: %s\n",dest);
}
static void criar_personagem(void) {
    Personagem p; memset(&p,0,sizeof p); p.id=proximo_id(); p.ativo=1; p.nivel=1; p.dinheiro=150.0;
    strcpy(p.reputacao,"Sem reputacao definida"); strcpy(p.anotacoes,"-");
    linha(); puts("CRIACAO DE PERSONAGEM | O SOM DAS SEIS"); linha();
    ler_texto("Nome da personagem: ",p.nome,sizeof p.nome);
    while(!p.nome[0]) ler_texto("O nome nao pode ficar vazio. Nome: ",p.nome,sizeof p.nome);
    ler_texto("Jogador(a): ",p.jogador,sizeof p.jogador);
    p.idade=ler_int("Idade: ",1,120);
    ler_texto("Origem / procedencia: ",p.origem,sizeof p.origem);
    ler_texto("Conceito / breve descricao: ",p.descricao,sizeof p.descricao);
    puts("\nATRIBUTOS: distribua 4 pontos. Cada ponto representa uma melhoria.");
    int restantes=4;
    while(restantes>0) {
        printf("Pontos restantes: %d | Fisico %d, Agilidade %d, Intelecto %d, Coragem %d\n",restantes,p.fisico,p.agilidade,p.intelecto,p.coragem);
        int op=ler_int("1-Fisico  2-Agilidade  3-Intelecto  4-Coragem: ",1,4);
        if(op==1)p.fisico++; else if(op==2)p.agilidade++; else if(op==3)p.intelecto++; else p.coragem++;
        restantes--;
    }
    p.pv_max=6; for(int i=0;i<p.fisico;i++) p.pv_max += 5; /* valor-base; rolagem individual pode ser ajustada */
    printf("PV inicial: regra = 6 + 1d6+2 por ponto em Fisico.\n");
    p.pv_max=6;
    for(int i=0;i<p.fisico;i++) { int dado=ler_int("Resultado de 1D6 para PV (1-6): ",1,6); p.pv_max += dado+2; }
    p.pv_atual=p.pv_max; p.iniciativa=1+p.coragem; p.defesa=5; p.acoes=1+p.agilidade;
    puts("\nANTECEDENTES: distribua 4 pontos. Maximo de 2 em cada antecedente no nivel 1.");
    restantes=4;
    while(restantes>0) {
        printf("Pontos restantes: %d\n",restantes);
        for(int i=0;i<MAX_ANTECEDENTES;i++) printf("%d. %-13s (%d)\n",i+1,ANTECEDENTES[i],p.antecedentes[i]);
        int op=ler_int("Escolha o antecedente: ",1,MAX_ANTECEDENTES)-1;
        if(p.antecedentes[op]>=2) puts("Limite inicial de 2 pontos atingido nesse antecedente.");
        else {p.antecedentes[op]++; restantes--;}
    }
    /* Intelecto concede pontos extras de Antecedente, respeitando o limite inicial. */
    for(int k=0;k<p.intelecto;k++) {
        printf("Ponto adicional de Antecedente concedido por Intelecto (%d/%d).\n",k+1,p.intelecto);
        for(int i=0;i<MAX_ANTECEDENTES;i++) printf("%d. %-13s (%d)\n",i+1,ANTECEDENTES[i],p.antecedentes[i]);
        int op=ler_int("Escolha (1-8): ",1,8)-1; p.antecedentes[op]++;
    }
    puts("\nHABILIDADES: escolha duas habilidades iniciais.");
    for(int i=0;i<QTD_HABILIDADES;i++) printf("%2d. %s\n",i+1,HABILIDADES[i]);
    while(p.qtd_habilidades<2) {
        int op=ler_int("Numero da habilidade: ",1,QTD_HABILIDADES)-1; int duplicada=0;
        for(int j=0;j<p.qtd_habilidades;j++) if(strcmp(p.habilidades[j],HABILIDADES[op])==0) duplicada=1;
        if(duplicada) puts("Habilidade ja selecionada."); else strcpy(p.habilidades[p.qtd_habilidades++],HABILIDADES[op]);
    }
    puts("\nTORmento: escolha sorteio pela tabela do livro (2D6, coluna e linha)."); escolher_tormento(p.tormento,sizeof p.tormento);
    ler_texto("Reputacao inicial (ENTER para 'Sem reputacao definida'): ",p.reputacao,sizeof p.reputacao);
    if(!p.reputacao[0]) strcpy(p.reputacao,"Sem reputacao definida");
    p.reputacao_valor=ler_int("Valor da reputacao (-1, 0 ou 1): ",-1,1);
    puts("\nEquipamento inicial: o personagem comeca com $150. Cadastre itens separados.");
    int mais=ler_int("Deseja cadastrar um item agora? 1-Sim 0-Nao: ",0,1);
    while(mais && p.qtd_itens<MAX_ITENS) { ler_texto("Descricao do item: ",p.itens[p.qtd_itens],MAX_TEXTO); if(p.itens[p.qtd_itens][0])p.qtd_itens++; mais=ler_int("Adicionar outro item? 1-Sim 0-Nao: ",0,1); }
    p.dinheiro=ler_double("Dinheiro restante apos compras ($): ",0,1000000);
    ler_texto("Anotacoes adicionais: ",p.anotacoes,sizeof p.anotacoes);
    if(gravar_novo(&p)) printf("\nPersonagem criada com sucesso! Codigo: %d\n",p.id); else puts("Falha ao salvar personagem.");
}
static void consultar_personagem(void) {
    int id=ler_int("Codigo da personagem: ",1,100000000); Personagem p;
    if(localizar(id,&p,NULL)) mostrar_ficha(&p); else puts("Personagem ativa nao encontrada.");
}
static void listar_personagens(void) {
    FILE *f=abrir_db("rb"); Personagem p; int n=0;
    if(!f) { puts("Ainda nao ha banco de dados."); return; }
    linha(); puts("CODIGO | NOME | JOGADOR | NIVEL | PV | DINHEIRO"); linha();
    while(fread(&p,sizeof p,1,f)==1) if(p.ativo) { printf("%-6d | %-24s | %-18s | %2d | %2d/%-2d | $%.2f\n",p.id,p.nome,p.jogador,p.nivel,p.pv_atual,p.pv_max,p.dinheiro); n++; }
    fclose(f); if(!n) puts("Nenhuma personagem cadastrada.");
}
static void editar_personagem(void) {
    int id=ler_int("Codigo da personagem: ",1,100000000); Personagem p;
    if(!localizar(id,&p,NULL)) { puts("Personagem nao encontrada."); return; }
    puts("ENTER mantem o valor atual para campos textuais."); char b[MAX_TEXTO];
    printf("Nome atual: %s\n",p.nome); ler_texto("Novo nome: ",b,sizeof b); if(b[0]){ strncpy(p.nome,b,sizeof p.nome-1); p.nome[sizeof p.nome-1]='\0'; }
    printf("Jogador atual: %s\n",p.jogador); ler_texto("Novo jogador: ",b,sizeof b); if(b[0]){ strncpy(p.jogador,b,sizeof p.jogador-1); p.jogador[sizeof p.jogador-1]='\0'; }
    printf("Origem atual: %s\n",p.origem); ler_texto("Nova origem: ",b,sizeof b); if(b[0])snprintf(p.origem,sizeof p.origem,"%s",b);
    printf("Descricao atual: %s\n",p.descricao); ler_texto("Nova descricao: ",b,sizeof b); if(b[0])snprintf(p.descricao,sizeof p.descricao,"%s",b);
    if(atualizar_registro(&p)) puts("Ficha atualizada."); else puts("Nao foi possivel atualizar.");
}
static void excluir_personagem(void) {
    int id=ler_int("Codigo a excluir: ",1,100000000); Personagem p;
    if(!localizar(id,&p,NULL)) { puts("Personagem nao encontrada."); return; }
    printf("Confirma exclusao logica de %s? 1-Sim 0-Nao: ",p.nome);
    int op=ler_int("Opcao: ",0,1); if(!op)return; p.ativo=0;
    if(atualizar_registro(&p)) puts("Registro marcado como excluido. Os dados permanecem no arquivo binario."); else puts("Falha na exclusao.");
}
static void gerenciar_campanha(void) {
    int id=ler_int("Codigo da personagem: ",1,100000000); Personagem p;
    if(!localizar(id,&p,NULL)) { puts("Personagem nao encontrada."); return; }
    int op;
    do {
        linha(); printf("CAMPANHA | %s (#%d)\n",p.nome,p.id); linha();
        printf("1. Ajustar PV (%d/%d)\n2. Adicionar XP (%d)\n3. Alterar nivel (%d)\n4. Dinheiro ($%.2f)\n5. Reputacao (%s, %+d)\n6. Cartas de Sina (%d/2)\n7. Adicionar item\n8. Remover item\n9. Adicionar montaria\n10. Anotacoes\n0. Salvar e voltar\n",p.pv_atual,p.pv_max,p.xp,p.nivel,p.dinheiro,p.reputacao,p.reputacao_valor,p.qtd_cartas);
        op=ler_int("Opcao: ",0,10);
        if(op==1) p.pv_atual=ler_int("PV atual: ",0,p.pv_max);
        else if(op==2) p.xp+=ler_int("XP a adicionar: ",0,10000);
        else if(op==3) {p.nivel=ler_int("Novo nivel: ",1,20); puts("Ajuste manualmente atributos e habilidades conforme a progressao da mesa.");}
        else if(op==4) p.dinheiro=ler_double("Novo saldo ($): ",0,1000000);
        else if(op==5) {ler_texto("Descricao da reputacao: ",p.reputacao,sizeof p.reputacao); p.reputacao_valor=ler_int("Valor (-1, 0, 1): ",-1,1);}
        else if(op==6) {
            if(p.qtd_cartas>=MAX_CARTAS) puts("Limite de duas Cartas de Sina atingido nesta sessao.");
            else {char c[16]; ler_texto("Carta (ex.: A de Copas): ",c,sizeof c); if(c[0])strcpy(p.cartas[p.qtd_cartas++],c);}
        }
        else if(op==7) {if(p.qtd_itens<MAX_ITENS){ler_texto("Novo item: ",p.itens[p.qtd_itens],MAX_TEXTO); if(p.itens[p.qtd_itens][0])p.qtd_itens++;}else puts("Inventario cheio.");}
        else if(op==8) {for(int i=0;i<p.qtd_itens;i++)printf("%d. %s\n",i+1,p.itens[i]); if(p.qtd_itens){int k=ler_int("Numero do item a remover: ",1,p.qtd_itens)-1; for(int i=k;i<p.qtd_itens-1;i++)strcpy(p.itens[i],p.itens[i+1]); p.qtd_itens--;}}
        else if(op==9) {if(p.qtd_montarias<MAX_MONTARIAS){ler_texto("Nome e descricao da montaria: ",p.montarias[p.qtd_montarias],MAX_TEXTO); if(p.montarias[p.qtd_montarias][0])p.qtd_montarias++;}else puts("Limite de montarias atingido.");}
        else if(op==10) ler_texto("Anotacoes: ",p.anotacoes,sizeof p.anotacoes);
    } while(op!=0);
    if(atualizar_registro(&p)) puts("Alteracoes salvas no arquivo binario."); else puts("Erro ao salvar alteracoes.");
}
static void exportar_um(FILE *f,const Personagem *p) {
    fprintf(f,"O SOM DAS SEIS - FICHA DE PERSONAGEM\nCodigo: %d\nNome: %s\nJogador: %s\nIdade: %d\nOrigem: %s\nConceito: %s\nNivel: %d | XP: %d\n\n",p->id,p->nome,p->jogador,p->idade,p->origem,p->descricao,p->nivel,p->xp);
    fprintf(f,"ATRIBUTOS\nFisico: %d | Agilidade: %d | Intelecto: %d | Coragem: %d\nPV: %d/%d | Iniciativa: %d | Defesa: %d | Acoes: %d\n\nANTECEDENTES\n",p->fisico,p->agilidade,p->intelecto,p->coragem,p->pv_atual,p->pv_max,p->iniciativa,p->defesa,p->acoes);
    for(int i=0;i<MAX_ANTECEDENTES;i++)fprintf(f,"%s: %d\n",ANTECEDENTES[i],p->antecedentes[i]);
    fprintf(f,"\nTormento: %s\nReputacao: %s (%+d)\nHabilidades:\n",p->tormento,p->reputacao,p->reputacao_valor);
    for(int i=0;i<p->qtd_habilidades;i++)fprintf(f,"- %s\n",p->habilidades[i]);
    fprintf(f,"\nDinheiro: $%.2f\nItens:\n",p->dinheiro); for(int i=0;i<p->qtd_itens;i++)fprintf(f,"- %s\n",p->itens[i]);
    fprintf(f,"Montarias:\n"); for(int i=0;i<p->qtd_montarias;i++)fprintf(f,"- %s\n",p->montarias[i]);
    fprintf(f,"Cartas de Sina: %d/2\n",p->qtd_cartas); for(int i=0;i<p->qtd_cartas;i++)fprintf(f,"- %s\n",p->cartas[i]);
    fprintf(f,"Anotacoes: %s\n\n",p->anotacoes); linha(); fprintf(f,"\n");
}
static void exportar_txt(void) {
    char nome[128]; ler_texto("Nome do arquivo (ENTER = fichas_som_das_seis.txt): ",nome,sizeof nome);
    if(!nome[0])strcpy(nome,"fichas_som_das_seis.txt");
    FILE *db=abrir_db("rb"); if(!db){puts("Nao ha registros para exportar.");return;}
    FILE *out=fopen(nome,"w"); if(!out){perror("Falha ao criar TXT");fclose(db);return;}
    Personagem p; int n=0; while(fread(&p,sizeof p,1,db)==1)if(p.ativo){exportar_um(out,&p);n++;}
    fclose(db); fclose(out); printf("Exportacao concluida: %d ficha(s) em %s\n",n,nome);
}
static void relatorio(void) {
    FILE *f=abrir_db("rb"); Personagem p; int ativos=0, excluidos=0, soma_nivel=0; double dinheiro=0;
    if(f){while(fread(&p,sizeof p,1,f)==1){if(p.ativo){ativos++;soma_nivel+=p.nivel;dinheiro+=p.dinheiro;}else excluidos++;}fclose(f);}
    linha(); puts("RELATORIO DO BANCO DE PERSONAGENS"); linha();
    printf("Registros ativos: %d\nRegistros excluidos logicamente: %d\n",ativos,excluidos);
    if(ativos)printf("Nivel medio: %.2f\nDinheiro total das personagens: $%.2f\n",(double)soma_nivel/ativos,dinheiro);
    else puts("Nenhuma personagem ativa cadastrada.");
    printf("Arquivo de dados: %s\nTamanho do registro: %lu bytes\n",DB_FILE,(unsigned long)sizeof(Personagem));
}
int main(void) {
    srand((unsigned)time(NULL));
    int op;
    do {
        exibir_menu(); op=ler_int("Escolha uma opcao: ",0,8);
        switch(op) {
            case 1: criar_personagem(); pausar(); break;
            case 2: consultar_personagem(); pausar(); break;
            case 3: listar_personagens(); pausar(); break;
            case 4: editar_personagem(); pausar(); break;
            case 5: excluir_personagem(); pausar(); break;
            case 6: gerenciar_campanha(); pausar(); break;
            case 7: exportar_txt(); pausar(); break;
            case 8: relatorio(); pausar(); break;
            case 0: puts("Encerrando. Que a estrada seja gentil, cowboy."); break;
        }
    } while(op!=0);
    return 0;
}
