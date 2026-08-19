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

## Próxima ação técnica

Investigar a substituição/portabilidade de `libCore.so` e das bibliotecas privadas dependentes, preservando a ABI esperada pelo AIR e pelo `TTPixelExtension`. Não adianta continuar variando somente `targetSdkVersion`.

