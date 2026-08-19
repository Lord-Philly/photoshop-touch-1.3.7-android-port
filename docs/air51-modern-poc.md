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

Todas as builds foram assinadas com uma chave de teste local. Elas não são uma
atualização assinada pela Adobe e não devem ser tratadas como versão estável.

## O que foi comprovado

1. O Android 16 instala a aplicação quando o pacote usa AIR moderno e target 36.
2. O `libCore.so` do AIR 51 carrega no namespace do aplicativo; o erro anterior de
   `libstagefright.so` privada do AIR legado deixa de ser o primeiro bloqueio.
3. As extensões originais foram reconstruídas como ANEs Android-only para diagnóstico,
   usando os descriptors/SWFs presentes no APK. Isso não recompila o código nativo.
4. O APK com as ANEs chega a carregar os três SWFs de extensão, mas as bibliotecas
   nativas antigas não são aceitas pelo linker moderno.

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

## Conclusão desta etapa

O alvo Android 16 é tecnicamente instalável com AIR 51, mas Photoshop Touch ainda
não está funcional. O próximo marco exige uma destas rotas:

- obter/reconstruir o código-fonte das ANEs e recompilar para ARMv7 com código PIC,
  APIs públicas atuais e AIR/FRE moderno;
- substituir cada ANE por uma implementação compatível, preservando os contratos
  ActionScript usados pelo `TTPixel.swf`;
- testar novamente o editor real e só então preencher Android 10–15.

Até essa etapa, a matriz permanece `Pendente` para Android 10–15 e `Bloqueio nativo`
para Android 16. Instalação bem-sucedida não é compatibilidade funcional.

