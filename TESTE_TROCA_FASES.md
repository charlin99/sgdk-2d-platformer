# Troca de fases: teste e autoria

Compile com `powershell -ExecutionPolicy Bypass -File tools/build.ps1` (variável `GDK` apontando para o SGDK). O script valida os mapas antes de chamar o make. A ROM fica em `out/rom.bin`. A validação isolada é `powershell -File tools/validate-stages.ps1`.

## Roteiro no emulador

### Apresentação entre fases

- Na ida da fase 1 para a 2, a cena congela com `LEVEL CLEAR!` por 2 segundos. Jogador, inimigos, tiros, animações e timer ficam parados e os controles ficam bloqueados.
- Após um fade para preto de aproximadamente 0,5 segundo, aparecem `LEVEL 2` e `START!`, centralizados em branco, durante 2 segundos. A nova fase é preparada com a tela apagada e revelada com outro fade de aproximadamente 0,5 segundo; só então os controles voltam.
- No retorno da fase 2 para a 1, a sequência pula `LEVEL CLEAR!` e apresenta `LEVEL 1` / `START!`. Nesta seleção de fases, um destino com ID maior representa avanço e um ID menor representa retorno.
- Os tempos usam 120 quadros em NTSC e 100 em PAL. Apertar START/B durante as mensagens não deve pausar nem disparar um salto ao terminar; manter uma direção pressionada permite andar quando a fase começa.
- Verifique a ausência de flashes da fase nova antes do fade, a preservação de vidas/score e o reinício de saúde/timer. A entrada inicial pelo título e o respawn mantêm seu fluxo anterior.
- Compilação da sequência concluída. A conferência visual e de duração no emulador está pendente. O validador de mapas atualmente rejeita a saída editada da fase 2: X=688,932 e largura=7,25 ultrapassam a borda de 696 px; as saídas atuais também possuem coordenadas fracionárias. Os TMX foram preservados nesta alteração.

### Percurso

1. Inicie o jogo normalmente. O HUD identifica `FASE: 1`.
2. Na fase original (`level_test2.tmx`), alcance a segunda sala e a área X=352..376, Y=144..176. Basta sobrepor o jogador à área: ela leva à fase 2. A saída é um trigger sem arte de porta.
3. A fase nova (`level_transition_test.tmx`) mede 696 × 200 px. O jogador aparece em (400,168), na segunda sala, com câmera X=320. Não há slimes, checkpoints ou canhões. Vidas e score permanecem; saúde volta a 2 e timer a 120.
4. Ande para a esquerda para testar o retorno à primeira sala. À direita, a última vista termina em X=376: a câmera deve percorrer os 56 pixels finais sem travar ou deslocar o jogador para outra posição do mundo.
5. O buraco em X=560..592 permite testar morte e respawn na fase 2. Caia nele; o jogador deve retornar a (400,168), sem usar checkpoints da fase 1. Para passar, salte com B; A permite correr.
6. Depois do buraco, a saída de retorno está em X=672..696, Y=152..184, junto às duas marcas de grama. Ela retorna ao spawn original da fase 1. Repita a ida e volta e verifique ausência de sprites/tiros antigos.
7. Confira pausa, morte por tempo, game over e reinício. A saída não deve atuar durante pausa/tela de vidas nem quando a morte ocorre no mesmo quadro. Reiniciar após game over retorna à fase 1.

## Adicionar saídas e fases

- Cadastre cada fase em `src/stage.c`, com ID único, largura exata do TMX em pixels, mapa, tileset e ponteiros/contagens de objetos. Listas vazias usam `NULL, 0`, sem objetos fictícios nem arrays de tamanho zero. Nesta versão há limite de 16 canhões por fase.
- Declare os recursos em `res/resources.res`; `res/resources.h` é gerado pelo rescomp. As duas fases compartilham `level_tileset`, declarado diretamente a partir de `tileset.png`: o rescomp elimina os tilesets duplicados dos mapas. Essa declaração também faz o build acompanhar alterações na imagem, que não entra automaticamente nas dependências do TMX. Os marcadores ficam invisíveis pelo código, sem depender de `tileset_invisible.png`.
- Crie exatamente um `PlayerSpawn` na camada `Spawns`, com `exportPosition = true`.
- Na camada `StageExits`, use retângulos sem rotação, com dimensões inteiras positivas, `exportPosition = true`, **`exportSize = true`** e `nextStage` inteiro. IDs começam em 1; 0 não encerra a campanha. Não sobreponha saídas e mantenha o spawn fora delas.
- As coordenadas são do mundo, sem HUD/câmera. `exportSize` fornece `width` e `height`; não são necessárias propriedades duplicadas de dimensões.
- Objetos da ROM são imutáveis. Ao revisitar uma fase, inimigos e checkpoints são recriados. Cada troca descarta o checkpoint anterior.

## Verificações realizadas

- Build SGDK 2.11 / rescomp 3.95 concluído, gerando `out/rom.bin`.
- Geometria exportada inspecionada no assembly do rescomp: saída 1 = `(352,144,24,32,2)`; saída 2 = `(672,152,24,32,1)`, cinco campos de 16 bits na ordem de `StageExit`.
- Sete testes de rejeição de TMX inválidos passaram (`tools/test-stage-validation.ps1`).
- Validação dos dois TMX: largura, spawn, dimensões/tipos das saídas e destinos.
- Os cenários interativos acima ainda precisam de execução no emulador; compilação e validação estática não comprovam comportamento visual, consumo de memória ou controle durante o jogo.
