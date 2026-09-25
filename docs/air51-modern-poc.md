# PoC AIR 51 — Android 16

Data do teste: 2026-08-19  
Dispositivo: Redmi Note 9S (`curtana`, serial ADB `87003b11`)  
Android: 16 / API 36  
ABI do aparelho: `arm64-v8a, armeabi-v7a, armeabi`

Esta etapa troca o runtime AIR legado embutido no APK por um captive runtime AIR SDK
51.3.4.1 e mantém `targetSdkVersion=36`. O SDK foi obtido pelo fluxo oficial do AIR
SDK Manager; o ZIP usado localmente tem SHA-256:

```text
4D6DD4449A2850536A8E4623CEB446DDA4A796D7622BA98B48E6ABC9608AA8E0
```

Referências do runtime: [instalação do AIR SDK](https://airsdk.dev/docs/basics/install/windows),
[comando ADT package](https://airsdk.dev/docs/building/air-developer-tool/commands/package)
e [documentação de ANEs](https://airsdk.dev/docs/building/using-native-extensions).

## Artefatos testados

| Artefato | Target | Instalação | Execução no Android 16 | SHA-256 |
|---|---:|---|---|---|
| `android16-air51-no-ane.apk` | 36 | Sucesso | Processo permaneceu vivo por 12 s; ficou na tela inicial/splash | `E75B3E2AD9B899EADB0EB9EF8CA2AF6F848D7FC0CB6ACBD4F8EC15C5891FB531` |
| `android16-air51-ane-modernized.apk` | 36 | Sucesso | Crash durante `TTPixelExtension.initIDs()` | `4065F29E83CE8997C720A0F4C5250103E782D7569CE37C6562567DBB86F262E3` |
| `android16-air51-no-native-ane3.apk` | 36 | Sucesso | Bootstrap chega a `java_object == null` e aborta | `FA62A7BFAB4F541DC428B4B78C8EC262EA5FFB386C22B48FF139767E931C88B1` |
| `android16-air51-ane-compat2.apk` | 36 | Sucesso | Os três `.so` substitutos carregam; splash é removido; editor ainda não validado | `19290C344F38A67248C6EF6D5784BF0F6852018B51407943C7BBB16917C526FE` |

Todas as builds foram assinadas com uma chave de teste local. Elas não são uma
atualização assinada pela Adobe e não devem ser tratadas como versão estável.

## Revalidação do build preservado no Android 13

Em 2026-09-25 foi instalado no Redmi 10C (`220333QAG`, serial ADB `b5aa2b42`,
Android 13 / API 33, ABI ARM64 com suporte a ARMv7) o pacote lateral
`air.com.lordphilly.pstouch.originalcompat`, com o artefato
`original-preserved-compat2.apk`. A instalação foi concluída sem substituir o
pacote `air.com.adobe.pstouchphone` existente.

Resultado: `AIRAppEntry` permanece em primeiro plano, sem crash, mas a superfície
fica preta e o editor não aparece. Portanto, a variante confirma apenas que o
runtime AIR 51 e as bibliotecas PIC carregam nesse aparelho; os stubs atuais das
ANEs ainda não satisfazem o bootstrap do `TTPixel.swf`. O próximo marco deve
implementar o primeiro contrato real de inicialização do TTPixel, começando por
`ECUtils`/`TTPixelExtensionContextImpExp`, antes de testar filtros ou a UI.

## Contrato incremental TTPixel: ECUtils + ImpExp

Em 2026-09-25 foi implementado o primeiro contrato funcional na fonte
[`native/ane-compat/compat_stubs.c`](../native/ane-compat/compat_stubs.c), sem
alterar o SWF original e sem substituir o pacote Adobe instalado.

### ECUtils implementado

As entradas JNI abaixo deixaram de ser stubs vazios:

```text
bitmapDataCopy
bitmapDataResample
getPixelsEx
setPixelsEx
moveBitmapDataEx
getScaledPixelsEx
uncompressBitmapDataEx
lz4GetMaxCompressDestLength
lz4Compress
lz4Uncompress
lz4Free
```

Os caminhos de pixels validam stride, regiao e capacidade do `ByteBuffer`. A
descompressao bitmap usa zlib. O LZ4 emite um bloco literal valido e possui
decoder correspondente; ele ainda nao tem a otimizacao de busca de matches do
LZ4 original. A conversao de alpha premultiplicado e uma implementacao de
compatibilidade baseada no contrato dos wrappers Java; deve ser comparada com
imagens reais assim que o SWF atingir essas funcoes.

Continuam deliberadamente sem implementacao real, por nao terem sido observadas
nesta etapa: operacoes de arquivo bitmap, `alphaBlend`, `isolateColor`,
`getPixelsBitmapEx`, `copyBitmapData` e os demais contextos nao relacionados ao
primeiro contrato.

### TTPixelExtensionContextImpExp implementado

O estado `exporterPtr` agora aponta para um estado nativo que guarda os bytes,
tamanho, progresso, conclusao e cancelamento. Foram implementados:

```text
startEncodeLz4       startEncodeZLib
startEncodePNG       startEncodeJPEG
getEncodedData       getEncodedDataSize
getEncodingProgress  hasFinishedEncoding
requestCancel        waitFinishedEncoding
clearEncodedData     isPossiblyPremultipliedData
premultiplyData      unPremultiplyData
```

PNG/JPEG usam `android.graphics.Bitmap.compress`; zlib e LZ4 usam buffers nativos.
O JPEG recebe o caminho textual do wrapper original, mas esta POC nao grava nesse
caminho: retorna os bytes para `getEncodedData`, que e o fluxo usado pelo
contexto Java. A codificacao e sincrona nesta primeira implementacao; progresso
fica em 100 quando a funcao retorna e eventos de conclusao sao despachados para
o contexto.

### Mapa estatico dos nomes ActionScript registrados

O descriptor Java do contexto `ImpExp` registra exatamente:

```text
startEncodeLz4, startEncodeZLib, startEncodeJPEG, startEncodePNG,
getEncodedData, getEncodedDataSize, getEncodingProgress,
hasFinishedEncoding, requestCancel, waitFinishedEncoding, clearEncodedData
```

O contexto `Utils` registra:

```text
trace, getDeviceID, getDeviceName, canLaunchApp, launchApp, leaveApp,
setStatusBarHidden, getNetworkState, resolveImageContentURI,
copyURIContentToFile, isolateColor, clipboardHasFormat,
clipboardSetFormatData, clipboardSetMultiData, clipboardGetFormatData,
plistToXMLString, compressBitmapRLE, getPixelsEx, setPixelsEx,
lz4Compress, lz4Uncompress, lz4Deflate, lz4Inflate, getPixelsBitmapEx,
getMemoryStatsEx, uncompressBitmapDataEx, bitmapDataToFileEx,
bitmapDataFromFileEx, getScaledPixelsEx, bitmapFileCreateEmpty,
bitmapFileCreateFromBitmapData, bitmapFileWrite, bitmapFileRead,
bitmapFileResample, bitmapDataCopy, bitmapDataResample, getSystemInfo,
setWakeLock, getLocalIP, alphaBlend, getTimestamp, openPath,
setRequestedOrientation, getRequestedOrientation,
showNetworkActivityIndicator
```

A lista acima e o mapa exato de funcoes exposto pelo Java original. A sequencia
exata de chamadas do `TTPixel.swf` durante o bootstrap ainda nao pode ser
marcada como observada: no teste abaixo nem o carregamento da biblioteca nativa
ocorreu, logo nao houve trace JNI para atribuir ao SWF.

### APK side-by-side e resultado verificavel

Build compilada com NDK 25.2.9519653 para `armeabi-v7a` e empacotada com AIR
51.3.4.1:

```text
APK: original-preserved-ecutils-impexp-02.apk
SHA-256: AACDA5CEB7CF4F216B6992AF75B4BAA48CFFB71110E86F8294E82A4F36AB39D0
Package de teste: air.com.lordphilly.pstouch.originalcompat
versionCode: 1003010
Dispositivo: Redmi 10C / 220333QAG / Android 13 API 33
```

O APK foi instalado separadamente via `/data/local/tmp` e `pm install -r`.
O pacote original permaneceu intacto:

```text
air.com.adobe.pstouchphone / versionCode 9009009 / targetSdk 17
```

Resultado da execucao: `AIRAppEntry` permanece vivo, sem crash, mas a superficie
fica preta. O marcador de carregamento `TTPixelCompat` e nenhuma chamada JNI
`ECUtils`/`ImpExp` aparecem no `logcat`, mesmo apos limpar os dados somente do
pacote de teste. Isso desloca o proximo bloqueio para antes da ANE: carregamento
do SWF, registro/resolucao da extensao ou inicializacao do stage AIR. Nao ha
evidencia suficiente para afirmar que o bootstrap passou pelo contrato real.

Limite documentado: a implementacao nativa esta compilada e instalada no APK de
teste, mas a validacao comportamental no aparelho permanece pendente ate o SWF
chegar a primeira chamada JNI. Nao avancar para outras ANEs ou para a interface
antes de obter esse primeiro trace.

Após esse teste, a fonte recebeu apenas ajustes de conversao de alpha e validacao
de regioes, e foi recompilada no artefato `original-preserved-ecutils-impexp-03.apk`:

```text
SHA-256: 7422A5106919724E291B0D95FE1C71739D851E891117DB34578A37ABB74DA7C4
versionCode: 1003011
```

Essa build esta pronta, mas a instalacao final ficou pendente porque o ADB do
dispositivo voltou ao estado `unauthorized` e exige confirmacao manual da chave
de depuracao USB. O teste documentado acima continua sendo o ultimo teste de
execucao efetivamente instalado.

## O que foi comprovado

1. O Android 16 instala a aplicação quando o pacote usa AIR moderno e target 36.
2. O `libCore.so` do AIR 51 carrega no namespace do aplicativo; o erro anterior de
   `libstagefright.so` privada do AIR legado deixa de ser o primeiro bloqueio.
3. As extensões originais foram reconstruídas como ANEs Android-only para diagnóstico,
   usando os descriptors/SWFs presentes no APK. Isso não recompila o código nativo.
4. O APK com as ANEs chega a carregar os três SWFs de extensão, mas as bibliotecas
   nativas antigas não são aceitas pelo linker moderno.

5. A camada de compatibilidade ARMv7 em [`native/ane-compat`](../native/ane-compat)
   recompila `TTPixelExtensionAndroid` e fornece bibliotecas PIC mínimas para
   `sibsynclib` e `SyncEngine`. No teste do APK `android16-air51-ane-compat2.apk`,
   o log confirmou `ok` para os três carregamentos e `AIR - removed splash screen`;
   o processo permaneceu vivo por pelo menos 27 segundos.

## Bloqueio nativo confirmado

`llvm-readelf --dyn-syms` confirma que `libTTPixelExtensionAndroid.so` exporta:

```text
Java_com_adobe_ttpixel_extension_TTPixelExtension_initIDs
```

Porém `llvm-readelf -d` também mostra `DT_TEXTREL` em:

- `libTTPixelExtensionAndroid.so`;
- `libair.com.adobe.cc.sync.SyncEngine.so`;
- `libsibsynclib.so`;
- `libCore.so` legado.

No Android 16 o linker recusa essas bibliotecas com:

```text
has text relocations
dlopen failed
```

Por isso o ART depois relata `No implementation found ... initIDs`: o símbolo
existe no arquivo, mas a biblioteca nunca chegou a ser carregada.

## Diagnóstico da variante sem carga nativa

Uma variante de laboratório substituiu somente a classe Java de entrada da
`TTPixelExtension` por uma implementação vazia, sem chamar `System.loadLibrary`.
Ela confirmou que o AIR 51 inicia as três extensões, mas o SWF/aplicação ainda
depende de objetos nativos e provoca:

```text
JNI DETECTED ERROR IN APPLICATION: java_object == null
```

Essa variante serve apenas para separar o bootstrap do runtime da funcionalidade
do editor. Não é uma solução de compatibilidade.

## Limite atual da substituição

O substituto remove o bloqueio de carregamento e permite que o AIR conclua o
bootstrap, mas ainda não implementa o editor. Operações de pixels, filtros,
codificação JPEG/PNG, quick selection, câmera e sincronização continuam sendo
placeholders ou precisam de uma implementação real compatível com os contratos
ActionScript. Portanto, o resultado é `bootstrap compatível`, não ainda
`Photoshop Touch funcional`.

## Conclusão desta etapa

O alvo Android 16 é tecnicamente instalável com AIR 51, mas Photoshop Touch ainda
não está funcional. O próximo marco exige uma destas rotas:

- implementar gradualmente os contratos de pixels e exportação na camada
  `native/ane-compat`, começando por `ECUtils` e `TTPixelExtensionContextImpExp`;
- substituir os contextos de câmera, pressão, quick selection e SyncEngine por
  implementações compatíveis, preservando os contratos ActionScript usados pelo
  `TTPixel.swf`;
- testar novamente o editor real e só então preencher Android 10–15.

Até essa etapa, a matriz permanece `Pendente` para Android 10–15 e `Bloqueio nativo`
para Android 16. Instalação bem-sucedida não é compatibilidade funcional.
