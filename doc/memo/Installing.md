*Un memo sur toutes les dépendances a instller et comment lancer les tests par la suite.*

## VGPK

1. **Vérifier si vcpkg est déjà installé**
- echo $VCPKG_ROOT
- ls ~/vcpkg 2>/dev/null
- which vcpkg

 1. **Si vcpkg n’est pas installé**
- sudo apt install -y build-essential cmake git curl zip unzip tar pkg-config
- git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
- ~/vcpkg/bootstrap-vcpkg.sh -disableMetrics0

1. **Définir la variable pour la session en cours :**
- export VCPKG_ROOT=~/vcpkg

1. **Pour rendre la variable permanente (bash) :**
- echo 'export VCPKG_ROOT=$HOME/vcpkg' >> ~/.bashrc
- source ~/.bashrc

**Lancer un test:**
- rm -rf build
- cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
- cmake --build build
- ctest --test-dir build --output-on-failure
