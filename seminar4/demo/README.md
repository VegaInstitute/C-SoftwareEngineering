# Демо к семинару «Дизайн архитектуры»

Небольшая программа на C++20: загрузчик минутных баров из двух источников. Основной источник — файл в формате MOEX ISS, резервный — CSV-выгрузка брокера. Если основной источник отвечает ошибкой, класс `FailoverSource` переключается на резервный, не выдаёт бары повторно и считает пропуски. Данные лежат в папке `data/`, сеть и внешние библиотеки не нужны.

Нужны CMake 3.20 или новее и компилятор с поддержкой C++20. Все команды выполняются из этой папки.

## Windows (PowerShell)

С Visual Studio 2022:

```powershell
cmake -S . -B build
cmake --build build
.\build\Debug\rp_cli.exe
.\build\Debug\rp_cli.exe --iss-fail-after 3
ctest --test-dir build -C Debug --output-on-failure
```

С MinGW (g++) и Ninja:

```powershell
cmake -S . -B build -G Ninja
cmake --build build
.\build\rp_cli.exe
.\build\rp_cli.exe --iss-fail-after 3
ctest --test-dir build --output-on-failure
```

## Linux (bash) и macOS (zsh)

```bash
cmake -S . -B build
cmake --build build
./build/rp_cli
./build/rp_cli --iss-fail-after 3
ctest --test-dir build --output-on-failure
```

Инструменты устанавливаются так: в Ubuntu — `sudo apt install build-essential cmake`, в macOS — `xcode-select --install` и `brew install cmake`.

## Что должно получиться

```text
bars=15 (iss=15, broker=0) gaps=0 source_of_last=iss
WARN: iss failed: HTTP 503 from MOEX ISS (simulated) -> switching to broker
bars=14 (iss=3, broker=11) gaps=1 source_of_last=broker
100% tests passed, 0 tests failed out of 1
```

Обычный запуск берёт все бары из основного источника. Флаг `--iss-fail-after 3` имитирует отказ основного источника после трёх баров: программа переключается на брокера, а пропущенная минута видна в `gaps=1`. Тест `failover_test` проверяет это переключение.
