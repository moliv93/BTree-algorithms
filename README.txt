## English ##

Description:
    This is a simple collection of B-Tree and B*-Tree simulation algorithms, made for the Data Structure class of the Computer Science course at IFMG - Campus Formiga. The simulation is done at main memory, and some versions have a graphical interface made with OpenGL for ease of visualization.

Algorithms:
    BStar_visual.cpp: B*-Tree of order 4 with OpenGL graphical interface
    BStar_text_mode.cpp: B*-Tree of order 4 with simple text-mode menu
    BTree_visual_normal.cpp: simple B-tree with graphical interface
    BTree_visual_key_duplication.cpp: B-tree with leaf keys replication at parent pages

Compilation (linux):
    The compilation of the graphical implementations need GLFW3 and GLM's libraries, which can be installed with:
    sudo apt-get install libglfw3-dev
    and
    sudo apt-get install libglm-dev

    After installed, the compilation can be done manually, linking the correct dynamic libs, and the file glad/src/glad.c, or using make:
    make bstar: compiles and executes BStar_visual.cpp
    make bstar_text: compiles and executes BStar_text_mode.cpp
    make btree: compiles and executes BTree_visual_normal.cpp
    make btree_key_repeat: compiles and executes BTree_visual_key_duplication.cpp

Compilation (Windows):
    The compilation on Windows (the graphical interface compilation) is not yet guaranteed, but is possible to achieve this by installing and linking the correct libs for your compiler version. Note that a C++ compiler with support to C++17 is needed.

Graphical interface controls:
    P: insert a new element sequentially bigger to the last inserted (default: 101)
    O: insert a new element sequentially smaller to the last inserted (default: 99)
    I: asks the user for a number on the terminal, inserts, and override the last record of insertion
    Arrow keys: move the visualization's point of view
    Z: zoom in
    X: zoom out
    R: sets point of view to default position
    ESC: close the program
    M/N: changes the background
    to add more backgrounds, simply put any JPG/JPEG image (with .jpg extension) on the folder "backgrounds"

Contribution:
    In case you find a bug, want to recommend an upgrade, or have some question about the project or the algorithms, feel free to contact me by email: tiagomoliv93@gmail.com

## Portugues ##

Descri????o:
Esta ?? uma simples cole????o de algoritmos de simula????o de ??rvores B e B*, feitos para a disciplina de Estruturas de Dados do curso de Ci??ncia da Computa????o do IFMG - Campus Formiga. A simula????o ?? feita em mem??ria principal, e algumas das vers??es possuem interface gr??fica feita em OpenGL para facilitar a visualiza????o.

Algoritmos:
        BStar_visual.cpp: ??rvore B* de ordem 4 com recursos visuais usando OpenGL
        BStar_text_mode.cpp: ??rvore B* de ordem 4 com menu simples em modo texto
        BTree_visual_normal.cpp: ??rvore B simples com recursos visuais usando OpenGL
        BTree_visual_key_duplication.cpp: ??rvore B com replica????o de chaves dos n??s folha nas p??ginas acima

Compila????o (linux):
    A compila????o das implementa????es com OpenGL dependem das bibliotecas GLFW3 e GLM e podem ser instaladas usando:
    sudo apt-get install libglfw3-dev
    e
    sudo apt-get install libglm-dev

    Ap??s instaladas, a compila????o pode ser feita manualmente, linkando as devidas bibliotecas din??micas e o arquivo glad/src/glad.c, ou usando o make:
    make bstar: compila e executa o algoritmo BStar_visual.cpp
    make bstar_text: compila e executa o algoritmo BStar_text_mode.cpp
    make btree: compila e executa o algoritmo BTree_visual_normal.cpp
    make btree_key_repeat: compila e executa o algoritmo BTree_visual_key_duplication.cpp

Compila????o (windows):
    A compila????o em ambiente Windows (para as vers??es que usam OpenGL) n??o ?? garantida por enquanto, mas ?? poss??vel de se realizar caso as bibliotecas necess??rias sejam corretamente instaladas e ligadas para a vers??o do seu compilador ou IDE. Note que ?? necess??rio um compilador C++ que suporte a vers??o C++17

Instru????es de controle da vers??o gr??fica:
    P: insere um novo elemento sequencialmente maior que o ??ltimo inserido (default: 101)
    O: insere um novo elemento sequencialmente menor que o ??ltimo inserido (default: 99)
    I: insere um elemento digitado no terminal, e sobrescreve a variavel que guarda o ??ltimo elemento inserido
    Setas do teclado: move a posi????o de visualiza????o
    Z: aumenta o zoom
    X: diminui o zoom
    R: volta a vizualiza????o para a sua posi????o inicial
    ESC: finaliza o programa
    M/N: troca de plano de fundo (se houverem mais de um)
    para adicionar planos de fundo, simplesmente coloque uma imagem em formato e extens??o .jpg na pasta "backgrounds"

Contribui????es:
    Caso encontre um bug, queira propor uma melhoria, ou tenha alguma d??vida sobre o projeto ou algoritmo, sinta-se livre para me contactar por email: tiagomoliv93@gmail.com
