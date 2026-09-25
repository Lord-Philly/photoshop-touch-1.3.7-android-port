# Photoshop Touch Native — Plano B

Primeira fatia do caminho nativo: um editor offline Kotlin usando `View` e
`android.graphics.Canvas`. O Canvas Android é renderizado pelo Skia; não há
dependência do AIR, SWF ou das ANEs antigas.

## Estado atual

- alvo `minSdk 29` (Android 10) e `targetSdk 36` (Android 16);
- tela de editor nativa;
- pincel livre com toque;
- Novo, Limpar e Desfazer;
- exportação PNG via MediaStore em `Pictures/PS Touch Native`;
- sem conta, nuvem, push ou permissões de armazenamento legado.

Esta é uma prova vertical do novo motor, não uma reimplementação completa do
Photoshop Touch. O modelo de camadas, PSDX, filtros e importação entram nas
próximas etapas.

## APK experimental

O APK debug desta primeira fatia está publicado na release [`v0.4-native-editor-poc`](https://github.com/Lord-Philly/photoshop-touch-1.3.7-android-port/releases/tag/v0.4-native-editor-poc).
Ele foi compilado localmente com sucesso; a instalação no aparelho físico fica
para quando a sessão de depuração USB estiver enumerada novamente.

## Compilar

No Windows, com Android SDK e Java 17:

```powershell
cd native-app
.\gradlew.bat assembleDebug
```

O APK sai em `app/build/outputs/apk/debug/app-debug.apk`.
