# PoC de travamento em retrato com autocorreção de composição

Data dos testes: 2026-08-23 a 2026-08-24
Dispositivo: Redmi Note 9S (`curtana`), Android 16 / API 36
Base: build AIR 51 target 36 descrita em [air51-modern-poc.md](air51-modern-poc.md)

## Problema

Com a Activity travada em `portrait` no Manifest (requisito do port), o editor
abre com a composição de tela e o mapeamento de toque incorretos. O estado só é
corrigido depois de um ciclo físico real de rotação do aparelho (girar para
paisagem e voltar). Depois disso, tudo funciona indefinidamente.

## Causa raiz mapeada

A cadeia completa foi rastreada em `libCore.so` (ARMv7, Thumb-2):

1. Uma rotação real dispara `onConfigurationChanged`, que enfileira um evento
   Java `WindowEventData` via `Entrypoints.registerWindowCallback`.
2. O evento chega ao `customHandler` (id de mensagem interno `0x463`) e executa
   `run()`, que chama `OrientationManager.nativeOrientationChanged(before, after)`.
3. O JNI (`0x10bc79`) converte os inteiros para enums de orientação e invoca o
   recalculo nativo em `0x1df728`.

Varredura de referências cruzadas (`BL`) confirmou que `0x1df728` possui **um
único chamador** (o caminho acima) e que o equivalente de "mudando"
(`nativeOrientationChanging` → `0x1df5d8`) não tem nenhum chamador. Ou seja:
**nenhum caminho de boot executa esse recalculo** — por isso a composição nasce
errada e só uma rotação real corrige.

### Protocolo de eventos (reconstruído)

`WindowEventData(view, msg, i1, i2, z)` — mensagens relevantes:

| msg | significado | argumentos |
|---:|---|---|
| 2 | ON_SIZE_CHANGED | `i1`=largura, `i2`=altura |
| 3 | ORIENTATION_CHANGED | `i1`=antes, `i2`=depois |
| 4 | ON_FORMAT_CHANGED | formato da surface |
| 5 | FORCE_REDRAW | — |

Enums `EOrientation`: `0`=UNKNOWN, `1`=DEFAULT, `2`=ROTATED_LEFT,
`3`=ROTATED_RIGHT, `4`=UPSIDE_DOWN. A sequência real emitida por
`surfaceChanged` numa rotação é: formato → tamanho → orientação → redraw.

## Solução: repetição sintética do ciclo

Reemitir manualmente a sequência observada engana o runtime de forma limpa,
sem alterar código nativo:

```text
size(h, w) → orient(DEFAULT → ROTATED_RIGHT) → size(w, h)
→ orient(ROTATED_RIGHT → DEFAULT) → FORCE_REDRAW
```

Implementado em Java injetado (`OrientHeal`), despachando cada evento com
`registerWindowCallback(0, evento, null)` na thread principal.

Resultado confirmado em campo: **um toque qualquer na tela corrige a composição
e o toque instantaneamente**, de forma permanente (equivalente ao ciclo físico
de rotação). A partir dessa base, o objetivo passou a ser disparar o ciclo
automaticamente no fim do boot.

## Evolução das builds

| Build | Gatilho do ciclo | Resultado |
|---|---|---|
| v22 | — (só validação de feed de descriptor) | abre sem rejeição; comportamento inalterado |
| v23 | primeiro foco de janela, só par de orientação | falhou: faltou o evento de tamanho antes |
| v24 | primeiro toque, ciclo completo de 5 eventos | **funcionou** — toque único cura |
| v25–v26 | splash removido / onda de frames | morto em silêncio (ver diagnóstico) |
| v27–v28 | instrumentação total | revelou as causas dos insucessos |
| v29–v30 | primeira `surfaceChanged` pós-boot + correções | candidato atual |

Durante a v22 também ficou provado que os valores de orientação do descriptor
AIR não comandam a composição inicial nesta build: o parser consome os valores
ajustados em memória e o comportamento não muda. O arquivo em disco permanece
byte a byte idêntico ao original.

## Diagnóstico sem logcat

Sem acesso a logcat no ambiente, o próprio aplicativo foi instrumentado para
escrever um diário cronológico em `/sdcard/PSX/heal.log` (timestamp + marca),
com captura de exceções (`markEx`). O método se provou decisivo e expôs duas
armadilhas clássicas de edição smali:

1. **VerifyError**: desviar um branch para dentro de um handler de exceção cuja
   primeira instrução é `move-exception` é ilegal no dex — o verificador rejeita
   a classe inteira no primeiro uso.
2. **IllegalAccessError**: campos `private` acessados entre classes internas
   compiladas à mão não recebem os *bridges* que o compilador Java geraria;
   precisam ser `public` explicitamente.

Também ficou caracterizado que o boot do runtime bloqueia a thread principal
por ~10 s (mensagens postadas nesse período só entregam depois) e que um
*warm restart* do processo preserva estáticos — gatilhos precisam ser rearmed
por sessão de foreground (`onResume`), não por processo.

## Estado atual

- Editor abre e opera no Android 16 com trava de retrato; correção de composição
  por toque único confirmada em uso real.
- Disparo automático implementado (onda imediata pós-boot + onda sincronizada
  por frames + gatilho de `surfaceChanged` pós-boot com rearme por resume).
- Validação final do disparo automático pendente em dispositivo.
