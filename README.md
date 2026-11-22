# O Som das Seis — Gerenciador de Personagens

Projeto acadêmico em **C11** para criação e gerenciamento de fichas do RPG *O Som das Seis*.

## Funcionalidades

- Cadastro de personagens com código único automático.
- Distribuição dos atributos Físico, Agilidade, Intelecto e Coragem.
- Cálculo de PV, Iniciativa, Defesa e Ações em Combate.
- Distribuição de Antecedentes e seleção de habilidades iniciais.
- Sorteio/definição do Tormento pela tabela de 2D6 do livro.
- Registro de reputação, dinheiro, inventário, montarias e Cartas de Sina.
- Consulta por código, listagem, edição de dados e exclusão lógica.
- Gestão de campanha: PV atual, XP, nível, dinheiro, reputação, cartas, itens e montarias.
- Exportação das fichas ativas para arquivo TXT legível.
- Relatório de registros ativos/excluídos e estatísticas.
- Persistência em `personagens.dat` por meio de arquivo binário.

## Compilação

### GCC / MinGW

```bash
gcc -std=c11 -Wall -Wextra -pedantic main.c -o som_das_seis
```

No Linux/macOS:

```bash
./som_das_seis
```

No Windows:

```powershell
.\som_das_seis.exe
```

Também é possível executar `make` em sistemas com Make instalado.

## Arquivos gerados

- `personagens.dat`: banco binário com registros completos. Não editar manualmente.
- `fichas_som_das_seis.txt`: exportação textual padrão (o nome pode ser alterado no menu).

Mantenha o executável e `personagens.dat` no mesmo diretório para continuar usando o mesmo banco.

## Observações sobre regras

O programa usa como referência o livro *O Som das Seis*, de Ramon Mineiro. A criação inicial usa 4 pontos para atributos, 4 pontos-base para Antecedentes, duas habilidades e US$ 150. O Intelecto concede pontos adicionais de Antecedente. O livro apresenta divergência entre a página 18 (4 pontos de Antecedentes) e o resumo da página 47 (5 pontos); por isso, o programa adota 4 como padrão, coerente com a seção inicial de criação, e informa os pontos adicionais de Intelecto separadamente.

Os valores derivados são calculados na criação. A progressão de nível é editável, pois a evolução deve acompanhar a decisão da Juíza e as regras da campanha. As descrições extensas das habilidades não são reproduzidas integralmente; seus nomes oficiais são disponibilizados para seleção e registro.

## Persistência e exclusão lógica

Cada personagem é gravada como um registro `struct` em `personagens.dat`. A exclusão altera `ativo` de 1 para 0, sem apagar o registro. A consulta, listagem, contagem e exportação consideram apenas registros ativos.

## Limitações conhecidas

- O formato binário grava a representação da `struct` da compilação atual. Se a estrutura for modificada em versões futuras, faça backup e migre/apague o banco antigo.
- Os valores de dano, regras de combate e efeitos de habilidades são anotados/consultados na mesa; este programa é um gerenciador de fichas, não um simulador integral de combate.
- O Tormento é escolhido pela tabela apresentada no livro; o usuário informa os resultados dos dois D6 ou escolhe os valores manualmente.
