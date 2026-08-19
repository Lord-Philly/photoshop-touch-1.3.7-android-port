# Teste real — Android 16

Data: 2026-08-19  
Dispositivo: Redmi Note 9S (`curtana`)  
Android: `16`  
API: `36`  
ABIs: `arm64-v8a, armeabi-v7a, armeabi`

O aparelho foi detectado e mantido conectado pelo ADB durante os testes.

## Resultado resumido

| Artefato | Instalação | Inicialização | Resultado |
|---|---|---|---|
| APK original, target 12 | Falhou | Não executou | Android 16 rejeitou SDK obsoleto |
| PoC target 35, primeiro build | Falhou | Não executou | `resources.arsc` precisava estar descompactado/alinhado |
| PoC target 36, alinhada | Sucesso | Crash | `libCore.so` não carrega `libstagefright.so` |
| Diagnóstico target 24 | Sucesso | Crash | Mesmo bloqueio nativo; reduzir target não resolve |
| AIR 51 target 36, sem ANEs declaradas | Sucesso | Processo vivo; splash | Runtime moderno carrega; funcionalidade do editor não comprovada |
| AIR 51 target 36, ANEs reconstruídas | Sucesso | Crash | ANEs antigas recusadas por `DT_TEXTREL` |

## APK original

Comando executado:

```text
adb install air.com.adobe.pstouchphone_1.3.7-1003007_minAPI14(...).apk
```

Resultado:

```text
INSTALL_FAILED_DEPRECATED_SDK_VERSION:
App package must target at least SDK version 24, but found 12
```

## PoC Android 16

A variante foi criada com:

- `targetSdkVersion=36`;
- `versionCode=1003009`;
- `versionName=1.3.7-android16-poc`;
- `android:exported` explícito;
- `resources.arsc` descompactado;
- ZIP alinhado com `zipalign -p -f 4`;
- assinatura de debug local.

Hash SHA-256:

```text
2B1E2FBD64BE0E17175F51FC0D74E1414B4090B346E7986B649B8FFDEB9FFD95
```

Instalação: **sucesso**.

O primeiro erro nativo relevante foi:

```text
dlopen failed: library "libstagefright.so" not found: needed by libCore.so
```

Embora `/system/lib/libstagefright.so` exista no aparelho, o Android 16 não a disponibiliza no namespace de bibliotecas do aplicativo. Em seguida, o AIR tenta chamar métodos JNI cujo runtime nativo não foi carregado:

```text
java.lang.UnsatisfiedLinkError:
No implementation found for
com.adobe.air.AIRWindowSurfaceView.nativeOnFormatChangeListener(int)
```

O processo é encerrado pelo sistema.

## Diagnóstico target 24

Foi criado um segundo APK com `targetSdkVersion=24`, instalado com sucesso e executado no mesmo aparelho. O log mostrou o mesmo erro `libCore.so → libstagefright.so` e o mesmo `UnsatisfiedLinkError`.

Conclusão: o target mínimo de instalação é um problema separado. O bloqueio de execução é a dependência nativa privada do AIR legado.

## PoC com AIR 51

O runtime legado foi substituído pelo AIR SDK 51.3.4.1, com `targetSdkVersion=36`.
O `libCore.so` moderno carrega no Android 16 e elimina o erro inicial de
`libstagefright.so` privada. O APK sem extensões declaradas manteve o processo
vivo por 12 segundos, mas ficou no splash porque o SWF depende das ANEs.

As ANEs foram reconstruídas para diagnóstico com os descriptors e SWFs presentes
no APK original. A instalação foi bem-sucedida, mas o log mostrou:

```text
has text relocations
dlopen failed: .../libsibsynclib.so
dlopen failed: .../libair.com.adobe.cc.sync.SyncEngine.so
dlopen failed: .../libTTPixelExtensionAndroid.so
No implementation found for ... TTPixelExtension.initIDs()
```

O símbolo JNI existe na biblioteca, mas o linker a rejeita antes de disponibilizá-lo.
Uma variante que neutraliza a carga Java das ANEs chega a `JNI DETECTED ERROR IN
APPLICATION: java_object == null`, confirmando uma segunda dependência nativa do
SWF. O relatório completo está em [air51-modern-poc.md](air51-modern-poc.md).

## Próxima ação técnica

Investigar a substituição/portabilidade de `libCore.so` e das bibliotecas privadas dependentes, preservando a ABI esperada pelo AIR e pelo `TTPixelExtension`. Não adianta continuar variando somente `targetSdkVersion`.
